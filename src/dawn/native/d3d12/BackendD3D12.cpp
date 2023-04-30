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

#include "dawn/native/d3d12/BackendD3D12.h"

#include <memory>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/AdapterD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"

// HELLO!

namespace dawn::native::d3d12 {
    
Backend::Backend(InstanceBase* instance) : Base(instance, wgpu::BackendType::D3D12) {}

MaybeError Backend::Initialize() {
    auto functions = std::make_unique<PlatformFunctions>();
    DAWN_TRY(functions->LoadFunctions());

    // Enable the debug layer (requires the Graphics Tools "optional feature").
    const auto instance = GetInstance();
    if (instance->GetBackendValidationLevel() != BackendValidationLevel::Disabled) {
        ComPtr<ID3D12Debug3> debugController;
        if (SUCCEEDED(functions->d3d12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            ASSERT(debugController != nullptr);
            debugController->EnableDebugLayer();
            if (instance->GetBackendValidationLevel() == BackendValidationLevel::Full) {
                debugController->SetEnableGPUBasedValidation(true);
            }
        }
    }

    if (instance->IsBeginCaptureOnStartupEnabled()) {
        ComPtr<IDXGraphicsAnalysis> graphicsAnalysis;
        if (functions->dxgiGetDebugInterface1 != nullptr &&
            SUCCEEDED(functions->dxgiGetDebugInterface1(0, IID_PPV_ARGS(&graphicsAnalysis)))) {
            graphicsAnalysis->BeginCapture();
        }
    }

    DAWN_TRY(Base::Initialize(std::move(functions)));

    return {};
}

const PlatformFunctions* Backend::GetFunctions() const {
    return static_cast<const PlatformFunctions*>(Base::GetFunctions());
}

ResultOrError<Ref<AdapterBase>> Backend::CreateAdapterFromIDXGIAdapter(
    ComPtr<IDXGIAdapter> dxgiAdapter,
    const TogglesState& adapterToggles) {
    ComPtr<IDXGIAdapter3> dxgiAdapter3;
    DAWN_TRY(CheckHRESULT(dxgiAdapter.As(&dxgiAdapter3), "DXGIAdapter retrieval"));
    Ref<Adapter> adapter = AcquireRef(new Adapter(this, std::move(dxgiAdapter3), adapterToggles));
    DAWN_TRY(adapter->Initialize());

    return {std::move(adapter)};
}

BackendConnection* Connect(InstanceBase* instance) {
    Backend* backend = new Backend(instance);

    if (instance->ConsumedError(backend->Initialize())) {
        delete backend;
        return nullptr;
    }

    return backend;
}

}  // namespace dawn::native::d3d12
