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
# Target:    tint_utils_ice
# Kind:      lib
################################################################################
tint_add_target(tint_utils_ice lib
  utils/ice/ice.cc
  utils/ice/ice.h
)

tint_target_add_dependencies(tint_utils_ice lib
  tint_utils_debug
  tint_utils_macros
)

################################################################################
# Target:    tint_utils_ice_test
# Kind:      test
################################################################################
tint_add_target(tint_utils_ice_test test
  utils/ice/ice_test.cc
)

tint_target_add_dependencies(tint_utils_ice_test test
  tint_utils_ice
  tint_utils_macros
)

tint_target_add_external_dependencies(tint_utils_ice_test test
  "gtest"
)
