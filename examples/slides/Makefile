ifeq ($(strip $(GBA_LLVM)),)
	GBA_LLVM=../..
endif

NAME = slides

BIN = $(GBA_LLVM)/bin
CC = $(BIN)/clang
GBAFIX = $(BIN)/gbafix
GRIT = $(BIN)/grit
LFLAGS = -ltonc -lAAS
CFLAGS = -O3 -g -mthumb --config armv4t-gba.cfg -Wl,-T,gba_cart.ld -funroll-loops \
    -fsanitize=array-bounds -fsanitize-trap=array-bounds -ftrap-function=ubsan_alert

POINTER_LEFT = assets/pointer-left.png assets/wyvern-left.png
POINTER_RIGHT = assets/pointer-right.png assets/wyvern-right.png
POINTER_LEFT_ASM = pointer-left.s
POINTER_RIGHT_ASM = pointer-right.s

GBA_CPU_BG = assets/gba-cpu.png
GBA_CPU_BG_ASM = gba-cpu.s
COREMARK_GCC_BG = assets/coremark-gcc.png
COREMARK_GCC_BG_ASM = coremark-gcc.s
COREMARK_CLANG_BG = assets/coremark-clang.png
COREMARK_CLANG_BG_ASM = coremark-clang.s
COREMARK_COMP_BG = assets/coremark-comp.png
COREMARK_COMP_BG_ASM = coremark-comp.s
GBA_MEM_MAP_BG = assets/gba-mem-layout.png
GBA_MEM_MAP_BG_ASM = gba-mem-layout.s
DEP_GRAPH_BG = assets/dep-graph.png
DEP_GRAPH_BG_ASM = dep-graph.s
DEP_GRAPH_RUNTIMES_BG = assets/dep-graph-runtimes.png
DEP_GRAPH_RUNTIMES_BG_ASM = dep-graph-runtimes.s
NOKIAS_BG = assets/nokias.png
NOKIAS_BG_ASM = nokias.s
TIMELINE_BG = assets/timeline.png
TIMELINE_BG_ASM = timeline.s
QR_CODE_BG = assets/qr-code.png
QR_CODE_BG_ASM = qr-code.s

AAS_DATA = AAS_Data
AAS_DATA_ASM = AAS_Data.s

C_FILES = $(NAME).c
ASM_FILES = $(POINTER_LEFT_ASM) $(POINTER_RIGHT_ASM) $(GBA_CPU_BG_ASM) \
    $(COREMARK_GCC_BG_ASM) $(COREMARK_CLANG_BG_ASM) $(COREMARK_COMP_BG_ASM) \
    $(DEP_GRAPH_BG_ASM) $(DEP_GRAPH_RUNTIMES_BG_ASM) \
	$(NOKIAS_BG_ASM) $(TIMELINE_BG_ASM) \
	$(AAS_DATA_ASM) $(GBA_MEM_MAP_BG_ASM) $(QR_CODE_BG_ASM) \

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

$(GBA_MEM_MAP_BG_ASM): $(GBA_MEM_MAP_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $* -pT5

$(COREMARK_GCC_BG_ASM): $(COREMARK_GCC_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $*

$(COREMARK_CLANG_BG_ASM): $(COREMARK_CLANG_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $*

$(COREMARK_COMP_BG_ASM): $(COREMARK_COMP_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $*

$(DEP_GRAPH_BG_ASM): $(DEP_GRAPH_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $*

$(DEP_GRAPH_RUNTIMES_BG_ASM): $(DEP_GRAPH_RUNTIMES_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $*

$(QR_CODE_BG_ASM): $(QR_CODE_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $*

$(NOKIAS_BG_ASM): $(NOKIAS_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $*

$(TIMELINE_BG_ASM): $(TIMELINE_BG)
	$(GRIT) $^ -mR4 -gB4 -pn16 -o $*

$(AAS_DATA_ASM):
	$(CONV2AAS) $(AAS_DATA)

.PHONY: clean

clean:
	rm $(NAME).elf $(NAME).gba pointer* gba-cpu* coremark* \
      AAS_Data.* gba-mem-layout* qr-code* dep-graph* nokias* slides.sav \
      timeline*
