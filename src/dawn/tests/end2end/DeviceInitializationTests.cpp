// Copyright 2021 The Dawn Authors
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

#include <memory>
#include <utility>
#include <vector>

#include "dawn/dawn_proc.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class DeviceInitializationTest : public testing::Test {
  protected:
    void SetUp() override { dawnProcSetProcs(&native::GetProcs()); }

    void TearDown() override { dawnProcSetProcs(nullptr); }

    // Test that the device can still be used by testing a buffer copy.
    void ExpectDeviceUsable(wgpu::Device device) {
        wgpu::Buffer src =
            utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::CopySrc, {1, 2, 3, 4});

        wgpu::Buffer dst = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead, {0, 0, 0, 0});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(src, 0, dst, 0, 4 * sizeof(uint32_t));

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        bool done = false;
        dst.MapAsync(
            wgpu::MapMode::Read, 0, 4 * sizeof(uint32_t),
            [](WGPUBufferMapAsyncStatus status, void* userdata) {
                EXPECT_EQ(status, WGPUBufferMapAsyncStatus_Success);
                *static_cast<bool*>(userdata) = true;
            },
            &done);

        // Note: we can't actually test this if Tick moves over to
        // wgpuInstanceProcessEvents. We can still test that object creation works
        // without crashing.
        while (!done) {
            device.Tick();
            utils::USleep(100);
        }

        const uint32_t* mapping = static_cast<const uint32_t*>(dst.GetConstMappedRange());
        EXPECT_EQ(mapping[0], 1u);
        EXPECT_EQ(mapping[1], 2u);
        EXPECT_EQ(mapping[2], 3u);
        EXPECT_EQ(mapping[3], 4u);
    }
};

// Test that device operations are still valid if the reference to the instance
// is dropped.
TEST_F(DeviceInitializationTest, DeviceOutlivesInstance) {
    // Get properties of all available adapters and then free the instance.
    // We want to create a device on a fresh instance and adapter each time.
    std::vector<wgpu::AdapterProperties> availableAdapterProperties;
    {
        auto instance = std::make_unique<native::Instance>();
        instance->EnableAdapterBlocklist(false);
        instance->DiscoverDefaultAdapters();
        for (const native::Adapter& adapter : instance->GetAdapters()) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            if (properties.backendType == wgpu::BackendType::Null) {
                continue;
            }

            availableAdapterProperties.push_back(properties);
        }
    }

    for (const wgpu::AdapterProperties& desiredProperties : availableAdapterProperties) {
        wgpu::Device device;

        auto instance = std::make_unique<native::Instance>();
        instance->EnableAdapterBlocklist(false);
        instance->DiscoverDefaultAdapters();
        for (native::Adapter& adapter : instance->GetAdapters()) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            if (properties.deviceID == desiredProperties.deviceID &&
                properties.vendorID == desiredProperties.vendorID &&
                properties.adapterType == desiredProperties.adapterType &&
                properties.backendType == desiredProperties.backendType) {
                // Create the device, destroy the instance, and break out of the loop.
                device = wgpu::Device::Acquire(adapter.CreateDevice());
                instance.reset();
                break;
            }
        }

        if (device) {
            ExpectDeviceUsable(std::move(device));
        }
    }
}

// Test that it is still possible to create a device from an adapter after the reference to the
// instance is dropped.
TEST_F(DeviceInitializationTest, AdapterOutlivesInstance) {
    // Get properties of all available adapters and then free the instance.
    // We want to create a device on a fresh instance and adapter each time.
    std::vector<wgpu::AdapterProperties> availableAdapterProperties;
    {
        auto instance = std::make_unique<native::Instance>();
        instance->EnableAdapterBlocklist(false);
        instance->DiscoverDefaultAdapters();
        for (const native::Adapter& adapter : instance->GetAdapters()) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            if (properties.backendType == wgpu::BackendType::Null) {
                continue;
            }
            // TODO(dawn:1705): Remove this once D3D11 backend is fully implemented.
            if (properties.backendType == wgpu::BackendType::D3D11) {
                continue;
            }
            availableAdapterProperties.push_back(properties);
        }
    }

    for (const wgpu::AdapterProperties& desiredProperties : availableAdapterProperties) {
        wgpu::Adapter adapter;

        auto instance = std::make_unique<native::Instance>();
        instance->EnableAdapterBlocklist(false);
        // Save a pointer to the instance.
        // It will only be valid as long as the instance is alive.
        WGPUInstance unsafeInstancePtr = instance->Get();

        instance->DiscoverDefaultAdapters();
        for (native::Adapter& nativeAdapter : instance->GetAdapters()) {
            wgpu::AdapterProperties properties;
            nativeAdapter.GetProperties(&properties);

            if (properties.deviceID == desiredProperties.deviceID &&
                properties.vendorID == desiredProperties.vendorID &&
                properties.adapterType == desiredProperties.adapterType &&
                properties.backendType == desiredProperties.backendType) {
                // Save the adapter, and reset the instance.
                // Check that the number of adapters before the reset is > 0, and after the reset
                // is 0. Unsafe, but we assume the pointer is still valid since the adapter is
                // holding onto the instance. The instance should have cleared all internal
                // references to adapters when the last external ref is dropped.
                adapter = wgpu::Adapter(nativeAdapter.Get());
                EXPECT_GT(native::GetAdapterCountForTesting(unsafeInstancePtr), 0u);
                instance.reset();
                EXPECT_EQ(native::GetAdapterCountForTesting(unsafeInstancePtr), 0u);
                break;
            }
        }

        if (adapter) {
            ExpectDeviceUsable(adapter.CreateDevice());
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
