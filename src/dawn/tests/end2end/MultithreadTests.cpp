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

#include <atomic>
#include <condition_variable>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

template <typename Step>
class LockStep {
  public:
    LockStep() = delete;
    explicit LockStep(Step startStep) : mStep(startStep) {}

    void Signal(Step step) {
        std::lock_guard<std::mutex> lg(mMutex);
        mStep = step;
        mCv.notify_all();
    }

    void Wait(Step step) {
        std::unique_lock<std::mutex> lg(mMutex);
        mCv.wait(lg, [=] { return mStep == step; });
    }

  private:
    Step mStep;
    std::mutex mMutex;
    std::condition_variable mCv;
};

class MultithreadTests : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
        if (!UsesWire()) {
            features.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
        }
        return features;
    }

    void SetUp() override {
        DawnTest::SetUp();
        // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        // TODO(crbug.com/dawn/1679): OpenGL backend doesn't support thread safe API yet.
        DAWN_TEST_UNSUPPORTED_IF(IsOpenGL() || IsOpenGLES());
    }

    wgpu::Buffer CreateBuffer(uint32_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;
        return device.CreateBuffer(&descriptor);
    }

    wgpu::Texture CreateTexture(uint32_t width,
                                uint32_t height,
                                wgpu::TextureFormat format,
                                wgpu::TextureUsage usage,
                                uint32_t mipLevelCount = 1,
                                uint32_t sampleCount = 1) {
        wgpu::TextureDescriptor texDescriptor = {};
        texDescriptor.size = {width, height, 1};
        texDescriptor.format = format;
        texDescriptor.usage = usage;
        texDescriptor.mipLevelCount = mipLevelCount;
        texDescriptor.sampleCount = sampleCount;
        return device.CreateTexture(&texDescriptor);
    }
};

// Test that dropping a device's last ref on another thread won't crash Instance::ProcessEvents.
TEST_P(MultithreadTests, Device_DroppedOnAnotherThread) {
    // TODO(crbug.com/dawn/1779): This test seems to cause flakiness in other sampling tests on
    // NVIDIA.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());

    std::vector<wgpu::Device> devices(5);

    // Create devices.
    for (size_t i = 0; i < devices.size(); ++i) {
        devices[i] = CreateDevice();
    }

    std::atomic<uint32_t> numAliveDevices = static_cast<uint32_t>(devices.size());

    // Create threads
    utils::RunInParallel(
        numAliveDevices.load(),
        [&devices, &numAliveDevices](uint32_t index) {
            EXPECT_NE(devices[index].Get(), nullptr);

            // Drop device.
            devices[index] = nullptr;

            numAliveDevices--;
        },
        [this, &numAliveDevices] {
            while (numAliveDevices.load() > 0) {
                // main thread process events from all devices
                WaitABit();
            }
        });
}

// Test that dropping a device's last ref inside a callback on another thread won't crash
// Instance::ProcessEvents.
TEST_P(MultithreadTests, Device_DroppedInCallback_OnAnotherThread) {
    // TODO(crbug.com/dawn/1779): This test seems to cause flakiness in other sampling tests on
    // NVIDIA.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());

    std::vector<wgpu::Device> devices(10);

    // Create devices.
    for (auto& device : devices) {
        device = CreateDevice();
    }

    // Create threads
    utils::RunInParallel(static_cast<uint32_t>(devices.size()), [&devices, this](uint32_t index) {
        auto additionalDevice = std::move(devices[index]);
        struct UserData {
            wgpu::Device device2ndRef;
            std::atomic_bool isCompleted{false};
        } userData;

        userData.device2ndRef = additionalDevice;

        // Drop the last ref inside a callback.
        additionalDevice.PushErrorScope(wgpu::ErrorFilter::Validation);
        additionalDevice.PopErrorScope(
            [](WGPUErrorType type, const char*, void* userdataPtr) {
                auto userdata = static_cast<UserData*>(userdataPtr);
                userdata->device2ndRef = nullptr;
                userdata->isCompleted = true;
            },
            &userData);
        // main ref dropped.
        additionalDevice = nullptr;

        do {
            WaitABit();
        } while (!userData.isCompleted.load());

        EXPECT_EQ(userData.device2ndRef, nullptr);
    });
}

// Test that multiple buffers being created and mapped on multiple threads won't interfere with
// each other.
TEST_P(MultithreadTests, Buffers_MapInParallel) {
    constexpr uint32_t kDataSize = 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    constexpr uint32_t kSize = static_cast<uint32_t>(kDataSize * sizeof(uint32_t));

    utils::RunInParallel(10, [=, &myData = std::as_const(myData)](uint32_t) {
        wgpu::Buffer buffer;
        std::atomic<bool> mapCompleted(false);

        // Create buffer and request mapping.
        buffer = CreateBuffer(kSize, wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc);

        buffer.MapAsync(
            wgpu::MapMode::Write, 0, kSize,
            [](WGPUBufferMapAsyncStatus status, void* userdata) {
                EXPECT_EQ(WGPUBufferMapAsyncStatus_Success, status);
                (*static_cast<std::atomic<bool>*>(userdata)) = true;
            },
            &mapCompleted);

        // Wait for the mapping to complete
        while (!mapCompleted.load()) {
            device.Tick();
            FlushWire();
        }

        // Buffer is mapped, write into it and unmap .
        memcpy(buffer.GetMappedRange(0, kSize), myData.data(), kSize);
        buffer.Unmap();

        // Check the content of the buffer.
        EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffer, 0, kDataSize);
    });
}

