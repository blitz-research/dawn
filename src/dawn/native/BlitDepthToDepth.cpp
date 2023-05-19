// Copyright 2023 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn/native/BlitDepthToDepth.h"

#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPipeline.h"

namespace dawn::native {

namespace {

constexpr char kBlitToDepthShaders[] = R"(

@vertex fn vert_fullscreen_quad(
  @builtin(vertex_index) vertex_index : u32,
) -> @builtin(position) vec4f {
  const pos = array(
      vec2f(-1.0, -1.0),
      vec2f( 3.0, -1.0),
      vec2f(-1.0,  3.0));
  return vec4f(pos[vertex_index], 0.0, 1.0);
}

@group(0) @binding(0) var src_tex : texture_depth_2d;

// Load the depth value and return it as the frag_depth.
@fragment fn blit_to_depth(@builtin(position) position : vec4f) -> @builtin(frag_depth) f32 {
  return textureLoad(src_tex, vec2u(position.xy), 0);
}

)";

ResultOrError<Ref<RenderPipelineBase>> GetOrCreateDepthBlitPipeline(DeviceBase* device,
                                                                    wgpu::TextureFormat format) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    {
        auto it = store->depthBlitPipelines.find(format);
        if (it != store->depthBlitPipelines.end()) {
            return it->second;
        }
    }

    ShaderModuleWGSLDescriptor wgslDesc = {};
    ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;
    wgslDesc.code = kBlitToDepthShaders;

    Ref<ShaderModuleBase> shaderModule;
    DAWN_TRY_ASSIGN(shaderModule, device->CreateShaderModule(&shaderModuleDesc));

    FragmentState fragmentState = {};
    fragmentState.module = shaderModule.Get();
    fragmentState.entryPoint = "blit_to_depth";

    DepthStencilState dsState = {};
    dsState.format = format;
    dsState.depthWriteEnabled = true;
    dsState.depthCompare = wgpu::CompareFunction::Always;

    RenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.vertex.module = shaderModule.Get();
    renderPipelineDesc.vertex.entryPoint = "vert_fullscreen_quad";
    renderPipelineDesc.depthStencil = &dsState;
    renderPipelineDesc.fragment = &fragmentState;

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateRenderPipeline(&renderPipelineDesc));

    store->depthBlitPipelines[format] = pipeline;
    return pipeline;
}

}  // namespace

