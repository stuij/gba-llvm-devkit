ifeq ($(strip $(GBA_LLVM)),)
      $(error Please set GBA_LLVM in your environment. export GBA_LLVM=<path to gba-llvm installation>)
endif

NAME = slides

BIN = $(GBA_LLVM)/bin
CC = $(BIN)/clang
GBAFIX = $(BIN)/gbafix
LFLAGS = -ltonc
CFLAGS = -O3 -mthumb --config armv4t-gba.cfg -Wl,-T,gba_cart.ld

WYVERN_LEFT = assets/wyvern-left.png
WYVERN_RIGHT = assets/wyvern-right.png
WYVERN_LEFT_ASM = wyvern-left.s
WYVERN_RIGHT_ASM = wyvern-right.s

C_FILES = $(NAME).c
ASM_FILES = $(WYVERN_LEFT_ASM) $(WYVERN_RIGHT_ASM)

$(NAME).gba: $(NAME).elf
	$(BIN)/llvm-objcopy -O binary $^ $@
	$(BIN)/gbafix $@

$(NAME).elf: $(C_FILES) $(ASM_FILES)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

$(WYVERN_LEFT_ASM): $(WYVERN_LEFT)
	grit $^ -gB4 -Mw 8 -Mh 8 -gT 00FFFF -pS -o $*

$(WYVERN_RIGHT_ASM): $(WYVERN_RIGHT)
	grit $^ -gB4 -Mw 4 -Mh 8 -gT 00FFFF -pS -o $*

.PHONY: clean

clean:
	rm $(NAME).elf $(NAME).gba wyvern*
