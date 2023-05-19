// Copyright 2022 The Dawn Authors
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

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 16;
constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

using RequireShaderF16Feature = bool;
DAWN_TEST_PARAM_STRUCT(ShaderF16TestsParams, RequireShaderF16Feature);

class ShaderF16Tests : public DawnTestWithParams<ShaderF16TestsParams> {
  public:
    wgpu::Texture CreateDefault2DTexture() {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kRTSize;
        descriptor.size.height = kRTSize;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = 1;
        descriptor.format = kFormat;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        return device.CreateTexture(&descriptor);
    }

  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        mIsShaderF16SupportedOnAdapter = SupportsFeatures({wgpu::FeatureName::ShaderF16});
        if (!mIsShaderF16SupportedOnAdapter) {
            return {};
        }

        if (!IsD3D12()) {
            mUseDxcEnabledOrNonD3D12 = true;
        } else {
            for (auto* enabledToggle : GetParam().forceEnabledWorkarounds) {
                if (strncmp(enabledToggle, "use_dxc", 7) == 0) {
                    mUseDxcEnabledOrNonD3D12 = true;
                    break;
                }
            }
        }

        if (GetParam().mRequireShaderF16Feature && mUseDxcEnabledOrNonD3D12) {
            return {wgpu::FeatureName::ShaderF16};
        }

        return {};
    }

    bool IsShaderF16SupportedOnAdapter() const { return mIsShaderF16SupportedOnAdapter; }
    bool UseDxcEnabledOrNonD3D12() const { return mUseDxcEnabledOrNonD3D12; }

  private:
    bool mIsShaderF16SupportedOnAdapter = false;
    bool mUseDxcEnabledOrNonD3D12 = false;
};

// Test simple f16 arithmetic within shader with enable directive. The result should be as expect if
// device enable f16 extension, otherwise a shader creation error should be caught.
TEST_P(ShaderF16Tests, BasicShaderF16FeaturesTest) {
    const char* computeShader = R"(
        enable f16;

        struct Buf {
            v : f32,
        }
        @group(0) @binding(0) var<storage, read_write> buf : Buf;

        @compute @workgroup_size(1)
        fn CSMain() {
            let a : f16 = f16(buf.v) + 1.0h;
            buf.v = f32(a);
        }
    )";

    const bool shouldShaderF16FeatureSupportedByDevice =
        // Required when creating device
        GetParam().mRequireShaderF16Feature &&
        // Adapter support the feature
        IsShaderF16SupportedOnAdapter() &&
        // Proper toggle, allow_unsafe_apis and use_dxc if d3d12
        // Note that "allow_unsafe_apis" is always enabled in DawnTestBase::CreateDeviceImpl.
        HasToggleEnabled("allow_unsafe_apis") && UseDxcEnabledOrNonD3D12();
    const bool deviceSupportShaderF16Feature = device.HasFeature(wgpu::FeatureName::ShaderF16);
    EXPECT_EQ(deviceSupportShaderF16Feature, shouldShaderF16FeatureSupportedByDevice);

    if (!deviceSupportShaderF16Feature) {
        ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, computeShader));
        return;
    }

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4u;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer bufferOut = device.CreateBuffer(&bufferDesc);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, computeShader);
    csDesc.compute.entryPoint = "CSMain";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, bufferOut},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t expected[] = {0x3f800000};  // 1.0f
    EXPECT_BUFFER_U32_RANGE_EQ(expected, bufferOut, 0, 1);
}

// Test that fragment shader use f16 vector type as render target output.
TEST_P(ShaderF16Tests, RenderPipelineIOF16_RenderTarget) {
    // Skip if device don't support f16 extension.
    DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::ShaderF16));

    const char* shader = R"(
enable f16;

@vertex
fn VSMain(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
    var pos = array(
        vec2f(-1.0,  1.0),
        vec2f( 1.0, -1.0),
        vec2f(-1.0, -1.0));

    return vec4f(pos[VertexIndex], 0.0, 1.0);
}

