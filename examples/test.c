void draw_screen() {
  *(unsigned long*) 0x4000000 = 0x403;

  unsigned short* vram = (unsigned short*) 0x6000000;
  for(int x = 0; x < 240; x++)
    for(int y = 0; y < 160; y++)
      vram[x + y * 240] = 31;
}

int main() {
  draw_screen();
  while(1) {};
  return 1;
}
