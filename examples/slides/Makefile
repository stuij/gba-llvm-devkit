ifeq ($(strip $(GBA_LLVM)),)
      $(error Please set GBA_LLVM in your environment. export GBA_LLVM=<path to gba-llvm installation>)
endif

NAME = slides

BIN = $(GBA_LLVM)/bin
CC = $(BIN)/clang
GBAFIX = $(BIN)/gbafix
LFLAGS = -ltonc
CFLAGS = -O3 -mthumb --config armv4t-gba.cfg -Wl,-T,gba_cart.ld

POINTER_LEFT = assets/pointer-left.png assets/wyvern-left.png
POINTER_RIGHT = assets/pointer-right.png assets/wyvern-right.png
GBA_CPU_BG = assets/gba-cpu.png
POINTER_LEFT_ASM = pointer-left.s
POINTER_RIGHT_ASM = pointer-right.s
GBA_CPU_BG_ASM = gba-cpu.s

C_FILES = $(NAME).c
ASM_FILES = $(POINTER_LEFT_ASM) $(POINTER_RIGHT_ASM) $(GBA_CPU_BG_ASM)

$(NAME).gba: $(NAME).elf
	$(BIN)/llvm-objcopy -O binary $^ $@
	$(BIN)/gbafix $@

$(NAME).elf: $(C_FILES) $(ASM_FILES)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

$(POINTER_LEFT_ASM): $(POINTER_LEFT)
	grit $^ -gB4 -Mw 8 -Mh 8 -pS -o pointer-left -fa -pT1

$(POINTER_RIGHT_ASM): $(POINTER_RIGHT)
	grit $^ -gB4 -Mw 4 -Mh 8 -pS -o pointer-right -fa

$(GBA_CPU_BG_ASM): $(GBA_CPU_BG)
	grit $^ -mR4 -gB4 -pn16 -o $* -pT8

.PHONY: clean

clean:
	rm $(NAME).elf $(NAME).gba pointer*  gba-cpu*
