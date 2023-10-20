#include <etl/vector.h>
#include <tonc.h>

// These next functions make it possible to in general build with `-nostdlib`
// They are not all necessary for this particular example.
// They should be made into a minimal nostdlib stub library.

// to compile with -nostdlib we need to define __libc_init_array()
// to initialize our globals
extern const void (*__preinit_array_start[])(void);
extern const void (*__preinit_array_end[])(void);
extern const void (*__init_array_start[])(void);
extern const void (*__init_array_end[])(void);

extern "C"
void __libc_init_array(void) {
  size_t i, n;

  n = __preinit_array_end - __preinit_array_start;
  for (i = 0; i < n; i++)
    __preinit_array_start[i] ();

  n = __init_array_end - __init_array_start;
  for (i = 0; i < n; i++)
    __init_array_start[i] ();
}


// next two are stubs for fns used when exiting from main
// (which we (should) never do)
extern "C"
int __aeabi_atexit(
    void *object,
    void (*destructor)(void *),
    void *dso_handle) {
  static_cast<void>(object);
  static_cast<void>(destructor);
  static_cast<void>(dso_handle);
  return 0;
}

// handles destructor chain
void* __dso_handle = nullptr;


// pure virtual function stubbing
// this one is called when bug
extern "C"
void __cxa_pure_virtual() {
  // we could do some better error reporting
  while (true) {}
}

// might sometimes get included, even though we don't do dynamic memory
// allocation
void operator delete(void *) {}


void draw_screen() {
  // put colors red, green and blue in a vector
  etl::vector<unsigned short, 5> v = {31};
  v.push_back(31 << 5);
  v.insert(v.end(), 31 << 10);

  // pixel by pixel, paint red, green and blue stripes to the 240x160 screen
  volatile unsigned short* vram = (unsigned short*) 0x6000000;
  for(int x = 0; x < 240; x++)
    for(int y = 0; y < 160; y++)
     vram[x + y * 240] = v[y % 3];
}

int main(void) {
  // set background mode 3 (bitmap) and turn on background 2
  *(unsigned long*) 0x4000000 = 0x403;

  draw_screen();
  while(1) {
    vid_vsync();
  };
  return 1;
}
