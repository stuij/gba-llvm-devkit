ifeq ($(strip $(GBA_LLVM)),)
	GBA_LLVM = ../..
endif

NAME = test

BIN = $(GBA_LLVM)/bin
CC = $(BIN)/clang
GBAFIX = $(BIN)/gbafix
CFLAGS = -O3 -mthumb --config armv4t-gba.cfg -Wl,-T,gba_cart.ld -ltonc

C_FILES = $(NAME).c

$(NAME).gba: $(NAME).elf
	$(BIN)/llvm-objcopy -O binary $^ $@
	$(GBAFIX) $@

$(NAME).elf: $(C_FILES)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

.PHONY: clean

clean:
	rm $(NAME).elf $(NAME).gba