// Test CreateComputePipelineAsync on multiple threads.
TEST_P(MultithreadTests, CreateComputePipelineAsyncInParallel) {
    // TODO(crbug.com/dawn/1766): TSAN reported race conditions in NVIDIA's vk driver.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia() && IsTsan());

    std::vector<wgpu::ComputePipeline> pipelines(10);
    std::vector<std::string> shaderSources(pipelines.size());
    std::vector<uint32_t> expectedValues(shaderSources.size());

    for (uint32_t i = 0; i < pipelines.size(); ++i) {
        expectedValues[i] = i + 1;

        std::ostringstream ss;
        ss << R"(
        struct SSBO {
            value : u32
        }
        @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

        @compute @workgroup_size(1) fn main() {
            ssbo.value =
        )";
        ss << expectedValues[i];
        ss << ";}";

        shaderSources[i] = ss.str();
    }

    // Create pipelines in parallel
    utils::RunInParallel(static_cast<uint32_t>(pipelines.size()), [&](uint32_t index) {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shaderSources[index].c_str());
        csDesc.compute.entryPoint = "main";

        struct Task {
            wgpu::ComputePipeline computePipeline;
            std::atomic<bool> isCompleted{false};
        } task;
        device.CreateComputePipelineAsync(
            &csDesc,
            [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
               const char* message, void* userdata) {
                EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Success,
                          status);

                auto task = static_cast<Task*>(userdata);
                task->computePipeline = wgpu::ComputePipeline::Acquire(returnPipeline);
                task->isCompleted = true;
            },
            &task);

        while (!task.isCompleted.load()) {
            WaitABit();
        }

        pipelines[index] = task.computePipeline;
    });

    // Verify pipelines' executions
    for (uint32_t i = 0; i < pipelines.size(); ++i) {
        wgpu::Buffer ssbo =
            CreateBuffer(sizeof(uint32_t), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

            ASSERT_NE(nullptr, pipelines[i].Get());
            wgpu::BindGroup bindGroup =
                utils::MakeBindGroup(device, pipelines[i].GetBindGroupLayout(0),
                                     {
                                         {0, ssbo, 0, sizeof(uint32_t)},
                                     });
            pass.SetBindGroup(0, bindGroup);
            pass.SetPipeline(pipelines[i]);

            pass.DispatchWorkgroups(1);
            pass.End();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(expectedValues[i], ssbo, 0);
    }
}

// Test CreateRenderPipelineAsync on multiple threads.
TEST_P(MultithreadTests, CreateRenderPipelineAsyncInParallel) {
    // TODO(crbug.com/dawn/1766): TSAN reported race conditions in NVIDIA's vk driver.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia() && IsTsan());

    constexpr uint32_t kNumThreads = 10;
    constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr uint8_t kColorStep = 250 / kNumThreads;

    std::vector<wgpu::RenderPipeline> pipelines(kNumThreads);
    std::vector<std::string> fragmentShaderSources(kNumThreads);
    std::vector<utils::RGBA8> minExpectedValues(kNumThreads);
    std::vector<utils::RGBA8> maxExpectedValues(kNumThreads);

    for (uint32_t i = 0; i < kNumThreads; ++i) {
        // Due to floating point precision, we need to use min & max values to compare the
        // expectations.
        auto expectedGreen = kColorStep * i;
        minExpectedValues[i] =
            utils::RGBA8(0, expectedGreen == 0 ? 0 : (expectedGreen - 2), 0, 255);
        maxExpectedValues[i] =
            utils::RGBA8(0, expectedGreen == 255 ? 255 : (expectedGreen + 2), 0, 255);

        std::ostringstream ss;
        ss << R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0,
        )";
        ss << expectedGreen / 255.0;
        ss << ", 0.0, 1.0);}";

        fragmentShaderSources[i] = ss.str();
    }

    // Create pipelines in parallel
    utils::RunInParallel(kNumThreads, [&](uint32_t index) {
        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
        wgpu::ShaderModule fsModule =
            utils::CreateShaderModule(device, fragmentShaderSources[index].c_str());
        renderPipelineDescriptor.vertex.module = vsModule;
        renderPipelineDescriptor.cFragment.module = fsModule;
        renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
        renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

        struct Task {
            wgpu::RenderPipeline renderPipeline;
            std::atomic<bool> isCompleted{false};
        } task;
        device.CreateRenderPipelineAsync(
            &renderPipelineDescriptor,
            [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
               const char* message, void* userdata) {
                EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Success,
                          status);

                auto* task = static_cast<Task*>(userdata);
                task->renderPipeline = wgpu::RenderPipeline::Acquire(returnPipeline);
                task->isCompleted = true;
            },
            &task);

        while (!task.isCompleted) {
            WaitABit();
        }

        pipelines[index] = task.renderPipeline;
    });

    // Verify pipelines' executions
    for (uint32_t i = 0; i < pipelines.size(); ++i) {
        wgpu::Texture outputTexture =
            CreateTexture(1, 1, kRenderAttachmentFormat,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

        utils::ComboRenderPassDescriptor renderPassDescriptor({outputTexture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPassDescriptor.cColorAttachments[0].clearValue = {1.f, 0.f, 0.f, 1.f};

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder renderPassEncoder =
                encoder.BeginRenderPass(&renderPassDescriptor);

            ASSERT_NE(nullptr, pipelines[i].Get());

            renderPassEncoder.SetPipeline(pipelines[i]);
            renderPassEncoder.Draw(1);
            renderPassEncoder.End();
            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_BETWEEN(minExpectedValues[i], maxExpectedValues[i], outputTexture, 0, 0);
    }
}

class MultithreadCachingTests : public MultithreadTests {
  protected:
    wgpu::ShaderModule CreateComputeShaderModule() const {
        return utils::CreateShaderModule(device, R"(
            struct SSBO {
                value : u32
            }
            @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

            @compute @workgroup_size(1) fn main() {
                ssbo.value = 1;
            })");
    }

    wgpu::BindGroupLayout CreateComputeBindGroupLayout() const {
        return utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                    });
    }
};

