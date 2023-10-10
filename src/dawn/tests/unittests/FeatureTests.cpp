// Copyright 2019 The Dawn Authors
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

#include "dawn/native/Features.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/null/DeviceNull.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

class FeatureTests : public testing::Test {
  public:
    FeatureTests()
        : testing::Test(),
          mInstanceBase(native::APICreateInstance(nullptr)),
          mPhysicalDevice(mInstanceBase.Get()),
          mUnsafePhysicalDevice(mInstanceBase.Get()),
          mAdapterBase(&mPhysicalDevice,
                       native::FeatureLevel::Core,
                       native::TogglesState(native::ToggleStage::Adapter),
                       wgpu::PowerPreference::Undefined),
          mUnsafeAdapterBase(&mUnsafePhysicalDevice,
                             native::FeatureLevel::Core,
                             native::TogglesState(native::ToggleStage::Adapter)
                                 .SetForTesting(native::Toggle::AllowUnsafeAPIs, true, true),
                             wgpu::PowerPreference::Undefined) {}

    std::vector<wgpu::FeatureName> GetAllFeatureNames() {
        std::vector<wgpu::FeatureName> allFeatureNames(kTotalFeaturesCount);
        for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
            allFeatureNames[i] = native::ToAPI(static_cast<native::Feature>(i));
        }
        return allFeatureNames;
    }

    static constexpr size_t kTotalFeaturesCount =
        static_cast<size_t>(native::kEnumCount<native::Feature>);

  protected:
    // By default DisallowUnsafeAPIs is enabled in this instance.
    Ref<dawn::native::InstanceBase> mInstanceBase;
    native::null::PhysicalDevice mPhysicalDevice;
    native::null::PhysicalDevice mUnsafePhysicalDevice;
    // The adapter that inherit toggles states from the instance, also have DisallowUnsafeAPIs
    // enabled.
    native::AdapterBase mAdapterBase;
    native::AdapterBase mUnsafeAdapterBase;
};

// Test the creation of a device will fail if the requested feature is not supported on the
// Adapter.
TEST_F(FeatureTests, AdapterWithRequiredFeatureDisabled) {
    const std::vector<wgpu::FeatureName> kAllFeatureNames = GetAllFeatureNames();
    for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
        native::Feature notSupportedFeature = static_cast<native::Feature>(i);

        std::vector<wgpu::FeatureName> featureNamesWithoutOne = kAllFeatureNames;
        featureNamesWithoutOne.erase(featureNamesWithoutOne.begin() + i);

        // Test that the default adapter validates features as expected.
        {
            mPhysicalDevice.SetSupportedFeaturesForTesting(featureNamesWithoutOne);
            native::Adapter adapterWithoutFeature(&mAdapterBase);

            wgpu::DeviceDescriptor deviceDescriptor;
            wgpu::FeatureName featureName = native::ToAPI(notSupportedFeature);
            deviceDescriptor.requiredFeatures = &featureName;
            deviceDescriptor.requiredFeatureCount = 1;

            WGPUDevice deviceWithFeature = adapterWithoutFeature.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor));
            ASSERT_EQ(nullptr, deviceWithFeature);
        }

        // Test that an adapter with AllowUnsafeApis enabled validates features as expected.
        {
            mUnsafePhysicalDevice.SetSupportedFeaturesForTesting(featureNamesWithoutOne);
            native::Adapter adapterWithoutFeature(&mUnsafeAdapterBase);

            wgpu::DeviceDescriptor deviceDescriptor;
            wgpu::FeatureName featureName = ToAPI(notSupportedFeature);
            deviceDescriptor.requiredFeatures = &featureName;
            deviceDescriptor.requiredFeatureCount = 1;

            WGPUDevice deviceWithFeature = adapterWithoutFeature.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor));
            ASSERT_EQ(nullptr, deviceWithFeature);
        }
    }
}

// Test creating device requiring a supported feature can succeed (with DisallowUnsafeApis adapter
// toggle disabled for experimental features), and Device.GetEnabledFeatures() can return the names
// of the enabled features correctly.
TEST_F(FeatureTests, RequireAndGetEnabledFeatures) {
    native::Adapter adapter(&mAdapterBase);
    native::Adapter unsafeAdapterAllow(&mUnsafeAdapterBase);

    for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
        native::Feature feature = static_cast<native::Feature>(i);
        wgpu::FeatureName featureName = ToAPI(feature);

        wgpu::DeviceDescriptor deviceDescriptor;
        deviceDescriptor.requiredFeatures = &featureName;
        deviceDescriptor.requiredFeatureCount = 1;

        // Test with the default adapter.
        {
            native::DeviceBase* deviceBase = native::FromAPI(adapter.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor)));

            // Creating a device with experimental feature requires the adapter enables
            // AllowUnsafeAPIs or disables DisallowUnsafeApis, otherwise expect validation error.
            if (native::kFeatureNameAndInfoList[feature].featureState ==
                native::FeatureInfo::FeatureState::Experimental) {
                ASSERT_EQ(nullptr, deviceBase) << i;
            } else {
                // Requiring stable features should succeed.
                ASSERT_NE(nullptr, deviceBase);
                ASSERT_EQ(1u, deviceBase->APIEnumerateFeatures(nullptr));
                wgpu::FeatureName enabledFeature;
                deviceBase->APIEnumerateFeatures(&enabledFeature);
                EXPECT_EQ(enabledFeature, featureName);
                deviceBase->APIRelease();
            }
        }

        // Test with the adapter with AllowUnsafeApis toggles enabled, creating device should always
        // succeed.
        {
            native::DeviceBase* deviceBase = native::FromAPI(unsafeAdapterAllow.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor)));

            ASSERT_NE(nullptr, deviceBase);
            ASSERT_EQ(1u, deviceBase->APIEnumerateFeatures(nullptr));
            wgpu::FeatureName enabledFeature;
            deviceBase->APIEnumerateFeatures(&enabledFeature);
            EXPECT_EQ(enabledFeature, featureName);
            deviceBase->APIRelease();
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
