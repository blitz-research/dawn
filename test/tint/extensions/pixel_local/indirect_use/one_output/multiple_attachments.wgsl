// flags: --pixel_local_attachments 0=1,1=6,2=3
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

fn f0() {
  P.a += 9;
}

fn f1() {
  f0();
  P.a += 8;
}

fn f2() {
  P.a += 7;
  f1();
}

@fragment fn f() -> @location(0) vec4f {
  f2();
  return vec4f(2);
}
