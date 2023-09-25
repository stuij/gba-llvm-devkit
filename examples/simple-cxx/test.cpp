#include <vector>

void draw_screen() {
  // set background mode 3 (bitmap) and turn on background 2
  *(unsigned long*) 0x4000000 = 0x403;

  // put colors red, green and blue in a vector
  std::vector<int> v = {31};
  v.push_back(31 << 5);
  v.insert(v.end(), 31 << 10);

  // pixel by pixel, paint red, green and blue stripes to the 240x160 screen
  unsigned short* vram = (unsigned short*) 0x6000000;
  for(int x = 0; x < 240; x++)
    for(int y = 0; y < 160; y++)
      vram[x + y * 240] = v[y % 3];
}

int main(void) {
  draw_screen();
  while(1) {};
  return 1;
}
