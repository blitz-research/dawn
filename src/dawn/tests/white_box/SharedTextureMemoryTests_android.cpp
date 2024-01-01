// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <android/hardware_buffer.h>
#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/tests/white_box/SharedTextureMemoryTests.h"
#include "dawn/utils/WGPUHelpers.h"
#include "dawn/webgpu_cpp.h"

namespace dawn {
namespace {

class Backend : public SharedTextureMemoryTestVulkanBackend {
  public:
    static SharedTextureMemoryTestBackend* GetInstance() {
        static Backend b;
        return &b;
    }

    std::string Name() const override { return "AHardwareBuffer"; }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter&) const override {
        return {wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer,
                wgpu::FeatureName::SharedFenceVkSemaphoreSyncFD};
    }

    static std::string MakeLabel(const AHardwareBuffer_Desc& desc) {
        std::string label = std::to_string(desc.width) + "x" + std::to_string(desc.height);
        switch (desc.format) {
            case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
                label += " R8G8B8A8_UNORM";
                break;
            case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
                label += " R8G8B8X8_UNORM";
                break;
            case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
                label += " R16G16B16A16_FLOAT";
                break;
            case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
                label += " R10G10B10A2_UNORM";
                break;
            case AHARDWAREBUFFER_FORMAT_R8_UNORM:
                label += " R8_UNORM";
                break;
        }
        if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) {
            label += " GPU_FRAMEBUFFER";
        }
        if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) {
            label += " GPU_SAMPLED_IMAGE";
        }
        return label;
    }

    template <typename CreateFn>
    auto CreateSharedTextureMemoryHelper(uint32_t size,
                                         AHardwareBuffer_Format format,
                                         AHardwareBuffer_UsageFlags usage,
                                         CreateFn createFn) {
        wgpu::SharedTextureMemoryDescriptor desc;
        AHardwareBuffer_Desc aHardwareBufferDesc = {};
        aHardwareBufferDesc.width = size;
        aHardwareBufferDesc.height = size;
        aHardwareBufferDesc.layers = 1;
        aHardwareBufferDesc.format = format;
        aHardwareBufferDesc.usage = usage;

        AHardwareBuffer* aHardwareBuffer;
        if (AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer) < 0) {
            // This combination of format / usage is not supported.
            // Unfortunately, AHardwareBuffer_isSupported requires API version 29.
            return decltype(createFn(desc)){};
        }

        wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
        stmAHardwareBufferDesc.handle = aHardwareBuffer;

        std::string label = MakeLabel(aHardwareBufferDesc);
        desc.label = label.c_str();
        desc.nextInChain = &stmAHardwareBufferDesc;

        auto ret = createFn(desc);
        AHardwareBuffer_release(aHardwareBuffer);

        return ret;
    }

    // Create one basic shared texture memory. It should support most operations.
    wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device) override {
        wgpu::SharedTextureMemory ret = CreateSharedTextureMemoryHelper(
            16, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
            static_cast<AHardwareBuffer_UsageFlags>(
                AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                AHARDWAREBUFFER_USAGE_CPU_READ_NEVER | AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER),
            [&](const wgpu::SharedTextureMemoryDescriptor& desc) {
                return device.ImportSharedTextureMemory(&desc);
            });
        EXPECT_NE(ret, nullptr);
        return ret;
    }

    std::vector<std::vector<wgpu::SharedTextureMemory>> CreatePerDeviceSharedTextureMemories(
        const std::vector<wgpu::Device>& devices) override {
        std::vector<std::vector<wgpu::SharedTextureMemory>> memories;

        for (auto format : {
                 AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                 AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
                 AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT,
                 AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM,
                 AHARDWAREBUFFER_FORMAT_R8_UNORM,
             }) {
            for (auto usage : {
                     static_cast<AHardwareBuffer_UsageFlags>(
                         AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER |
                         AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                         AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                         AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER),
                 }) {
                for (uint32_t size : {4, 64}) {
                    std::vector<wgpu::SharedTextureMemory> perDeviceMemories =
                        CreateSharedTextureMemoryHelper(
                            size, format, usage,
                            [&](const wgpu::SharedTextureMemoryDescriptor& desc) {
                                std::vector<wgpu::SharedTextureMemory> perDeviceMemories;
                                for (auto& device : devices) {
                                    auto stm = device.ImportSharedTextureMemory(&desc);
                                    if (!stm) {
                                        // The format/usage is not supported.
                                        // It won't be supported on any device, so break.
                                        return std::vector<wgpu::SharedTextureMemory>{};
                                    }
                                    perDeviceMemories.push_back(std::move(stm));
                                }
                                return perDeviceMemories;
                            });
                    if (!perDeviceMemories.empty()) {
                        memories.push_back(std::move(perDeviceMemories));
                    }
                }
            }
        }
        return memories;
    }
};