// Test that creating a same shader module (which will return the cached shader module) and release
// it on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedShaderModulesInParallel) {
    utils::RunInParallel(100, [this](uint32_t) {
        wgpu::ShaderModule csModule = CreateComputeShaderModule();
        EXPECT_NE(nullptr, csModule.Get());
    });
}

// Test that creating a same compute pipeline (which will return the cached pipeline) and release it
// on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedComputePipelinesInParallel) {
    wgpu::ShaderModule csModule = CreateComputeShaderModule();
    wgpu::BindGroupLayout bglayout = CreateComputeBindGroupLayout();
    wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, {bglayout});

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = csModule;
    csDesc.compute.entryPoint = "main";
    csDesc.layout = pipelineLayout;

    utils::RunInParallel(100, [&, this](uint32_t) {
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);
        EXPECT_NE(nullptr, pipeline.Get());
    });
}

// Test that creating a same bind group layout (which will return the cached layout) and
// release it on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedBindGroupLayoutsInParallel) {
    utils::RunInParallel(100, [&, this](uint32_t) {
        wgpu::BindGroupLayout layout = CreateComputeBindGroupLayout();
        EXPECT_NE(nullptr, layout.Get());
    });
}

// Test that creating a same pipeline layout (which will return the cached layout) and
// release it on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedPipelineLayoutsInParallel) {
    wgpu::BindGroupLayout bglayout = CreateComputeBindGroupLayout();

    utils::RunInParallel(100, [&, this](uint32_t) {
        wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, {bglayout});
        EXPECT_NE(nullptr, pipelineLayout.Get());
    });
}

// Test that creating a same render pipeline (which will return the cached pipeline) and release it
// on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedRenderPipelinesInParallel) {
    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    utils::RunInParallel(100, [&, this](uint32_t) {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);
        EXPECT_NE(nullptr, pipeline.Get());
    });
}

// Test that creating a same sampler pipeline (which will return the cached sampler) and release it
// on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedSamplersInParallel) {
    wgpu::SamplerDescriptor desc = {};
    utils::RunInParallel(100, [&, this](uint32_t) {
        wgpu::Sampler sampler = device.CreateSampler(&desc);
        EXPECT_NE(nullptr, sampler.Get());
    });
}

class MultithreadEncodingTests : public MultithreadTests {};

// Test that encoding render passes in parallel should work
TEST_P(MultithreadEncodingTests, RenderPassEncodersInParallel) {
    constexpr uint32_t kRTSize = 16;
    constexpr uint32_t kNumThreads = 10;

    wgpu::Texture msaaRenderTarget =
        CreateTexture(kRTSize, kRTSize, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
                      /*mipLevelCount=*/1, /*sampleCount=*/4);
    wgpu::TextureView msaaRenderTargetView = msaaRenderTarget.CreateView();

    wgpu::Texture resolveTarget =
        CreateTexture(kRTSize, kRTSize, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);
    wgpu::TextureView resolveTargetView = resolveTarget.CreateView();

    std::vector<wgpu::CommandBuffer> commandBuffers(kNumThreads);

    utils::RunInParallel(kNumThreads, [=, &commandBuffers](uint32_t index) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Clear the renderTarget to red.
        utils::ComboRenderPassDescriptor renderPass({msaaRenderTargetView});
        renderPass.cColorAttachments[0].resolveTarget = resolveTargetView;
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        commandBuffers[index] = encoder.Finish();
    });

    // Verify that the command buffers executed correctly.
    for (auto& commandBuffer : commandBuffers) {
        queue.Submit(1, &commandBuffer);

        EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, resolveTarget, {0, 0});
        EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, resolveTarget, {kRTSize - 1, kRTSize - 1});
    }
}

// Test that encoding render passes that resolve to a mip level in parallel should work
TEST_P(MultithreadEncodingTests, RenderPassEncoders_ResolveToMipLevelOne_InParallel) {
    // TODO(dawn:462): Issue in the D3D12 validation layers.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    constexpr uint32_t kRTSize = 16;
    constexpr uint32_t kNumThreads = 10;

    wgpu::Texture msaaRenderTarget =
        CreateTexture(kRTSize, kRTSize, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
                      /*mipLevelCount=*/1, /*sampleCount=*/4);
    wgpu::TextureView msaaRenderTargetView = msaaRenderTarget.CreateView();

    // Resolve to mip level = 1 to force render pass workarounds (there shouldn't be any deadlock
    // happening).
    wgpu::Texture resolveTarget =
        CreateTexture(kRTSize * 2, kRTSize * 2, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
                      /*mipLevelCount=*/2, /*sampleCount=*/1);
    wgpu::TextureViewDescriptor resolveTargetViewDesc;
    resolveTargetViewDesc.baseMipLevel = 1;
    resolveTargetViewDesc.mipLevelCount = 1;
    wgpu::TextureView resolveTargetView = resolveTarget.CreateView(&resolveTargetViewDesc);

    std::vector<wgpu::CommandBuffer> commandBuffers(kNumThreads);

    utils::RunInParallel(kNumThreads, [=, &commandBuffers](uint32_t index) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Clear the renderTarget to red.
        utils::ComboRenderPassDescriptor renderPass({msaaRenderTargetView});
        renderPass.cColorAttachments[0].resolveTarget = resolveTargetView;
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        commandBuffers[index] = encoder.Finish();
    });

    // Verify that the command buffers executed correctly.
    for (auto& commandBuffer : commandBuffers) {
        queue.Submit(1, &commandBuffer);

        EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, resolveTarget, {0, 0}, 1);
        EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, resolveTarget, {kRTSize - 1, kRTSize - 1}, 1);
    }
}

