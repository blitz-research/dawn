#version 310 es

void tint_symbol() {
  int j = 0;
  {
    for(int i = 0; (i < 2); i = (i + 2)) {
      switch(i) {
        case 0: {
          switch(j) {
            case 0: {
              continue;
              break;
            }
            default: {
              break;
            }
          }
          break;
        }
        default: {
          break;
        }
      }
    }
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
