enable chromium_experimental_subgroups;

fn subgroupBroadcast_c36fe1() {
  var res : u32 = subgroupBroadcast(1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_c36fe1();
}