// Test that encoding compute passes in parallel should work
TEST_P(MultithreadEncodingTests, ComputePassEncodersInParallel) {
    constexpr uint32_t kNumThreads = 10;
    constexpr uint32_t kExpected = 0xFFFFFFFFu;

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var<storage, read_write> output : u32;

            @compute @workgroup_size(1, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
                output = 0xFFFFFFFFu;
            })");
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "main";
    auto pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::Buffer dstBuffer =
        CreateBuffer(sizeof(uint32_t), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc |
                                           wgpu::BufferUsage::CopyDst);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, dstBuffer, 0, sizeof(uint32_t)},
                                                     });

    std::vector<wgpu::CommandBuffer> commandBuffers(kNumThreads);

    utils::RunInParallel(kNumThreads, [=, &commandBuffers](uint32_t index) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1, 1, 1);
        pass.End();

        commandBuffers[index] = encoder.Finish();
    });

    // Verify that the command buffers executed correctly.
    for (auto& commandBuffer : commandBuffers) {
        constexpr uint32_t kSentinelData = 0;
        queue.WriteBuffer(dstBuffer, 0, &kSentinelData, sizeof(kSentinelData));
        queue.Submit(1, &commandBuffer);

        EXPECT_BUFFER_U32_EQ(kExpected, dstBuffer, 0);
    }
}

class MultithreadTextureCopyTests : public MultithreadTests {
  protected:
    void SetUp() override {
        MultithreadTests::SetUp();

        // TODO(crbug.com/dawn/1291): These tests are failing on GLES (both native and ANGLE)
        // when using Tint/GLSL.
        DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());
    }

    wgpu::Texture CreateAndWriteTexture(uint32_t width,
                                        uint32_t height,
                                        wgpu::TextureFormat format,
                                        wgpu::TextureUsage usage,
                                        const void* data,
                                        size_t dataSize) {
        auto texture = CreateTexture(width, height, format, wgpu::TextureUsage::CopyDst | usage);

        wgpu::Extent3D textureSize = {width, height, 1};

        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(texture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::TextureDataLayout textureDataLayout =
            utils::CreateTextureDataLayout(0, dataSize / height);

        queue.WriteTexture(&imageCopyTexture, data, dataSize, &textureDataLayout, &textureSize);

        return texture;
    }

    uint32_t BufferSizeForTextureCopy(uint32_t width, uint32_t height, wgpu::TextureFormat format) {
        uint32_t bytesPerRow = utils::GetMinimumBytesPerRow(format, width);
        return utils::RequiredBytesInCopy(bytesPerRow, height, {width, height, 1}, format);
    }

    void CopyTextureToTextureHelper(
        const wgpu::Texture& srcTexture,
        const wgpu::ImageCopyTexture& dst,
        const wgpu::Extent3D& dstSize,
        const wgpu::CommandEncoder& encoder,
        const wgpu::CopyTextureForBrowserOptions* copyForBrowerOptions = nullptr) {
        wgpu::ImageCopyTexture srcView =
            utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);

        if (copyForBrowerOptions == nullptr) {
            encoder.CopyTextureToTexture(&srcView, &dst, &dstSize);

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
        } else {
            // Don't need encoder
            ASSERT(encoder == nullptr);
            queue.CopyTextureForBrowser(&srcView, &dst, &dstSize, copyForBrowerOptions);
        }
    }

    void CopyBufferToTextureHelper(const wgpu::Buffer& srcBuffer,
                                   uint32_t srcBytesPerRow,
                                   const wgpu::ImageCopyTexture& dst,
                                   const wgpu::Extent3D& dstSize,
                                   const wgpu::CommandEncoder& encoder) {
        wgpu::ImageCopyBuffer srcView =
            utils::CreateImageCopyBuffer(srcBuffer, 0, srcBytesPerRow, dstSize.height);

        encoder.CopyBufferToTexture(&srcView, &dst, &dstSize);

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }
};

