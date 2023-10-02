char* text_slides[] = {
    "Introduction to the introduction\n\n"

    "we introduce the introduction by introducing things\n"
    "that are useful to know in advance of introducing\n"
    "the main text.\n"
    "which will be much more expansive than the\n"
    "introduction, and way more exapnsive than this\n"
    "introduction to the introduction.\n"
    "Which I would say is more of a preamble.\n"
    "I hope this clears things up. And that would be just"
    "in time, as we're about to introduce the actual\n"
    "introduction.",


    "Introduction\n\n"

    "So this is the real introduction!\n\n"
    "You know this is the introduction, because it has\n"
    "bullet points:\n"
    "  - this is bullet point 1\n"
    "  - and this is, you guessed it, nr 2!\n\n"
    "I think we're ready for some content!",


    "\n\n"
    "             Creating an embedded toolchain\n"
    "               for the Game Boy Advance\n"
    "\n\n\n\n\n\n"
    "               Taming a dragon bit by bit",

    "\n\n"
    "still some work to do..."
};

u32 text_slide_len = sizeof(text_slides)/sizeof(text_slides[0]);
u32 text_slide_cur = 0;

void write_slides() {
  tte_write("#{es;P}");
  tte_write(text_slides[text_slide_cur]);
}

void toggle_text() {
  REG_BLDCNT ^= 3 << 6;
  REG_DISPCNT ^= DCNT_BG0;
}

void text_on() {
  REG_BLDCNT |= 3 << 6;
  REG_DISPCNT |= DCNT_BG0;
}

void update_slides(s32 key_tri) {
  if (key_tri < 0) {
    if (text_slide_cur == 0)
      text_slide_cur = text_slide_len - 1;
    else
      text_slide_cur--;
    text_on();
    write_slides();
  } else if (key_tri > 0) {
    if (text_slide_cur == text_slide_len - 1)
      text_slide_cur = 0;
    else
      text_slide_cur++;
    text_on();
    write_slides();
  }
}
