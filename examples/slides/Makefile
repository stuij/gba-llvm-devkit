ifeq ($(strip $(GBA_LLVM)),)
	GBA_LLVM=../..
endif

NAME = slides

BIN = $(GBA_LLVM)/bin
CC = $(BIN)/clang
GBAFIX = $(BIN)/gbafix
GRIT = $(BIN)/grit
LFLAGS = -ltonc -lAAS
CFLAGS = -O3 -mthumb --config armv4t-gba.cfg -Wl,-T,gba_cart.ld -funroll-loops

POINTER_LEFT = assets/pointer-left.png assets/wyvern-left.png
POINTER_RIGHT = assets/pointer-right.png assets/wyvern-right.png
POINTER_LEFT_ASM = pointer-left.s
POINTER_RIGHT_ASM = pointer-right.s

GBA_CPU_BG = assets/gba-cpu.png
GBA_CPU_BG_ASM = gba-cpu.s
COREMARK_BG = assets/coremark.png
COREMARK_BG_ASM = coremark.s

AAS_DATA = AAS_Data
AAS_DATA_ASM = AAS_Data.s

C_FILES = $(NAME).c
ASM_FILES = $(POINTER_LEFT_ASM) $(POINTER_RIGHT_ASM) $(GBA_CPU_BG_ASM) $(COREMARK_BG_ASM) $(AAS_DATA_ASM)

CONV2AAS = $(GBA_LLVM)/bin/conv2aas
ifeq ($(OS),Windows_NT)
	CONV2AAS = $(GBA_LLVM)/bin/conv2aas.exe
endif

$(NAME).gba: $(NAME).elf
	$(BIN)/llvm-objcopy -O binary $^ $@
	$(BIN)/gbafix $@

$(NAME).elf: $(C_FILES) $(ASM_FILES)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

$(POINTER_LEFT_ASM): $(POINTER_LEFT)
	$(GRIT) $^ -gB4 -Mw 8 -Mh 8 -pS -o pointer-left -fa -pT1

$(POINTER_RIGHT_ASM): $(POINTER_RIGHT)
	$(GRIT) $^ -gB4 -Mw 4 -Mh 8 -pS -o pointer-right -fa

$(GBA_CPU_BG_ASM): $(GBA_CPU_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $* -pT8

$(COREMARK_BG_ASM): $(COREMARK_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $*

$(AAS_DATA_ASM):
	$(CONV2AAS) $(AAS_DATA)

.PHONY: clean

clean:
	rm $(NAME).elf $(NAME).gba pointer*  gba-cpu* coremark*
