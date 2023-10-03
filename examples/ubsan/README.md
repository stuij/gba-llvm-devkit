# What is this

This is a very simple undefined behaviour sanitizer test.

When compiling the example we turn on the [undefined behaviour
sanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html), and
specify that when we detect undefined behaviour (read past the end of an array)
we execute the `draw_screen` function, which will paint the screen red:

    -fsanitize=undefined -fsanitize-trap=undefined -ftrap-function=draw_screen

# How to build

type:

```make```
