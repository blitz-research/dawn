// flags: --overrides WGSL_SPEC_CONSTANT_0=0
override o : i32;

@stage(compute) @workgroup_size(1)
fn main() {
    _ = o;
}
