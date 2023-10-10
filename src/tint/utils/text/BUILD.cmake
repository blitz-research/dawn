# Copyright 2023 The Tint Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

################################################################################
# File generated by 'tools/src/cmd/gen' using the template:
#   tools/src/cmd/gen/build/BUILD.cmake.tmpl
#
# To regenerate run: './tools/run gen'
#
#                       Do not modify this file directly
################################################################################

################################################################################
# Target:    tint_utils_text
# Kind:      lib
################################################################################
tint_add_target(tint_utils_text lib
  utils/text/string.cc
  utils/text/string.h
  utils/text/string_stream.cc
  utils/text/string_stream.h
  utils/text/unicode.cc
  utils/text/unicode.h
)

tint_target_add_dependencies(tint_utils_text lib
  tint_utils_containers
  tint_utils_ice
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_rtti
  tint_utils_traits
)

################################################################################
# Target:    tint_utils_text_test
# Kind:      test
################################################################################
tint_add_target(tint_utils_text_test test
  utils/text/string_stream_test.cc
  utils/text/string_test.cc
  utils/text/unicode_test.cc
)

tint_target_add_dependencies(tint_utils_text_test test
  tint_utils_containers
  tint_utils_ice
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_rtti
  tint_utils_text
  tint_utils_traits
)

tint_target_add_external_dependencies(tint_utils_text_test test
  "gtest"
)