// Test that depth texture's CopyTextureToTexture() can work in parallel with other commands (such
// resources creation and texture to buffer copy for texture expectations).
// This test is needed since most of command encoder's commands are not synchronized, but
// CopyTextureToTexture() command might internally allocate resources and we need to make sure that
// it won't race with other threads' works.
TEST_P(MultithreadTextureCopyTests, CopyDepthToDepthNoRace) {
    enum class Step {
        Begin,
        WriteTexture,
    };

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    const std::vector<float> kExpectedData32 = {
        0,    0,    0,    0,  //
        0,    0,    0.4f, 0,  //
        1.0f, 1.0f, 0,    0,  //
        1.0f, 1.0f, 0,    0,  //
    };

    std::vector<uint16_t> kExpectedData16(kExpectedData32.size());
    for (size_t i = 0; i < kExpectedData32.size(); ++i) {
        kExpectedData16[i] = kExpectedData32[i] * std::numeric_limits<uint16_t>::max();
    }

    const size_t kExpectedDataSize16 = kExpectedData16.size() * sizeof(kExpectedData16[0]);

    LockStep<Step> lockStep(Step::Begin);

    wgpu::Texture depthTexture;
    std::thread writeThread([&] {
        depthTexture = CreateAndWriteTexture(
            kWidth, kHeight, wgpu::TextureFormat::Depth16Unorm,
            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment,
            kExpectedData16.data(), kExpectedDataSize16);

        lockStep.Signal(Step::WriteTexture);

        // Verify the initial data
        ExpectAttachmentDepthTestData(depthTexture, wgpu::TextureFormat::Depth16Unorm, kWidth,
                                      kHeight, 0, /*mipLevel=*/0, kExpectedData32);
    });

    std::thread copyThread([&] {
        auto destTexture =
            CreateTexture(kWidth * 2, kHeight * 2, wgpu::TextureFormat::Depth16Unorm,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc,
                          /*mipLevelCount=*/2);

        // Copy from depthTexture to destTexture.
        const wgpu::Extent3D dstSize = {kWidth, kHeight, 1};
        wgpu::ImageCopyTexture dest = utils::CreateImageCopyTexture(
            destTexture, /*dstMipLevel=*/1, {0, 0, 0}, wgpu::TextureAspect::All);
        auto encoder = device.CreateCommandEncoder();
        lockStep.Wait(Step::WriteTexture);
        CopyTextureToTextureHelper(depthTexture, dest, dstSize, encoder);

        // Verify the copied data
        ExpectAttachmentDepthTestData(destTexture, wgpu::TextureFormat::Depth16Unorm, kWidth,
                                      kHeight, 0, /*mipLevel=*/1, kExpectedData32);
    });

    writeThread.join();
    copyThread.join();
}

// Test that depth texture's CopyBufferToTexture() can work in parallel with other commands (such
// resources creation and texture to buffer copy for texture expectations).
// This test is needed since most of command encoder's commands are not synchronized, but
// CopyBufferToTexture() command might internally allocate resources and we need to make sure that
// it won't race with other threads' works.
TEST_P(MultithreadTextureCopyTests, CopyBufferToDepthNoRace) {
    enum class Step {
        Begin,
        WriteBuffer,
    };

    constexpr uint32_t kWidth = 16;
    constexpr uint32_t kHeight = 1;

    const std::vector<float> kExpectedData32 = {
        0,    0,    0,    0,  //
        0,    0,    0.4f, 0,  //
        1.0f, 1.0f, 0,    0,  //
        1.0f, 1.0f, 0,    0,  //
    };

    std::vector<uint16_t> kExpectedData16(kExpectedData32.size());
    for (size_t i = 0; i < kExpectedData32.size(); ++i) {
        kExpectedData16[i] = kExpectedData32[i] * std::numeric_limits<uint16_t>::max();
    }

    const uint32_t kExpectedDataSize16 = kExpectedData16.size() * sizeof(kExpectedData16[0]);

    const wgpu::Extent3D kSize = {kWidth, kHeight, 1};
    LockStep<Step> lockStep(Step::Begin);

    wgpu::Buffer buffer;
    std::thread writeThread([&] {
        buffer = CreateBuffer(
            BufferSizeForTextureCopy(kWidth, kHeight, wgpu::TextureFormat::Depth16Unorm),
            wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);

        queue.WriteBuffer(buffer, 0, kExpectedData16.data(), kExpectedDataSize16);
        device.Tick();

        lockStep.Signal(Step::WriteBuffer);

        EXPECT_BUFFER_U16_RANGE_EQ(kExpectedData16.data(), buffer, 0, kExpectedData16.size());
    });

    std::thread copyThread([&] {
        auto destTexture =
            CreateTexture(kWidth, kHeight, wgpu::TextureFormat::Depth16Unorm,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc);

        auto encoder = device.CreateCommandEncoder();

        wgpu::ImageCopyTexture dest = utils::CreateImageCopyTexture(
            destTexture, /*dstMipLevel=*/0, {0, 0, 0}, wgpu::TextureAspect::All);

        // Wait until src buffer is written.
        lockStep.Wait(Step::WriteBuffer);
        CopyBufferToTextureHelper(buffer, kTextureBytesPerRowAlignment, dest, kSize, encoder);

        // Verify the copied data
        ExpectAttachmentDepthTestData(destTexture, wgpu::TextureFormat::Depth16Unorm, kWidth,
                                      kHeight, 0, /*mipLevel=*/0, kExpectedData32);
    });

    writeThread.join();
    copyThread.join();
}

