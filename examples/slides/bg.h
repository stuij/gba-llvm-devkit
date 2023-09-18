// backgrounds
#include "coremark.h"
#include "gba-cpu.h"

struct Background {
  const unsigned int* tiles;
  u32 tiles_len;

  const unsigned short* map;
  u32 map_len;

  const unsigned short* pal;
  u32 pal_len;
};

#define BG_LIST(NAME) { NAME##Tiles, NAME##TilesLen, NAME##Map, NAME##MapLen, NAME##Pal, NAME##PalLen }

struct Background bg[] = {
  BG_LIST(gba_cpu),
  BG_LIST(coremark),
};

u32 bg_cur = 0;
u32 bg_len = sizeof(bg)/sizeof(bg[0]);
  
void load_bg() {
    // Load tiles into CBB 2
    memcpy32(&tile_mem[2][0], bg[bg_cur].tiles, bg[bg_cur].tiles_len / 4);
    // Load palette
    memcpy16(pal_bg_mem, bg[bg_cur].pal, bg[bg_cur].pal_len / 2);
    // Load gba_cpu into SBB 14
    memcpy16(&se_mem[14][0], bg[bg_cur].map, bg[bg_cur].map_len / 2);
}

void update_bg(s32 key_tri) {
  if (key_tri < 0) {
    if (bg_cur == 0)
      bg_cur = bg_len - 1;
    else
      bg_cur--;
    load_bg();
  } else if (key_tri > 0) {
    if (bg_cur == bg_len - 1)
      bg_cur = 0;
    else
      bg_cur++;
    load_bg();
  }
}

