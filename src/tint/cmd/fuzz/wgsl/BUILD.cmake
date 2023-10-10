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
# Target:    tint_cmd_fuzz_wgsl_fuzz_cmd
# Kind:      fuzz_cmd
################################################################################
tint_add_target(tint_cmd_fuzz_wgsl_fuzz_cmd fuzz_cmd
  cmd/fuzz/wgsl/main_fuzz.cc
)

tint_target_add_dependencies(tint_cmd_fuzz_wgsl_fuzz_cmd fuzz_cmd
  tint_cmd_fuzz_wgsl_fuzz
  tint_lang_core
  tint_lang_core_constant
  tint_lang_core_type
  tint_lang_wgsl
  tint_lang_wgsl_ast
  tint_lang_wgsl_program
  tint_lang_wgsl_sem
  tint_utils_containers
  tint_utils_diagnostic
  tint_utils_ice
  tint_utils_id
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_result
  tint_utils_rtti
  tint_utils_symbol
  tint_utils_text
  tint_utils_traits
)

tint_target_set_output_name(tint_cmd_fuzz_wgsl_fuzz_cmd fuzz_cmd "tint_wgsl_fuzzer")

################################################################################
# Target:    tint_cmd_fuzz_wgsl_fuzz
# Kind:      fuzz
################################################################################
tint_add_target(tint_cmd_fuzz_wgsl_fuzz fuzz
  cmd/fuzz/wgsl/wgsl_fuzz.cc
  cmd/fuzz/wgsl/wgsl_fuzz.h
)

tint_target_add_dependencies(tint_cmd_fuzz_wgsl_fuzz fuzz
  tint_api_common
  tint_lang_core
  tint_lang_core_constant
  tint_lang_core_ir
  tint_lang_core_type
  tint_lang_wgsl
  tint_lang_wgsl_ast
  tint_lang_wgsl_program
  tint_lang_wgsl_reader
  tint_lang_wgsl_sem
  tint_utils_containers
  tint_utils_diagnostic
  tint_utils_ice
  tint_utils_id
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_reflection
  tint_utils_result
  tint_utils_rtti
  tint_utils_symbol
  tint_utils_text
  tint_utils_traits
)
