// Copyright 2022 The Tint Authors.
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

#include "src/tint/type/reference.h"

#include "src/tint/core/diagnostic/diagnostic.h"
#include "src/tint/core/hash.h"
#include "src/tint/core/string_stream.h"
#include "src/tint/debug.h"
#include "src/tint/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Reference);

namespace tint::type {

Reference::Reference(builtin::AddressSpace address_space,
                     const Type* subtype,
                     builtin::Access access)
    : Base(utils::Hash(utils::TypeInfo::Of<Reference>().full_hashcode,
                       address_space,
                       subtype,
                       access),
           type::Flags{}),
      subtype_(subtype),
      address_space_(address_space),
      access_(access) {
    TINT_ASSERT(Type, !subtype->Is<Reference>());
    TINT_ASSERT(Type, access != builtin::Access::kUndefined);
}

bool Reference::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<Reference>()) {
        return o->address_space_ == address_space_ && o->subtype_ == subtype_ &&
               o->access_ == access_;
    }
    return false;
}

std::string Reference::FriendlyName() const {
    utils::StringStream out;
    out << "ref<";
    if (address_space_ != builtin::AddressSpace::kUndefined) {
        out << address_space_ << ", ";
    }
    out << subtype_->FriendlyName() << ", " << access_;
    out << ">";
    return out.str();
}

Reference::~Reference() = default;

Reference* Reference::Clone(CloneContext& ctx) const {
    auto* ty = subtype_->Clone(ctx);
    return ctx.dst.mgr->Get<Reference>(address_space_, ty, access_);
}

}  // namespace tint::type
