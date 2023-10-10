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

#include "dawn/native/AttachmentState.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

AttachmentState::AttachmentState(DeviceBase* device,
                                 const RenderBundleEncoderDescriptor* descriptor)
    : ObjectBase(device), mSampleCount(descriptor->sampleCount) {
    DAWN_ASSERT(descriptor->colorFormatCount <= kMaxColorAttachments);
    for (ColorAttachmentIndex i(uint8_t(0));
         i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->colorFormatCount)); ++i) {
        wgpu::TextureFormat format = descriptor->colorFormats[static_cast<uint8_t>(i)];
        if (format != wgpu::TextureFormat::Undefined) {
            mColorAttachmentsSet.set(i);
            mColorFormats[i] = format;
        }
    }
    mDepthStencilFormat = descriptor->depthStencilFormat;

    // TODO(dawn:1710): support MSAA render to single sampled in render bundles.
    // TODO(dawn:1704): support PLS in render bundles.

    SetContentHash(ComputeContentHash());
}

AttachmentState::AttachmentState(DeviceBase* device,
                                 const RenderPipelineDescriptor* descriptor,
                                 const PipelineLayoutBase* layout)
    : ObjectBase(device), mSampleCount(descriptor->multisample.count) {
    const DawnMultisampleStateRenderToSingleSampled* msaaRenderToSingleSampledDesc = nullptr;
    FindInChain(descriptor->multisample.nextInChain, &msaaRenderToSingleSampledDesc);
    if (msaaRenderToSingleSampledDesc != nullptr) {
        mIsMSAARenderToSingleSampledEnabled = msaaRenderToSingleSampledDesc->enabled;
    }

    if (descriptor->fragment != nullptr) {
        DAWN_ASSERT(descriptor->fragment->targetCount <= kMaxColorAttachments);
        for (ColorAttachmentIndex i(uint8_t(0));
             i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->fragment->targetCount));
             ++i) {
            wgpu::TextureFormat format =
                descriptor->fragment->targets[static_cast<uint8_t>(i)].format;
            if (format != wgpu::TextureFormat::Undefined) {
                mColorAttachmentsSet.set(i);
                mColorFormats[i] = format;
            }
        }
    }
    if (descriptor->depthStencil != nullptr) {
        mDepthStencilFormat = descriptor->depthStencil->format;
    }

    mHasPLS = layout->HasPixelLocalStorage();
    mStorageAttachmentSlots = layout->GetStorageAttachmentSlots();

    SetContentHash(ComputeContentHash());
}

AttachmentState::AttachmentState(DeviceBase* device, const RenderPassDescriptor* descriptor)
    : ObjectBase(device) {
    for (ColorAttachmentIndex i(uint8_t(0));
         i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->colorAttachmentCount)); ++i) {
        const RenderPassColorAttachment& colorAttachment =
            descriptor->colorAttachments[static_cast<uint8_t>(i)];
        TextureViewBase* attachment = colorAttachment.view;
        if (attachment == nullptr) {
            continue;
        }
        mColorAttachmentsSet.set(i);
        mColorFormats[i] = attachment->GetFormat().format;

        const DawnRenderPassColorAttachmentRenderToSingleSampled* msaaRenderToSingleSampledDesc =
            nullptr;
        FindInChain(colorAttachment.nextInChain, &msaaRenderToSingleSampledDesc);
        uint32_t attachmentSampleCount;
        if (msaaRenderToSingleSampledDesc != nullptr &&
            msaaRenderToSingleSampledDesc->implicitSampleCount > 1) {
            attachmentSampleCount = msaaRenderToSingleSampledDesc->implicitSampleCount;
            mIsMSAARenderToSingleSampledEnabled = true;
        } else {
            attachmentSampleCount = attachment->GetTexture()->GetSampleCount();
        }

        if (mSampleCount == 0) {
            mSampleCount = attachmentSampleCount;
        } else {
            DAWN_ASSERT(mSampleCount == attachmentSampleCount);
        }
    }

    // Gather the depth-stencil information.
    if (descriptor->depthStencilAttachment != nullptr) {
        TextureViewBase* attachment = descriptor->depthStencilAttachment->view;
        mDepthStencilFormat = attachment->GetFormat().format;
        if (mSampleCount == 0) {
            mSampleCount = attachment->GetTexture()->GetSampleCount();
        } else {
            DAWN_ASSERT(mSampleCount == attachment->GetTexture()->GetSampleCount());
        }
    }

    // Gather the PLS information.
    const RenderPassPixelLocalStorage* pls = nullptr;
    FindInChain(descriptor->nextInChain, &pls);
    if (pls != nullptr) {
        mHasPLS = true;
        mStorageAttachmentSlots = std::vector<wgpu::TextureFormat>(
            pls->totalPixelLocalStorageSize / kPLSSlotByteSize, wgpu::TextureFormat::Undefined);
        for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
            size_t slot = pls->storageAttachments[i].offset / kPLSSlotByteSize;
            const TextureViewBase* attachment = pls->storageAttachments[i].storage;
            mStorageAttachmentSlots[slot] = attachment->GetFormat().format;

            if (mSampleCount == 0) {
                mSampleCount = attachment->GetTexture()->GetSampleCount();
            } else {
                DAWN_ASSERT(mSampleCount == attachment->GetTexture()->GetSampleCount());
            }
        }
    }

    DAWN_ASSERT(mSampleCount > 0);
    SetContentHash(ComputeContentHash());
}

