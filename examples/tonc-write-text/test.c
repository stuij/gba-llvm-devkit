#include "tonc.h"

int main() {
  irq_init(NULL);
  irq_add(II_VBLANK, NULL);
  REG_DISPCNT= DCNT_MODE0 | DCNT_BG0;

  tte_init_se(
        0,                      // Background number (BG 0)
        BG_CBB(0)|BG_SBB(31),   // BG control (for REG_BGxCNT)
        0,                      // Tile offset (special cattr)
        CLR_GRAY,             // Ink color
        14,                     // BitUnpack offset (on-pixel = 15)
        NULL,                   // Default font (sys8)
        NULL);                  // Default renderer (se_drawg_s)

  tte_write("Hello Tonc World!");

  while(1)
    vid_vsync();;

  return 1;
}
