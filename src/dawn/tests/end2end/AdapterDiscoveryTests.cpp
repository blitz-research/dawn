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

#include "dawn/common/GPUInfo.h"
#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/webgpu_cpp.h"

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
#include "dawn/native/VulkanBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

#if defined(DAWN_ENABLE_BACKEND_D3D11)
#include "dawn/native/D3D11Backend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_D3D11)

#if defined(DAWN_ENABLE_BACKEND_D3D12)
#include "dawn/native/D3D12Backend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)

#if defined(DAWN_ENABLE_BACKEND_METAL)
#include "dawn/native/MetalBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_ENABLE_BACKEND_DESKTOP_GL) || defined(DAWN_ENABLE_BACKEND_OPENGLES)
#include "GLFW/glfw3.h"
#include "dawn/native/OpenGLBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_DESKTOP_GL) || defined(DAWN_ENABLE_BACKEND_OPENGLES)

#include <gtest/gtest.h>

namespace dawn {
namespace {

using testing::_;
using testing::MockCallback;
using testing::SaveArg;

class AdapterDiscoveryTests : public ::testing::Test {};

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
// Test only discovering the SwiftShader adapter
TEST(AdapterDiscoveryTests, OnlySwiftShader) {
    native::Instance instance;

    native::vulkan::AdapterDiscoveryOptions options;
    options.forceSwiftShader = true;
    instance.DiscoverAdapters(&options);

    const auto& adapters = instance.GetAdapters();
    EXPECT_LE(adapters.size(), 2u);  // 0 or 2 SwiftShader adapters.
    for (const auto& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.backendType, wgpu::BackendType::Vulkan);
        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::CPU);
        EXPECT_TRUE(gpu_info::IsGoogleSwiftshader(properties.vendorID, properties.deviceID));
    }
}

// Test discovering only Vulkan adapters
TEST(AdapterDiscoveryTests, OnlyVulkan) {
    native::Instance instance;

    native::vulkan::AdapterDiscoveryOptions options;
    instance.DiscoverAdapters(&options);

    const auto& adapters = instance.GetAdapters();
    for (const auto& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.backendType, wgpu::BackendType::Vulkan);
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

#if defined(DAWN_ENABLE_BACKEND_D3D11)
// Test discovering only D3D11 adapters
TEST(AdapterDiscoveryTests, OnlyD3D11) {
    native::Instance instance;

    native::d3d11::AdapterDiscoveryOptions options;
    instance.DiscoverAdapters(&options);

    const auto& adapters = instance.GetAdapters();
    for (const auto& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.backendType, wgpu::BackendType::D3D11);
    }
}

// Test discovering a D3D11 adapter from a prexisting DXGI adapter
TEST(AdapterDiscoveryTests, MatchingDXGIAdapterD3D11) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIFactory4> dxgiFactory;
    HRESULT hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
    ASSERT_EQ(hr, S_OK);

    for (uint32_t adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
        if (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) == DXGI_ERROR_NOT_FOUND) {
            break;  // No more adapters to enumerate.
        }

        native::Instance instance;

        native::d3d11::AdapterDiscoveryOptions options;
        options.dxgiAdapter = std::move(dxgiAdapter);
        instance.DiscoverAdapters(&options);

        const auto& adapters = instance.GetAdapters();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            EXPECT_EQ(properties.backendType, wgpu::BackendType::D3D11);
        }
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_D3D11)

#if defined(DAWN_ENABLE_BACKEND_D3D12)
// Test discovering only D3D12 adapters
TEST(AdapterDiscoveryTests, OnlyD3D12) {
    native::Instance instance;

    native::d3d12::AdapterDiscoveryOptions options;
    instance.DiscoverAdapters(&options);

    const auto& adapters = instance.GetAdapters();
    for (const auto& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.backendType, wgpu::BackendType::D3D12);
    }
}

// Test discovering a D3D12 adapter from a prexisting DXGI adapter
TEST(AdapterDiscoveryTests, MatchingDXGIAdapterD3D12) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIFactory4> dxgiFactory;
    HRESULT hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
    ASSERT_EQ(hr, S_OK);

    for (uint32_t adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
        if (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) == DXGI_ERROR_NOT_FOUND) {
            break;  // No more adapters to enumerate.
        }

        native::Instance instance;

        native::d3d12::AdapterDiscoveryOptions options;
        options.dxgiAdapter = std::move(dxgiAdapter);
        instance.DiscoverAdapters(&options);

        const auto& adapters = instance.GetAdapters();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            EXPECT_EQ(properties.backendType, wgpu::BackendType::D3D12);
        }
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)