// Test that stencil texture's CopyTextureToTexture() can work in parallel with other commands (such
// resources creation and texture to buffer copy for texture expectations).
// This test is needed since most of command encoder's commands are not synchronized, but
// CopyTextureToTexture() command might internally allocate resources and we need to make sure that
// it won't race with other threads' works.
TEST_P(MultithreadTextureCopyTests, CopyStencilToStencilNoRace) {
    // TODO(crbug.com/dawn/1497): glReadPixels: GL error: HIGH: Invalid format and type
    // combination.
    DAWN_SUPPRESS_TEST_IF(IsANGLE());

    // TODO(crbug.com/dawn/667): Work around the fact that some platforms are unable to read
    // stencil.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_depth_stencil_read"));

    enum class Step {
        Begin,
        WriteTexture,
    };

    constexpr uint32_t kWidth = 1;
    constexpr uint32_t kHeight = 1;

    constexpr uint8_t kExpectedData = 177;
    constexpr size_t kExpectedDataSize = sizeof(kExpectedData);

    LockStep<Step> lockStep(Step::Begin);

    wgpu::Texture stencilTexture;
    std::thread writeThread([&] {
        stencilTexture = CreateAndWriteTexture(
            kWidth, kHeight, wgpu::TextureFormat::Stencil8,
            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment, &kExpectedData,
            kExpectedDataSize);

        lockStep.Signal(Step::WriteTexture);

        // Verify the initial data
        ExpectAttachmentStencilTestData(stencilTexture, wgpu::TextureFormat::Stencil8, kWidth,
                                        kHeight, 0, /*mipLevel=*/0, kExpectedData);
    });

    std::thread copyThread([&] {
        auto destTexture =
            CreateTexture(kWidth * 2, kHeight * 2, wgpu::TextureFormat::Stencil8,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc,
                          /*mipLevelCount=*/2);

        // Copy from stencilTexture to destTexture.
        const wgpu::Extent3D dstSize = {kWidth, kHeight, 1};
        wgpu::ImageCopyTexture dest = utils::CreateImageCopyTexture(
            destTexture, /*dstMipLevel=*/1, {0, 0, 0}, wgpu::TextureAspect::All);
        auto encoder = device.CreateCommandEncoder();
        lockStep.Wait(Step::WriteTexture);

        CopyTextureToTextureHelper(stencilTexture, dest, dstSize, encoder);

        // Verify the copied data
        ExpectAttachmentStencilTestData(destTexture, wgpu::TextureFormat::Stencil8, kWidth, kHeight,
                                        0, /*mipLevel=*/1, kExpectedData);
    });

    writeThread.join();
    copyThread.join();
}

// Test that stencil texture's CopyBufferToTexture() can work in parallel with other commands (such
// resources creation and texture to buffer copy for texture expectations).
// This test is needed since most of command encoder's commands are not synchronized, but
// CopyBufferToTexture() command might internally allocate resources and we need to make sure that
// it won't race with other threads' works.
TEST_P(MultithreadTextureCopyTests, CopyBufferToStencilNoRace) {
    enum class Step {
        Begin,
        WriteBuffer,
    };

    constexpr uint32_t kWidth = 1;
    constexpr uint32_t kHeight = 1;

    constexpr uint8_t kExpectedData = 177;

    const wgpu::Extent3D kSize = {kWidth, kHeight, 1};
    LockStep<Step> lockStep(Step::Begin);

    wgpu::Buffer buffer;
    std::thread writeThread([&] {
        const auto kBufferSize = kTextureBytesPerRowAlignment;
        buffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);

        std::vector<uint8_t> bufferData(kBufferSize);
        bufferData[0] = kExpectedData;

        queue.WriteBuffer(buffer.Get(), 0, bufferData.data(), kBufferSize);
        device.Tick();

        lockStep.Signal(Step::WriteBuffer);

        EXPECT_BUFFER_U8_EQ(kExpectedData, buffer, 0);
    });

    std::thread copyThread([&] {
        auto destTexture =
            CreateTexture(kWidth, kHeight, wgpu::TextureFormat::Stencil8,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc);

        auto encoder = device.CreateCommandEncoder();

        wgpu::ImageCopyTexture dest = utils::CreateImageCopyTexture(
            destTexture, /*dstMipLevel=*/0, {0, 0, 0}, wgpu::TextureAspect::All);

        // Wait until src buffer is written.
        lockStep.Wait(Step::WriteBuffer);
        CopyBufferToTextureHelper(buffer, kTextureBytesPerRowAlignment, dest, kSize, encoder);

        // Verify the copied data
        ExpectAttachmentStencilTestData(destTexture, wgpu::TextureFormat::Stencil8, kWidth, kHeight,
                                        0, /*mipLevel=*/0, kExpectedData);
    });

    writeThread.join();
    copyThread.join();
}