AttachmentState::AttachmentState(const AttachmentState& blueprint)
    : ObjectBase(blueprint.GetDevice()) {
    mColorAttachmentsSet = blueprint.mColorAttachmentsSet;
    mColorFormats = blueprint.mColorFormats;
    mDepthStencilFormat = blueprint.mDepthStencilFormat;
    mSampleCount = blueprint.mSampleCount;
    mIsMSAARenderToSingleSampledEnabled = blueprint.mIsMSAARenderToSingleSampledEnabled;
    mHasPLS = blueprint.mHasPLS;
    mStorageAttachmentSlots = blueprint.mStorageAttachmentSlots;
    SetContentHash(blueprint.GetContentHash());
}

void AttachmentState::DeleteThis() {
    Uncache();
    RefCounted::DeleteThis();
}

bool AttachmentState::EqualityFunc::operator()(const AttachmentState* a,
                                               const AttachmentState* b) const {
    // Check set attachments
    if (a->mColorAttachmentsSet != b->mColorAttachmentsSet) {
        return false;
    }

    // Check color formats
    for (ColorAttachmentIndex i : IterateBitSet(a->mColorAttachmentsSet)) {
        if (a->mColorFormats[i] != b->mColorFormats[i]) {
            return false;
        }
    }

    // Check depth stencil format
    if (a->mDepthStencilFormat != b->mDepthStencilFormat) {
        return false;
    }

    // Check sample count
    if (a->mSampleCount != b->mSampleCount) {
        return false;
    }

    // Both attachment state must either enable MSAA render to single sampled or disable it.
    if (a->mIsMSAARenderToSingleSampledEnabled != b->mIsMSAARenderToSingleSampledEnabled) {
        return false;
    }

    // Check PLS
    if (a->mHasPLS != b->mHasPLS) {
        return false;
    }
    if (a->mStorageAttachmentSlots.size() != b->mStorageAttachmentSlots.size()) {
        return false;
    }
    for (size_t i = 0; i < a->mStorageAttachmentSlots.size(); i++) {
        if (a->mStorageAttachmentSlots[i] != b->mStorageAttachmentSlots[i]) {
            return false;
        }
    }

    return true;
}

size_t AttachmentState::ComputeContentHash() {
    size_t hash = 0;

    // Hash color formats
    HashCombine(&hash, mColorAttachmentsSet);
    for (ColorAttachmentIndex i : IterateBitSet(mColorAttachmentsSet)) {
        HashCombine(&hash, mColorFormats[i]);
    }

    // Hash depth stencil attachment
    HashCombine(&hash, mDepthStencilFormat);

    // Hash sample count
    HashCombine(&hash, mSampleCount);

    // Hash MSAA render to single sampled flag
    HashCombine(&hash, mIsMSAARenderToSingleSampledEnabled);

    // Hash the PLS state
    HashCombine(&hash, mHasPLS);
    for (wgpu::TextureFormat slotFormat : mStorageAttachmentSlots) {
        HashCombine(&hash, slotFormat);
    }

    return hash;
}

ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments> AttachmentState::GetColorAttachmentsMask()
    const {
    return mColorAttachmentsSet;
}

wgpu::TextureFormat AttachmentState::GetColorAttachmentFormat(ColorAttachmentIndex index) const {
    DAWN_ASSERT(mColorAttachmentsSet[index]);
    return mColorFormats[index];
}

bool AttachmentState::HasDepthStencilAttachment() const {
    return mDepthStencilFormat != wgpu::TextureFormat::Undefined;
}

wgpu::TextureFormat AttachmentState::GetDepthStencilFormat() const {
    DAWN_ASSERT(HasDepthStencilAttachment());
    return mDepthStencilFormat;
}

uint32_t AttachmentState::GetSampleCount() const {
    return mSampleCount;
}

bool AttachmentState::IsMSAARenderToSingleSampledEnabled() const {
    return mIsMSAARenderToSingleSampledEnabled;
}

bool AttachmentState::HasPixelLocalStorage() const {
    return mHasPLS;
}

const std::vector<wgpu::TextureFormat>& AttachmentState::GetStorageAttachmentSlots() const {
    return mStorageAttachmentSlots;
}
}  // namespace dawn::native
