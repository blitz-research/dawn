SKIP: FAILED


enable chromium_experimental_subgroups;

fn subgroupBroadcast_c36fe1() {
  var arg_0 = 1u;
  const arg_1 = 1u;
  var res : u32 = subgroupBroadcast(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_c36fe1();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBroadcast/c36fe1.wgsl:38:8 error: GLSL backend does not support extension 'chromium_experimental_subgroups'
enable chromium_experimental_subgroups;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

