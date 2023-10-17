// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_EXTERNAL_SEMAPHORE_SEMAPHORESERVICE_H_
#define SRC_DAWN_NATIVE_VULKAN_EXTERNAL_SEMAPHORE_SEMAPHORESERVICE_H_

#include <memory>

#include "dawn/native/Error.h"
#include "dawn/native/vulkan/ExternalHandle.h"

namespace dawn::native::vulkan {
class Device;
struct VulkanDeviceInfo;
struct VulkanFunctions;
}  // namespace dawn::native::vulkan

namespace dawn::native::vulkan::external_semaphore {
class ServiceImplementation;

class Service {
  public:
    explicit Service(Device* device);
    ~Service();

    static bool CheckSupport(const VulkanDeviceInfo& deviceInfo,
                             VkPhysicalDevice physicalDevice,
                             const VulkanFunctions& fn);
    void CloseHandle(ExternalSemaphoreHandle handle);

    // True if the device reports it supports this feature
    bool Supported();

    // Given an external handle, import it into a VkSemaphore
    ResultOrError<VkSemaphore> ImportSemaphore(ExternalSemaphoreHandle handle);

    // Create a VkSemaphore that is exportable into an external handle later
    ResultOrError<VkSemaphore> CreateExportableSemaphore();

    // Export a VkSemaphore into an external handle
    ResultOrError<ExternalSemaphoreHandle> ExportSemaphore(VkSemaphore semaphore);

    // Duplicate a new external handle from the given one.
    ExternalSemaphoreHandle DuplicateHandle(ExternalSemaphoreHandle handle);

  private:
    // Linux, ChromeOS and Android use FD semaphore and Fuchia uses ZirconHandle.
    std::unique_ptr<ServiceImplementation> mServiceImpl;
};

}  // namespace dawn::native::vulkan::external_semaphore

#endif  // SRC_DAWN_NATIVE_VULKAN_EXTERNAL_SEMAPHORE_SEMAPHORESERVICE_H_
