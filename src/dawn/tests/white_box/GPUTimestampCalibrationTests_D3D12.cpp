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

#include <memory>

#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/tests/white_box/GPUTimestampCalibrationTests.h"

namespace dawn {
namespace {

class GPUTimestampCalibrationTestsD3D12 : public GPUTimestampCalibrationTestBackend {
  public:
    explicit GPUTimestampCalibrationTestsD3D12(const wgpu::Device& device) {
        mBackendDevice = native::d3d12::ToBackend(native::FromAPI(device.Get()));
    }

    bool IsSupported() const override { return true; }

    void GetTimestampCalibration(uint64_t* gpuTimestamp, uint64_t* cpuTimestamp) override {
        mBackendDevice->GetCommandQueue()->GetClockCalibration(gpuTimestamp, cpuTimestamp);
    }

    float GetTimestampPeriod() const override { return mBackendDevice->GetTimestampPeriodInNS(); }

  private:
    native::d3d12::Device* mBackendDevice;
};

}  // anonymous namespace

// static
std::unique_ptr<GPUTimestampCalibrationTestBackend> GPUTimestampCalibrationTestBackend::Create(
    const wgpu::Device& device) {
    return std::make_unique<GPUTimestampCalibrationTestsD3D12>(device);
}

}  // namespace dawn
