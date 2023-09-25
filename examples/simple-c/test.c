void draw_screen() {
  // set background mode 3 (bitmap) and turn on background 2
  *(unsigned long*) 0x4000000 = 0x403;

  unsigned short* vram = (unsigned short*) 0x6000000;
  // write red to every pixel on the 240x160 screen
  for(int x = 0; x < 240; x++)
    for(int y = 0; y < 160; y++)
      vram[x + y * 240] = 31;
}

int main() {
  draw_screen();
  while(1) {};
  return 1;
}