// Test that color texture's CopyTextureForBrowser() can work in parallel with other commands (such
// resources creation and texture to buffer copy for texture expectations).
// This test is needed since CopyTextureForBrowser() command might internally allocate resources and
// we need to make sure that it won't race with other threads' works.
TEST_P(MultithreadTextureCopyTests, CopyTextureForBrowserNoRace) {
    // TODO(crbug.com/dawn/1232): Program link error on OpenGLES backend
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES());
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    enum class Step {
        Begin,
        WriteTexture,
    };

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    const std::vector<utils::RGBA8> kExpectedData = {
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kGreen, utils::RGBA8::kBlack,  //
        utils::RGBA8::kRed,   utils::RGBA8::kRed,   utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kRed,   utils::RGBA8::kBlue,  utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
    };

    const std::vector<utils::RGBA8> kExpectedFlippedData = {
        utils::RGBA8::kRed,   utils::RGBA8::kBlue,  utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kRed,   utils::RGBA8::kRed,   utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kGreen, utils::RGBA8::kBlack,  //
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
    };

    const size_t kExpectedDataSize = kExpectedData.size() * sizeof(kExpectedData[0]);

    LockStep<Step> lockStep(Step::Begin);

    wgpu::Texture srcTexture;
    std::thread writeThread([&] {
        srcTexture =
            CreateAndWriteTexture(kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                                  wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding,
                                  kExpectedData.data(), kExpectedDataSize);

        lockStep.Signal(Step::WriteTexture);

        // Verify the initial data
        EXPECT_TEXTURE_EQ(kExpectedData.data(), srcTexture, {0, 0}, {kWidth, kHeight});
    });

    std::thread copyThread([&] {
        auto destTexture =
            CreateTexture(kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc);

        // Copy from srcTexture to destTexture.
        const wgpu::Extent3D dstSize = {kWidth, kHeight, 1};
        wgpu::ImageCopyTexture dest = utils::CreateImageCopyTexture(
            destTexture, /*dstMipLevel=*/0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::CopyTextureForBrowserOptions options;
        options.flipY = true;

        lockStep.Wait(Step::WriteTexture);
        CopyTextureToTextureHelper(srcTexture, dest, dstSize, nullptr, &options);

        // Verify the copied data
        EXPECT_TEXTURE_EQ(kExpectedFlippedData.data(), destTexture, {0, 0}, {kWidth, kHeight});
    });

    writeThread.join();
    copyThread.join();
}

// Test that error from CopyTextureForBrowser() won't cause deadlock.
TEST_P(MultithreadTextureCopyTests, CopyTextureForBrowserErrorNoDeadLock) {
    // TODO(crbug.com/dawn/1232): Program link error on OpenGLES backend
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES());
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    enum class Step {
        Begin,
        WriteTexture,
    };

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    const std::vector<utils::RGBA8> kExpectedData = {
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kGreen, utils::RGBA8::kBlack,  //
        utils::RGBA8::kRed,   utils::RGBA8::kRed,   utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
        utils::RGBA8::kRed,   utils::RGBA8::kBlue,  utils::RGBA8::kBlack, utils::RGBA8::kBlack,  //
    };

    const size_t kExpectedDataSize = kExpectedData.size() * sizeof(kExpectedData[0]);

    LockStep<Step> lockStep(Step::Begin);

    wgpu::Texture srcTexture;
    std::thread writeThread([&] {
        srcTexture =
            CreateAndWriteTexture(kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                                  wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding,
                                  kExpectedData.data(), kExpectedDataSize);

        lockStep.Signal(Step::WriteTexture);

        // Verify the initial data
        EXPECT_TEXTURE_EQ(kExpectedData.data(), srcTexture, {0, 0}, {kWidth, kHeight});
    });

    std::thread copyThread([&] {
        wgpu::Texture invalidSrcTexture;
        invalidSrcTexture = CreateTexture(kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                                          wgpu::TextureUsage::CopySrc);
        auto destTexture =
            CreateTexture(kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst |
                              wgpu::TextureUsage::CopySrc);

        // Copy from srcTexture to destTexture.
        const wgpu::Extent3D dstSize = {kWidth, kHeight, 1};
        wgpu::ImageCopyTexture dest = utils::CreateImageCopyTexture(
            destTexture, /*dstMipLevel=*/0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::CopyTextureForBrowserOptions options = {};

        device.PushErrorScope(wgpu::ErrorFilter::Validation);

        // The first copy should be an error because of missing TextureBinding from src texture.
        lockStep.Wait(Step::WriteTexture);
        CopyTextureToTextureHelper(invalidSrcTexture, dest, dstSize, nullptr, &options);

        std::atomic<bool> errorThrown(false);
        device.PopErrorScope(
            [](WGPUErrorType type, char const* message, void* userdata) {
                EXPECT_EQ(type, WGPUErrorType_Validation);
                auto error = static_cast<std::atomic<bool>*>(userdata);
                *error = true;
            },
            &errorThrown);
        device.Tick();
        EXPECT_TRUE(errorThrown.load());

        // Second copy is valid.
        CopyTextureToTextureHelper(srcTexture, dest, dstSize, nullptr, &options);

        // Verify the copied data
        EXPECT_TEXTURE_EQ(kExpectedData.data(), destTexture, {0, 0}, {kWidth, kHeight});
    });

    writeThread.join();
    copyThread.join();
}

class MultithreadDrawIndexedIndirectTests : public MultithreadTests {
  protected:
    void SetUp() override {
        MultithreadTests::SetUp();

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@location(0) pos : vec4f) -> @builtin(position) vec4f {
                return pos;
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
        descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Uint32;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        descriptor.cTargets[0].format = utils::BasicRenderPass::kDefaultColorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        vertexBuffer = utils::CreateBufferFromData<float>(
            device, wgpu::BufferUsage::Vertex,
            {// First quad: the first 3 vertices represent the bottom left triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
             0.0f, 1.0f,

             // Second quad: the first 3 vertices represent the top right triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f,
             0.0f, 1.0f});
    }