#if defined(DAWN_ENABLE_BACKEND_METAL)
// Test discovering only Metal adapters
TEST(AdapterDiscoveryTests, OnlyMetal) {
    native::Instance instance;

    native::metal::AdapterDiscoveryOptions options;
    instance.DiscoverAdapters(&options);

    const auto& adapters = instance.GetAdapters();
    for (const auto& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.backendType, wgpu::BackendType::Metal);
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_ENABLE_BACKEND_METAL) && defined(DAWN_ENABLE_BACKEND_VULKAN)
// Test discovering the Metal backend, then the Vulkan backend
// does not duplicate adapters.
TEST(AdapterDiscoveryTests, OneBackendThenTheOther) {
    native::Instance instance;
    uint32_t metalAdapterCount = 0;
    {
        native::metal::AdapterDiscoveryOptions options;
        instance.DiscoverAdapters(&options);

        const auto& adapters = instance.GetAdapters();
        metalAdapterCount = adapters.size();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            ASSERT_EQ(properties.backendType, wgpu::BackendType::Metal);
        }
    }
    {
        native::vulkan::AdapterDiscoveryOptions options;
        instance.DiscoverAdapters(&options);

        uint32_t metalAdapterCount2 = 0;
        const auto& adapters = instance.GetAdapters();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            EXPECT_TRUE(properties.backendType == wgpu::BackendType::Metal ||
                        properties.backendType == wgpu::BackendType::Vulkan);
            if (properties.backendType == wgpu::BackendType::Metal) {
                metalAdapterCount2++;
            }
        }
        EXPECT_EQ(metalAdapterCount, metalAdapterCount2);
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN) && defined(DAWN_ENABLE_BACKEND_METAL)

class AdapterCreationTest : public ::testing::Test {
  protected:
    void SetUp() override {
        dawnProcSetProcs(&dawn_native::GetProcs());

        {
            auto nativeInstance = std::make_unique<dawn_native::Instance>();
            nativeInstance->DiscoverDefaultAdapters();
            for (dawn_native::Adapter& nativeAdapter : nativeInstance->GetAdapters()) {
                anyAdapterAvailable = true;

                wgpu::AdapterProperties properties;
                nativeAdapter.GetProperties(&properties);
                if (properties.compatibilityMode) {
                    continue;
                }
                swiftShaderAvailable |=
                    gpu_info::IsGoogleSwiftshader(properties.vendorID, properties.deviceID);
                discreteGPUAvailable |= properties.adapterType == wgpu::AdapterType::DiscreteGPU;
                integratedGPUAvailable |=
                    properties.adapterType == wgpu::AdapterType::IntegratedGPU;
            }
        }

        instance = wgpu::CreateInstance();
    }

    void TearDown() override {
        instance = nullptr;
        dawnProcSetProcs(nullptr);
    }

    wgpu::Instance instance;
    bool anyAdapterAvailable = false;
    bool swiftShaderAvailable = false;
    bool discreteGPUAvailable = false;
    bool integratedGPUAvailable = false;
};

// Test that requesting the default adapter works
TEST_F(AdapterCreationTest, DefaultAdapter) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
}

// Test that passing nullptr for the options gets the default adapter
TEST_F(AdapterCreationTest, NullGivesDefaultAdapter) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this + 1))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(nullptr, cb.Callback(), cb.MakeUserdata(this + 1));

    wgpu::Adapter adapter2 = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter2 != nullptr, anyAdapterAvailable);
}

// Test that requesting the fallback adapter returns SwiftShader.
TEST_F(AdapterCreationTest, FallbackAdapter) {
    wgpu::RequestAdapterOptions options = {};
    options.forceFallbackAdapter = true;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, swiftShaderAvailable);
    if (adapter != nullptr) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::CPU);
        EXPECT_TRUE(gpu_info::IsGoogleSwiftshader(properties.vendorID, properties.deviceID));
    }
}

// Test that requesting a high performance GPU works
TEST_F(AdapterCreationTest, PreferHighPerformance) {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::HighPerformance;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (discreteGPUAvailable) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);
        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::DiscreteGPU);
    }
}

// Test that requesting a low power GPU works
TEST_F(AdapterCreationTest, PreferLowPower) {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::LowPower;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (integratedGPUAvailable) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);
        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::IntegratedGPU);
    }
}

// Test that requesting a Compatibility adapter is supported.
TEST_F(AdapterCreationTest, Compatibility) {
    wgpu::RequestAdapterOptions options = {};
    options.compatibilityMode = true;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    wgpu::AdapterProperties properties;
    adapter.GetProperties(&properties);
    EXPECT_TRUE(properties.compatibilityMode);
}

// Test that requesting a Non-Compatibility adapter is supported and is default.
TEST_F(AdapterCreationTest, NonCompatibility) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    wgpu::AdapterProperties properties;
    adapter.GetProperties(&properties);
    EXPECT_FALSE(properties.compatibilityMode);
}

// Test that GetInstance() returns the correct Instance.
TEST_F(AdapterCreationTest, GetInstance) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    EXPECT_EQ(adapter.GetInstance().Get(), instance.Get());
}

}  // anonymous namespace
}  // namespace dawn
