// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_BUFFERVK_H_
#define SRC_DAWN_NATIVE_VULKAN_BUFFERVK_H_

#include <set>

#include "dawn/native/Buffer.h"

#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/ResourceMemoryAllocation.h"

namespace dawn::native::vulkan {

struct CommandRecordingContext;
class Device;
struct VulkanFunctions;

class Buffer final : public BufferBase {
  public:
    static ResultOrError<Ref<Buffer>> Create(Device* device, const BufferDescriptor* descriptor);

    VkBuffer GetHandle() const;

    // Transitions the buffer to be used as `usage`, recording any necessary barrier in
    // `commands`.
    // TODO(crbug.com/dawn/851): coalesce barriers and do them early when possible.
    void TransitionUsageNow(CommandRecordingContext* recordingContext, wgpu::BufferUsage usage);
    bool TrackUsageAndGetResourceBarrier(CommandRecordingContext* recordingContext,
                                         wgpu::BufferUsage usage,
                                         VkBufferMemoryBarrier* barrier,
                                         VkPipelineStageFlags* srcStages,
                                         VkPipelineStageFlags* dstStages);

    // All the Ensure methods return true if the buffer was initialized to zero.
    bool EnsureDataInitialized(CommandRecordingContext* recordingContext);
    bool EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                            uint64_t offset,
                                            uint64_t size);
    bool EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                            const CopyTextureToBufferCmd* copy);

    // Dawn API
    void SetLabelImpl() override;

    static void TransitionMappableBuffersEagerly(const VulkanFunctions& fn,
                                                 CommandRecordingContext* recordingContext,
                                                 const std::set<Ref<Buffer>>& buffers);

  private:
    ~Buffer() override;
    using BufferBase::BufferBase;

    MaybeError Initialize(bool mappedAtCreation);
    MaybeError InitializeHostMapped(const BufferHostMappedPointer* hostMappedDesc);
    void InitializeToZero(CommandRecordingContext* recordingContext);
    void ClearBuffer(CommandRecordingContext* recordingContext,
                     uint32_t clearValue,
                     uint64_t offset = 0,
                     uint64_t size = 0);

    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    void UnmapImpl() override;
    void DestroyImpl() override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;
    void* GetMappedPointer() override;

    VkBuffer mHandle = VK_NULL_HANDLE;
    ResourceMemoryAllocation mMemoryAllocation;

    // VkDeviceMemory that is used strictly for this buffer.
    VkDeviceMemory mDedicatedDeviceMemory = VK_NULL_HANDLE;

    wgpu::Callback mHostMappedDisposeCallback = nullptr;
    void* mHostMappedDisposeUserdata = nullptr;

    wgpu::BufferUsage mLastUsage = wgpu::BufferUsage::None;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_BUFFERVK_H_
