# What is this

This is the slide-software for a presentation on this project. As the slides are
meant to show off that the toolchain is actually working, it made sense to
include it as an example.

The program shows off:

- using backgrounds and sprites
- converting images with Grit to use as sprites and backgrounds
- converting and playing mod files on the GBA, which involves meeting tight
  deadlines to fill up the audio sample queue by heavily using interrupts and
  DMA transfers
  

# How to build

type:

```make```


# How to use

- `d-pad`:            move the cursor
- `start`:            toggle slide layer on and off
- `b` and `a`:        cycle back and forth through slides
- `l` and `r`:        cycle back and forth through background images
- `select` + `b`:     toggle between wyvern and pointer
- `select` + `a`:     tell wyvern to stop flapping
- `select` + `l`:     read past end of array, triggering ubsan sanitizer
- `select` + `r`:     toggle music
- `select` + `start`: reset background window to coordinates 0,0

# Attribution

- The wyvern sprite was adapted from the [LLVM-MOS
  logo](https://llvm-mos.org/wiki/File:LLV M-MOS_logo.png), which itself was
  adapted from the [LLVM logo](https://llvm.org/Logo.html).