    void Test(std::initializer_list<uint32_t> bufferList,
              uint64_t indexOffset,
              uint64_t indirectOffset,
              utils::RGBA8 bottomLeftExpected,
              utils::RGBA8 topRightExpected) {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
            device, kRTSize, kRTSize, utils::BasicRenderPass::kDefaultColorFormat);
        wgpu::Buffer indexBuffer =
            CreateIndexBuffer({0, 1, 2, 0, 3, 1,
                               // The indices below are added to test negatve baseVertex
                               0 + 4, 1 + 4, 2 + 4, 0 + 4, 3 + 4, 1 + 4});
        TestDraw(
            renderPass, bottomLeftExpected, topRightExpected,
            EncodeDrawCommands(bufferList, indexBuffer, indexOffset, indirectOffset, renderPass));
    }

  private:
    wgpu::Buffer CreateIndirectBuffer(std::initializer_list<uint32_t> indirectParamList) {
        return utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage, indirectParamList);
    }

    wgpu::Buffer CreateIndexBuffer(std::initializer_list<uint32_t> indexList) {
        return utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, indexList);
    }

    wgpu::CommandBuffer EncodeDrawCommands(std::initializer_list<uint32_t> bufferList,
                                           wgpu::Buffer indexBuffer,
                                           uint64_t indexOffset,
                                           uint64_t indirectOffset,
                                           const utils::BasicRenderPass& renderPass) {
        wgpu::Buffer indirectBuffer = CreateIndirectBuffer(bufferList);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, indexOffset);
            pass.DrawIndexedIndirect(indirectBuffer, indirectOffset);
            pass.End();
        }

        return encoder.Finish();
    }

    void TestDraw(const utils::BasicRenderPass& renderPass,
                  utils::RGBA8 bottomLeftExpected,
                  utils::RGBA8 topRightExpected,
                  wgpu::CommandBuffer commands) {
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }

    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    static constexpr uint32_t kRTSize = 4;
};

// Test indirect draws with offsets on multiple threads.
TEST_P(MultithreadDrawIndexedIndirectTests, IndirectOffsetInParallel) {
    // TODO(crbug.com/dawn/789): Test is failing after a roll on SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    utils::RunInParallel(10, [=](uint32_t) {
        // Test an offset draw call, with indirect buffer containing 2 calls:
        // 1) first 3 indices of the second quad (top right triangle)
        // 2) last 3 indices of the second quad

        // Test #1 (no offset)
        Test({3, 1, 0, 4, 0, 3, 1, 3, 4, 0}, 0, 0, notFilled, filled);

        // Offset to draw #2
        Test({3, 1, 0, 4, 0, 3, 1, 3, 4, 0}, 0, 5 * sizeof(uint32_t), filled, notFilled);
    });
}

class TimestampExpectation : public detail::Expectation {
  public:
    ~TimestampExpectation() override = default;

    // Expect the timestamp results are greater than 0.
    testing::AssertionResult Check(const void* data, size_t size) override {
        ASSERT(size % sizeof(uint64_t) == 0);
        const uint64_t* timestamps = static_cast<const uint64_t*>(data);
        for (size_t i = 0; i < size / sizeof(uint64_t); i++) {
            if (timestamps[i] == 0) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be greater than 0." << std::endl;
            }
        }

        return testing::AssertionSuccess();
    }
};

class MultithreadTimestampQueryTests : public MultithreadTests {
  protected:
    void SetUp() override {
        MultithreadTests::SetUp();

        // Skip all tests if timestamp feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::TimestampQuery}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = MultithreadTests::GetRequiredFeatures();
        if (SupportsFeatures({wgpu::FeatureName::TimestampQuery})) {
            requiredFeatures.push_back(wgpu::FeatureName::TimestampQuery);
        }
        return requiredFeatures;
    }

    wgpu::QuerySet CreateQuerySetForTimestamp(uint32_t queryCount) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.count = queryCount;
        descriptor.type = wgpu::QueryType::Timestamp;
        return device.CreateQuerySet(&descriptor);
    }

    wgpu::Buffer CreateResolveBuffer(uint64_t size) {
        return CreateBuffer(size, /*usage=*/wgpu::BufferUsage::QueryResolve |
                                      wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
    }
};

// Test resolving timestamp queries on multiple threads. ResolveQuerySet() will create temp
// resources internally so we need to make sure they are thread safe.
TEST_P(MultithreadTimestampQueryTests, ResolveQuerySets_InParallel) {
    constexpr uint32_t kQueryCount = 2;
    constexpr uint32_t kNumThreads = 10;

    std::vector<wgpu::QuerySet> querySets(kNumThreads);
    std::vector<wgpu::Buffer> destinations(kNumThreads);

    for (size_t i = 0; i < kNumThreads; ++i) {
        querySets[i] = CreateQuerySetForTimestamp(kQueryCount);
        destinations[i] = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    }

    utils::RunInParallel(kNumThreads, [&](uint32_t index) {
        const auto& querySet = querySets[index];
        const auto& destination = destinations[index];
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    });
}

DAWN_INSTANTIATE_TEST(MultithreadTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadCachingTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadEncodingTests,
                      D3D12Backend(),
                      D3D12Backend({"always_resolve_into_zero_level_and_layer"}),
                      MetalBackend(),
                      MetalBackend({"always_resolve_into_zero_level_and_layer"}),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      VulkanBackend({"always_resolve_into_zero_level_and_layer"}));

DAWN_INSTANTIATE_TEST(
    MultithreadTextureCopyTests,
    D3D12Backend(),
    MetalBackend(),
    MetalBackend({"use_blit_for_buffer_to_depth_texture_copy",
                  "use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource"}),
    MetalBackend({"use_blit_for_buffer_to_stencil_texture_copy"}),
    OpenGLBackend(),
    OpenGLESBackend(),
    VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadDrawIndexedIndirectTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadTimestampQueryTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