@fragment
fn FSMain() -> @location(0) vec4<f16> {
    // Paint it blue
    return vec4<f16>(0.0, 0.0, 1.0, 1.0);
})";

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader);

    // Create render pipeline.
    wgpu::RenderPipeline pipeline;
    {
        utils::ComboRenderPipelineDescriptor descriptor;

        descriptor.vertex.module = shaderModule;
        descriptor.vertex.entryPoint = "VSMain";

        descriptor.cFragment.module = shaderModule;
        descriptor.cFragment.entryPoint = "FSMain";
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cTargets[0].format = kFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);
    }

    wgpu::Texture renderTarget = CreateDefault2DTexture();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    {
        // In the render pass we clear renderTarget to red and draw a blue triangle in the
        // bottom left of renderTarget1.
        utils::ComboRenderPassDescriptor renderPass({renderTarget.CreateView()});
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Validate that bottom left of render target is drawed to blue while upper right is still red
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderTarget, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderTarget, kRTSize - 1, 1);
}

// Test using f16 types as vertex shader (user-defined) output and fragment shader
// (user-defined) input.
TEST_P(ShaderF16Tests, RenderPipelineIOF16_InterstageVariable) {
    // Skip if device don't support f16 extension.
    DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::ShaderF16));

    const char* shader = R"(
enable f16;

struct VSOutput{
    @builtin(position)
    pos: vec4f,
    @location(3)
    color_vsout: vec4<f16>,
}

@vertex
fn VSMain(@builtin(vertex_index) VertexIndex : u32) -> VSOutput {
    var pos = array(
        vec2f(-1.0,  1.0),
        vec2f( 1.0, -1.0),
        vec2f(-1.0, -1.0));

    // Blue
    var color = vec4<f16>(0.0h, 0.0h, 1.0h, 1.0h);

    var result: VSOutput;
    result.pos = vec4f(pos[VertexIndex], 0.0, 1.0);
    result.color_vsout = color;

    return result;
}

struct FSInput{
    @location(3)
    color_fsin: vec4<f16>,
}

@fragment
fn FSMain(fsInput: FSInput) -> @location(0) vec4f {
    // Paint it with given color
    return vec4f(fsInput.color_fsin);
})";

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader);

    // Create render pipeline.
    wgpu::RenderPipeline pipeline;
    {
        utils::ComboRenderPipelineDescriptor descriptor;

        descriptor.vertex.module = shaderModule;
        descriptor.vertex.entryPoint = "VSMain";

        descriptor.cFragment.module = shaderModule;
        descriptor.cFragment.entryPoint = "FSMain";
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cTargets[0].format = kFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);
    }

    wgpu::Texture renderTarget = CreateDefault2DTexture();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    {
        // In the first render pass we clear renderTarget1 to red and draw a blue triangle in the
        // bottom left of renderTarget1.
        utils::ComboRenderPassDescriptor renderPass({renderTarget.CreateView()});
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Validate that bottom left of render target is drawed to blue while upper right is still red
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderTarget, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderTarget, kRTSize - 1, 1);
}

// Test using f16 types as vertex shader user-defined input (vertex attributes), draw points of
// different color given as vertex attributes.
TEST_P(ShaderF16Tests, RenderPipelineIOF16_VertexAttribute) {
    // Skip if device don't support f16 extension.
    DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::ShaderF16));

    const char* shader = R"(
enable f16;

struct VSInput {
    // position / 2.0
    @location(0) pos_half : vec2<f16>,
    // color / 4.0
    @location(1) color_quarter : vec4<f16>,
}

struct VSOutput {
    @builtin(position) pos : vec4f,
    @location(0) color : vec4f,
}

@vertex
fn VSMain(in: VSInput) -> VSOutput {
    return VSOutput(vec4f(vec2f(in.pos_half * 2.0h), 0.0, 1.0), vec4f(in.color_quarter * 4.0h));
}

