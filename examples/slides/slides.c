#include <stdbool.h>
#include <tonc.h>

#include "AAS.h"
#include "AAS_Data.h"
#include "text.h"
#include "bg.h"

// sprites
#include "pointer-left.h"
#include "pointer-right.h"

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

u32 tid_offset_left(u32 frame, bool wyvern_p) {
  // frame nr * tiles per frame
  u32 offset = wyvern_p ? 0 : 512;
  return frame * 64 + offset;
}

u32 tid_offset_right(u32 frame, bool wyvern_p) {
  u32 offset = wyvern_p ? 0 : 512;
  // frame nr * tiles per frame + left sprite total tiles
  return frame * 32 + 256 + offset;
}

void animate_wyvern(u32* frame_offset, bool* reverse, u32 time, bool flapping) {
  u32 duration = 10;
  u32 offset = *frame_offset;
  u32 total_frames = 4;

  if (!flapping && *frame_offset == 1) {
    return;
  }

  if (time % duration == 0) {
    if (offset == 0) {
      *frame_offset = 1;
      *reverse = false;
    } else if (offset == total_frames - 1) {
      *frame_offset = total_frames - 2;
      *reverse = true;
    } else if (*reverse == true) {
      *frame_offset = offset - 1;
    } else {
      *frame_offset = offset + 1;
    }
  }
}

void load_sprite_data() {
  // Place the tiles of the 4bpp LLVM wyvern into LOW obj memory (cbb == 4)
  // As the wyvern is too big for just one sprite, we had to split it up
  // into a 64x64 sprite and a 32x64 sprite.
  memcpy32(&tile_mem[4][0], wyvern_leftTiles, wyvern_leftTilesLen / 4);
  memcpy16(&tile_mem[4][256], wyvern_rightTiles, wyvern_rightTilesLen / 2);

  // the pointer would really occupy a much smaller tile than the wyvern of
  // course, but to keep the implementation elegant(ish), we're keeping it the same
  // size and we animate it with the same static image!
  memcpy32(&tile_mem[5][0], pointer_leftTiles, pointer_leftTilesLen / 4);
  memcpy16(&tile_mem[5][256], pointer_rightTiles, pointer_rightTilesLen);

  // pal data (shared between pointer and wyvern)
  memcpy16(pal_obj_mem, pointer_leftSharedPal, pointer_leftSharedPalLen / 2);
  // Even though in Aseprite the colors are presented equal, somewhere in the
  // conversion process some color indexes get swapped, so we can't use the same
  // pal bank for both. Also I couldn't find a way in grit to merge pal data from
  // sprites of different sizes.
  memcpy16(pal_obj_bank[1], pointer_rightSharedPal, pointer_rightSharedPalLen / 2);
}

void init_slides() {
  // Init for text
  tte_init_chr4c(
      0,							// BG number.
      BG_CBB(0)|BG_SBB(15),		// BG control.
      0xF000,					// Screen-entry offset
      bytes2word(13,15,0,0),		// Color attributes.
      CLR_WHITE,					// Ink color
      &verdana9_b4Font,			// Verdana 9, with shade.
      (fnDrawg)chr4c_drawg_b4cts_fast);	// b4cts renderer, asm version

  tte_set_margins(4, 8, 236, 152); // left, top, right, bottom (absolute)
  write_slides();
}

int main();
bool ubsan_triggered = false;
void ubsan_alert() {
  ubsan_triggered = true;
  main();
}

void show_ubsan_triggered() {
  text_on();
  tte_write("#{es;P}");
  tte_write("\n\nOops!\nAlmost wrote past the end of an array!\n\n"
            "Thanks ubsan sanitizer for catching that!");
  ubsan_triggered = false;
}

int get_ubsan_arr(int index) {
  const int arr[] = {
      0,
      1,
  };
  return arr[index];
}