// Test clearing the texture memory on the device, then reading it on the CPU.
TEST_P(SharedTextureMemoryTests, GPUWriteThenCPURead) {
    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        .usage = AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER | AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Get actual desc for allocated buffer so we know the stride for cpu data.
    AHardwareBuffer_describe(aHardwareBuffer, &aHardwareBufferDesc);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);
    wgpu::Texture texture = memory.CreateTexture();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView()});
    passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
    passDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    passDescriptor.cColorAttachments[0].clearValue = {0.5001, 1.0, 0.2501, 1.0};

    encoder.BeginRenderPass(&passDescriptor).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    wgpu::SharedTextureMemoryVkImageLayoutBeginState beginLayout{};
    beginLayout.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    beginLayout.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    beginDesc.nextInChain = &beginLayout;
    memory.BeginAccess(texture, &beginDesc);

    device.GetQueue().Submit(1, &commandBuffer);

    wgpu::SharedTextureMemoryEndAccessState endState = {};
    wgpu::SharedTextureMemoryVkImageLayoutEndState endLayout{};
    endState.nextInChain = &endLayout;
    memory.EndAccess(texture, &endState);

    wgpu::SharedFenceExportInfo exportInfo;
    wgpu::SharedFenceVkSemaphoreSyncFDExportInfo syncFdExportInfo;
    exportInfo.nextInChain = &syncFdExportInfo;

    endState.fences[0].ExportInfo(&exportInfo);

    void* ptr;
    EXPECT_EQ(AHardwareBuffer_lock(aHardwareBuffer, AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN,
                                   syncFdExportInfo.handle, nullptr, &ptr),
              0);

    auto* pixels = static_cast<utils::RGBA8*>(ptr);
    for (uint32_t r = 0; r < aHardwareBufferDesc.height; ++r) {
        for (uint32_t c = 0; c < aHardwareBufferDesc.width; ++c) {
            EXPECT_EQ(pixels[r * aHardwareBufferDesc.stride + c], utils::RGBA8(128, 255, 64, 255))
                << r << ", " << c;
        }
    }

    EXPECT_EQ(AHardwareBuffer_unlock(aHardwareBuffer, nullptr), 0);
    AHardwareBuffer_release(aHardwareBuffer);
}

// Test writing the memory on the CPU, then sampling on the device.
TEST_P(SharedTextureMemoryTests, CPUWriteThenGPURead) {
    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        .usage = AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Get actual desc for allocated buffer so we know the stride for cpu data.
    AHardwareBuffer_describe(aHardwareBuffer, &aHardwareBufferDesc);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);
    wgpu::Texture texture = memory.CreateTexture();

    void* ptr;
    EXPECT_EQ(AHardwareBuffer_lock(aHardwareBuffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, -1,
                                   nullptr, &ptr),
              0);

    std::array<utils::RGBA8, 16> expected = {{
        utils::RGBA8::kRed,
        utils::RGBA8::kGreen,
        utils::RGBA8::kBlue,
        utils::RGBA8::kYellow,
        utils::RGBA8::kGreen,
        utils::RGBA8::kBlue,
        utils::RGBA8::kYellow,
        utils::RGBA8::kRed,
        utils::RGBA8::kBlue,
        utils::RGBA8::kYellow,
        utils::RGBA8::kRed,
        utils::RGBA8::kGreen,
        utils::RGBA8::kYellow,
        utils::RGBA8::kRed,
        utils::RGBA8::kGreen,
        utils::RGBA8::kBlue,
    }};

    auto* pixels = static_cast<utils::RGBA8*>(ptr);
    for (uint32_t r = 0; r < aHardwareBufferDesc.height; ++r) {
        for (uint32_t c = 0; c < aHardwareBufferDesc.width; ++c) {
            pixels[r * aHardwareBufferDesc.stride + c] =
                expected[r * aHardwareBufferDesc.width + c];
        }
    }

    EXPECT_EQ(AHardwareBuffer_unlock(aHardwareBuffer, nullptr), 0);
    AHardwareBuffer_release(aHardwareBuffer);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = true;
    wgpu::SharedTextureMemoryVkImageLayoutBeginState beginLayout{};
    beginDesc.nextInChain = &beginLayout;

    memory.BeginAccess(texture, &beginDesc);
    EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0},
                      {aHardwareBufferDesc.width, aHardwareBufferDesc.height});
}

DAWN_INSTANTIATE_PREFIXED_TEST_P(Vulkan,
                                 SharedTextureMemoryNoFeatureTests,
                                 {VulkanBackend()},
                                 {Backend::GetInstance()});

DAWN_INSTANTIATE_PREFIXED_TEST_P(Vulkan,
                                 SharedTextureMemoryTests,
                                 {VulkanBackend()},
                                 {Backend::GetInstance()});

}  // anonymous namespace
}  // namespace dawn