MaybeError BlitDepthToDepth(DeviceBase* device,
                            CommandEncoder* commandEncoder,
                            const TextureCopy& src,
                            const TextureCopy& dst,
                            const Extent3D& copyExtent) {
    ASSERT(device->IsLockedByCurrentThreadIfNeeded());
    // ASSERT that the texture have depth and are not multisampled.
    ASSERT(src.texture->GetFormat().HasDepth());
    ASSERT(dst.texture->GetFormat().HasDepth());
    ASSERT(src.texture->GetSampleCount() == 1u);
    ASSERT(dst.texture->GetSampleCount() == 1u);

    // Note: because depth texture subresources must be copied in full, this blit
    // does not need to handle copy subrects.

    // Allow internal usages since we need to use the source as a texture binding, and
    // the destination as a render attachment.
    auto scope = commandEncoder->MakeInternalUsageScope();

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline,
                    GetOrCreateDepthBlitPipeline(device, dst.texture->GetFormat().format));

    Ref<BindGroupLayoutBase> bgl;
    DAWN_TRY_ASSIGN(bgl, pipeline->GetBindGroupLayout(0));

    // TODO(crbug.com/dawn/838)
    // Metal Intel drivers incorrectly create texture views starting at a  nonzero array layer. They
    // also don't textureLoad in the shader at a non-zero array index correctly. Workaround this
    // issue by copying the non-zero array slices to a single-layer texture. That texture will be be
    // sampled as the source instead.
    std::vector<Ref<TextureViewBase>> srcViews;
    srcViews.reserve(copyExtent.depthOrArrayLayers);
    for (uint32_t z = 0; z < copyExtent.depthOrArrayLayers; ++z) {
        uint32_t layer = src.origin.z + z;
        Ref<TextureViewBase> srcView;
        if (layer == 0u) {
            // The zero'th slice. We can use the original texture.
            TextureViewDescriptor viewDesc = {};
            viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.baseMipLevel = src.mipLevel;
            viewDesc.mipLevelCount = 1u;
            DAWN_TRY_ASSIGN(srcView, src.texture->CreateView(&viewDesc));
        } else {
            // Create a single-layer intermediate texture to use as the texture source.
            TextureDescriptor intermediateTexDesc = {};
            intermediateTexDesc.format = src.texture->GetFormat().format;
            intermediateTexDesc.usage =
                wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
            intermediateTexDesc.size = {copyExtent.width, copyExtent.height};

            Ref<TextureBase> intermediateTexture;
            DAWN_TRY_ASSIGN(intermediateTexture, device->CreateTexture(&intermediateTexDesc));

            // Copy from the original texture source into the intermediate.
            {
                ImageCopyTexture intermediateSrc;
                intermediateSrc.texture = src.texture.Get();
                intermediateSrc.mipLevel = src.mipLevel;
                intermediateSrc.origin = {0, 0, layer};
                intermediateSrc.aspect = wgpu::TextureAspect::All;

                ImageCopyTexture intermediateDst;
                intermediateDst.texture = intermediateTexture.Get();
                intermediateDst.mipLevel = 0u;
                intermediateDst.origin = {0, 0, 0};
                intermediateDst.aspect = wgpu::TextureAspect::All;

                // Note: This does not recurse infinitely because the workaround to
                // blit depth is not needed if the destination level and layer is 0,
                // and the copy depth is 1.
                commandEncoder->APICopyTextureToTexture(&intermediateSrc, &intermediateDst,
                                                        &intermediateTexDesc.size);
            }

            // Create a texture view pointing to the intermediate texture.
            TextureViewDescriptor viewDesc = {};
            viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
            DAWN_TRY_ASSIGN(srcView, intermediateTexture->CreateView(&viewDesc));
        }
        srcViews.push_back(std::move(srcView));
    }

    // For each copied layer, blit from the source into the destination.
    for (uint32_t z = 0; z < copyExtent.depthOrArrayLayers; ++z) {
        Ref<BindGroupBase> bindGroup;
        {
            BindGroupEntry bgEntry = {};
            bgEntry.binding = 0;
            bgEntry.textureView = srcViews[z].Get();

            BindGroupDescriptor bgDesc = {};
            bgDesc.layout = bgl.Get();
            bgDesc.entryCount = 1;
            bgDesc.entries = &bgEntry;
            DAWN_TRY_ASSIGN(bindGroup,
                            device->CreateBindGroup(&bgDesc, UsageValidationMode::Internal));
        }

        Ref<TextureViewBase> dstView;
        {
            TextureViewDescriptor viewDesc = {};
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.baseArrayLayer = dst.origin.z + z;
            viewDesc.arrayLayerCount = 1;
            viewDesc.baseMipLevel = dst.mipLevel;
            viewDesc.mipLevelCount = 1;
            DAWN_TRY_ASSIGN(dstView, dst.texture->CreateView(&viewDesc));
        }

        RenderPassDepthStencilAttachment dsAttachment = {};
        dsAttachment.view = dstView.Get();
        dsAttachment.depthClearValue = 0.0;
        dsAttachment.depthLoadOp = wgpu::LoadOp::Load;
        dsAttachment.depthStoreOp = wgpu::StoreOp::Store;
        if (dst.texture->GetFormat().HasStencil()) {
            dsAttachment.stencilLoadOp = wgpu::LoadOp::Load;
            dsAttachment.stencilStoreOp = wgpu::StoreOp::Store;
        }

        RenderPassDescriptor rpDesc = {};
        rpDesc.depthStencilAttachment = &dsAttachment;

        // Draw to perform the blit.
        Ref<RenderPassEncoder> pass = commandEncoder->BeginRenderPass(&rpDesc);
        pass->APISetBindGroup(0, bindGroup.Get());
        pass->APISetPipeline(pipeline.Get());
        pass->APIDraw(3, 1, 0, 0);
        pass->End();
    }

    return {};
}

}  // namespace dawn::native