int main() {
  // load things
  load_bg();
  load_sprite_data();

  // Initialize all sprites
  oam_init(obj_buffer, 128);

  AAS_SetConfig(AAS_CONFIG_MIX_32KHZ, AAS_CONFIG_CHANS_8,
                AAS_CONFIG_SPATIAL_STEREO, AAS_CONFIG_DYNAMIC_OFF);

  irq_init(NULL);
  irq_add(II_VBLANK, NULL);
  irq_add(II_TIMER1, AAS_Timer1InterruptHandler);

  // set up BG0 for a 4bpp 32x32t map,
  // using charblock 2 and screenblock 15
  REG_BG3CNT= BG_CBB(2) | BG_SBB(14) | BG_4BPP | BG_REG_32x32;
  REG_DISPCNT= DCNT_MODE0 | DCNT_BG3 | DCNT_OBJ | DCNT_OBJ_1D;

  init_slides();

  // prep blend register
  REG_BLDCNT= BLD_BUILD(
      BLD_BG3,    // Top layers
      0,          // Bottom layers (NONE)
      0);         // mode (OFF)

  REG_BLDY= 0xa;

  // Set object structs with relevant settings/data
  // first calculate initial tile id offsets
  u32 frame_offset = 1;
  bool wyvern_p = false; // show wyvern or pointer
  u32 tid_left= tid_offset_left(frame_offset, wyvern_p);
  u32 tid_right= tid_offset_right(frame_offset, wyvern_p);

  // pal banks
  u32 pb_left= 0, pb_right= 1;

  OBJ_ATTR *wyvern_l= &obj_buffer[0];
  obj_set_attr(wyvern_l,
               ATTR0_SQUARE | ATTR0_BLEND,               // Square, regular sprite
               ATTR1_SIZE_64,              // 64x64p,
               ATTR2_PALBANK(pb_left) | tid_left);   // palbank 0, tile 64 (second frame)

  OBJ_ATTR *wyvern_r= &obj_buffer[1];
  obj_set_attr(wyvern_r,
               ATTR0_TALL | ATTR0_BLEND,                 // Tall
               ATTR1_SIZE_64,              // 32x64p,
               ATTR2_PALBANK(pb_right) | tid_right);   // palbank 1, tile 256 + 32

  // obj and bg positioning vars
  s32 x_cursor = 240, y_cursor = 42;
  s32 x_cursor_min = 0, y_cursor_min = 0;
  s32 x_cursor_max = 240, y_cursor_max = 160;
  s32 x_view = 0, y_view = 0; // viewport offset (upper left corner) in map
  u32 speed = 2;

  u32 wyvern_r_pixel_offset = 64;

  int time = 0;
  bool reverse = false;
  bool flapping = false;

  bool paused = 0;

  if(ubsan_triggered)
    show_ubsan_triggered();
  
  while(1){
    vid_vsync();
    key_poll();
    time += 1;


    if(key_held(KEY_SELECT)) {
      if(key_hit(KEY_A))
        flapping = !flapping;

      if(key_hit(KEY_B)) {
        // flip sprite
        wyvern_p = !wyvern_p;
        flapping = false;
      }
      
      if(key_hit(KEY_L))
        get_ubsan_arr(2);

      if(key_hit(KEY_R)) {
        if (AAS_MOD_IsPlaying())
          AAS_MOD_Stop();
        else
          AAS_MOD_Play(AAS_DATA_MOD_FlatOutLies);
      }
    } else {
      s32 horz_key = key_tri_horz();
      s32 vert_key = key_tri_vert();

      // move left/right
      if(x_cursor + speed * horz_key >= x_cursor_max ||
         x_cursor + speed * horz_key <= x_cursor_min) {
        // move viewport horizontally
        x_view += speed * horz_key;
      } else {
        // move cursor horizontally
        x_cursor += speed * horz_key;
      }
      x_view = clamp(x_view, 0, 256 + 1 - SCREEN_WIDTH);

      // move up/down
      if(y_cursor + speed * vert_key >= y_cursor_max ||
         y_cursor + speed * vert_key <= y_cursor_min) {
        // move viewport vertically
        y_view += speed * vert_key;
      } else {
        // move cursor vertically
        y_cursor += speed * vert_key;
      }
      y_view = clamp(y_view, 0, 256 + 1 - SCREEN_HEIGHT);

      if (horz_key != 0 || vert_key != 0)
        flapping = true;


      // change slides and background
       update_slides(bit_tribool(key_hit(-1), KI_A, KI_B));
       update_bg(bit_tribool(key_hit(-1), KI_R, KI_L));

      if(key_hit(KEY_START))
        toggle_text();
    }

    animate_wyvern(&frame_offset, &reverse, time, flapping);

    // set cursor
    wyvern_l->attr2= ATTR2_BUILD(tid_offset_left(frame_offset, wyvern_p), pb_left, 0);
    wyvern_r->attr2= ATTR2_BUILD(tid_offset_right(frame_offset, wyvern_p), pb_right, 0);

    obj_set_pos(wyvern_l, x_cursor, y_cursor);
    obj_set_pos(wyvern_r, x_cursor + wyvern_r_pixel_offset, y_cursor);
    oam_copy(oam_mem, obj_buffer, 2);

    // set background scroll
    REG_BG3HOFS= x_view;
    REG_BG3VOFS= y_view;
  };
}
