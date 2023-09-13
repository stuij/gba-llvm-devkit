#include <string.h>
#include <tonc.h>

#include "wyvern-left.h"
#include "wyvern-right.h"

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

int main() {
  // (1) Places the tiles of a 4bpp boxed metroid sprite 
  //   into LOW obj memory (cbb == 4)
  memcpy32(&tile_mem[4][0], wyvern_leftTiles, wyvern_leftTilesLen);
  memcpy16(pal_obj_mem, wyvern_leftSharedPal, wyvern_leftSharedPalLen);

  memcpy32(&tile_mem[4][256], wyvern_rightTiles, wyvern_rightTilesLen);
  // even though in Aseprite the colors are presented equal, somewhere in the
  // conversion process some color indexes get swapped, so we can't use the same
  // pal bank for both. Also I couldn't find a way in grit to merge pal data from
  // sprites of different sizes.
  memcpy16(pal_obj_mem + 16, wyvern_rightSharedPal, wyvern_rightSharedPalLen);

  // (2) Initialize all sprites
  oam_init(obj_buffer, 128);
  REG_DISPCNT= DCNT_OBJ | DCNT_OBJ_1D;


  u32 tid= 64, pb= 0;      // (3) tile id, pal-bank
  OBJ_ATTR *wyvern_l= &obj_buffer[0];
  obj_set_attr(wyvern_l, 
               ATTR0_SQUARE,               // Square, regular sprite
               ATTR1_SIZE_64,              // 64x64p, 
               ATTR2_PALBANK(pb) | tid);   // palbank 0, tile 0

  OBJ_ATTR *wyvern_r= &obj_buffer[1];
  obj_set_attr(wyvern_r, 
               ATTR0_TALL,                 // Tall
               ATTR1_SIZE_64,              // 32x64p, 
               ATTR2_PALBANK(1) | 256 + 32);   // palbank 0, tile 0

  
  int x= 72, y= 32;
  obj_set_pos(wyvern_l, x, y);
  obj_set_pos(wyvern_r, x + 64, y);
  oam_copy(oam_mem, obj_buffer, 2);

  while(1){
    vid_vsync();
  };

}