@fragment
fn FSMain(@location(0) color : vec4f) -> @location(0) vec4f {
    return color;
})";

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader);

    constexpr uint32_t kPointCount = 8;

    // Position (divided by 2.0) for points on horizontal line
    std::vector<float> positionData;
    constexpr float xStep = 2.0 / kPointCount;
    constexpr float xBias = -1.0 + xStep / 2.0f;
    for (uint32_t i = 0; i < kPointCount; i++) {
        // X position, divided by 2.0
        positionData.push_back((xBias + xStep * i) / 2.0f);
        // Y position (0.0f) divided by 2.0
        positionData.push_back(0.0f);
    }

    // Expected color for each point
    using RGBA8 = utils::RGBA8;
    std::vector<RGBA8> colors = {
        RGBA8::kBlack,
        RGBA8::kRed,
        RGBA8::kGreen,
        RGBA8::kBlue,
        RGBA8::kYellow,
        RGBA8::kWhite,
        RGBA8(96, 192, 176, 255),
        RGBA8(184, 108, 184, 255),
    };

    ASSERT(colors.size() == kPointCount);
    // Color (divided by 4.0) for each point
    std::vector<float> colorData;
    for (RGBA8& color : colors) {
        colorData.push_back(color.r / 255.0 / 4.0);
        colorData.push_back(color.g / 255.0 / 4.0);
        colorData.push_back(color.b / 255.0 / 4.0);
        colorData.push_back(color.a / 255.0 / 4.0);
    }

    // Store the data as float32x2 and float32x4 in vertex buffer, which should be convert to
    // corresponding WGSL type vec2<f16> and vec4<f16> by driver.
    // Buffer for pos_half
    wgpu::Buffer vertexBufferPos = utils::CreateBufferFromData(
        device, positionData.data(), 2 * kPointCount * sizeof(float), wgpu::BufferUsage::Vertex);
    // Buffer for color_quarter
    wgpu::Buffer vertexBufferColor = utils::CreateBufferFromData(
        device, colorData.data(), 4 * kPointCount * sizeof(float), wgpu::BufferUsage::Vertex);

    // Create render pipeline.
    wgpu::RenderPipeline pipeline;
    {
        utils::ComboRenderPipelineDescriptor descriptor;

        descriptor.vertex.module = shaderModule;
        descriptor.vertex.entryPoint = "VSMain";
        descriptor.vertex.bufferCount = 2;
        // Interprete the vertex buffer data as Float32x2 and Float32x4, and the result should be
        // converted to vec2<f16> and vec4<f16>
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x2;
        descriptor.cAttributes[0].offset = 0;
        descriptor.cAttributes[0].shaderLocation = 0;
        descriptor.cBuffers[0].stepMode = wgpu::VertexStepMode::Vertex;
        descriptor.cBuffers[0].arrayStride = 8;
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cBuffers[0].attributes = &descriptor.cAttributes[0];
        descriptor.cAttributes[1].format = wgpu::VertexFormat::Float32x4;
        descriptor.cAttributes[1].offset = 0;
        descriptor.cAttributes[1].shaderLocation = 1;
        descriptor.cBuffers[1].stepMode = wgpu::VertexStepMode::Vertex;
        descriptor.cBuffers[1].arrayStride = 16;
        descriptor.cBuffers[1].attributeCount = 1;
        descriptor.cBuffers[1].attributes = &descriptor.cAttributes[1];

        descriptor.cFragment.module = shaderModule;
        descriptor.cFragment.entryPoint = "FSMain";
        descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
        descriptor.cTargets[0].format = kFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);
    }

    // Create a render target of horizontal line
    wgpu::Texture renderTarget;
    {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kPointCount;
        descriptor.size.height = 1;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = 1;
        descriptor.format = kFormat;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        renderTarget = device.CreateTexture(&descriptor);
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    {
        // Clear renderTarget to zero and draw points.
        utils::ComboRenderPassDescriptor renderPass({renderTarget.CreateView()});
        renderPass.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBufferPos);
        pass.SetVertexBuffer(1, vertexBufferColor);
        pass.Draw(kPointCount);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Validate the color of each point
    for (uint32_t i = 0; i < kPointCount; i++) {
        EXPECT_PIXEL_RGBA8_EQ(colors[i], renderTarget, i, 0);
    }
}

// DawnTestBase::CreateDeviceImpl always enables allow_unsafe_apis toggle.
DAWN_INSTANTIATE_TEST_P(ShaderF16Tests,
                        {
                            D3D12Backend(),
                            D3D12Backend({"use_dxc"}),
                            VulkanBackend(),
                            MetalBackend(),
                            OpenGLBackend(),
                            OpenGLESBackend(),
                        },
                        {true, false});

}  // anonymous namespace
}  // namespace dawn
