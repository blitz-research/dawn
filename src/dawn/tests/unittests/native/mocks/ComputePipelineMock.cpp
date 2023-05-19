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

#include "dawn/tests/unittests/native/mocks/ComputePipelineMock.h"

namespace dawn::native {

using ::testing::NiceMock;

ComputePipelineMock::ComputePipelineMock(DeviceBase* device,
                                         const ComputePipelineDescriptor* descriptor)
    : ComputePipelineBase(device, descriptor) {
    ON_CALL(*this, Initialize).WillByDefault([]() -> MaybeError { return {}; });
    ON_CALL(*this, DestroyImpl).WillByDefault([this]() {
        this->ComputePipelineBase::DestroyImpl();
    });
}

ComputePipelineMock::~ComputePipelineMock() = default;

// static
Ref<ComputePipelineMock> ComputePipelineMock::Create(DeviceMock* device,
                                                     const ComputePipelineDescriptor* descriptor) {
    ComputePipelineDescriptor appliedDescriptor;
    Ref<PipelineLayoutBase> layoutRef = ValidateLayoutAndGetComputePipelineDescriptorWithDefaults(
                                            device, *descriptor, &appliedDescriptor)
                                            .AcquireSuccess();
    return AcquireRef(new NiceMock<ComputePipelineMock>(device, &appliedDescriptor));
}

}  // namespace dawn::native
