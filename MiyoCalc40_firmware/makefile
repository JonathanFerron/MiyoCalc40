# ==============================================================================
# MiyoCalc40 firmware — makefile
#
# Now using a pure avr-gcc toolchain
# `make` builds a complete .hex
# `make flash`/`make flash_fuses` program it over UPDI.
# avrducore/ is a hand-rolled replacement for DxCore's init() and Arduino
# core: clock_init() (CCP-protected clock setup), fuses.c, and
# spi.c/gpio.h (SPI0 + direct-register GPIO replacing <SPI.h>/digitalWrite).
# ==============================================================================

MAKEFLAGS += -j$(shell nproc --ignore=2)

# ------------------------------------------------------------------------------
# Toolchain / hardware / programmer
# ------------------------------------------------------------------------------
CC      = avr-gcc
CXX     = avr-g++
AS      = avr-gcc          # assembles .S files through the C driver
OBJCOPY = avr-objcopy
SIZE    = avr-size
AVRDUDE = avrdude

MCU   = avr128da28
F_CPU = 24000000UL

PROG_ID   = serialupdi
PROG_PORT = /dev/ttyUSB0   # UPDI adapter; check dmesg | tail on plug-in if it differs

BUILD  = build
TARGET = MiyoCalc40_firmware

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
CXX_SRC = \
    backlight.cpp \
    calc.cpp \
    cards.cpp \
    config.cpp \
    lcd.cpp \
    main.cpp \
    matrix.cpp \
    programming.cpp \
    util.cpp

# avrducore/: hand-rolled replacement for DxCore's init() and Arduino core
# (clock, CCP writes, fuses, SPI0). See avrducore/clock.c and avrducore/fuses.c
# for the derivation of every value — cross-checked against ioavr128da28.h and
# the working Arduino/DxCore build's fuse write, not assumed.
C_SRC = \
    avrducore/clock.c \
    avrducore/fuses.c \
    avrducore/spi.c

S_SRC = \
    avrducore/ccp.S

INC = -Iavrducore

# ------------------------------------------------------------------------------
# Flags
# ------------------------------------------------------------------------------
# OPT is overridable so `make debug` can drop in -O0 -g without duplicating the
# whole flag list. `make debug` builds under build/debug/ so it never clobbers
# a normal -Os build's objects.
OPT ?= -Os

COMMON_CFLAGS  = -mmcu=$(MCU)
COMMON_CFLAGS += -DF_CPU=$(F_CPU)
COMMON_CFLAGS += $(OPT)
COMMON_CFLAGS += -Wall
COMMON_CFLAGS += -Wextra
COMMON_CFLAGS += -Wshadow
COMMON_CFLAGS += -Wundef               # #if on an undefined macro (typo guard)
COMMON_CFLAGS += -ffunction-sections
COMMON_CFLAGS += -fdata-sections
COMMON_CFLAGS += $(INC)

CFLAGS  = $(COMMON_CFLAGS)
CFLAGS += -std=gnu11
CFLAGS += -Wstrict-prototypes
CFLAGS += -MMD -MP

CXXFLAGS  = $(COMMON_CFLAGS)
CXXFLAGS += -std=gnu++17
CXXFLAGS += -fno-exceptions            # no unwind support on AVR; keep code small
CXXFLAGS += -fno-rtti                  # no RTTI needed, saves flash
CXXFLAGS += -fno-threadsafe-statics    # single-core MCU, no guard variables needed
CXXFLAGS += -MMD -MP

ASFLAGS  = -mmcu=$(MCU)
ASFLAGS += -DF_CPU=$(F_CPU)
ASFLAGS += -x assembler-with-cpp

LDFLAGS  = -mmcu=$(MCU)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--relax

# ------------------------------------------------------------------------------
# Build products
# ------------------------------------------------------------------------------
CXX_OBJ = $(patsubst %.cpp,$(BUILD)/%.o,$(CXX_SRC))
C_OBJ   = $(patsubst %.c,$(BUILD)/%.o,$(C_SRC))
S_OBJ   = $(patsubst %.S,$(BUILD)/%.o,$(S_SRC))
OBJ     = $(CXX_OBJ) $(C_OBJ) $(S_OBJ)
ELF     = $(BUILD)/$(TARGET).elf
HEX     = $(BUILD)/$(TARGET).hex

.PHONY: all clean size debug flash flash_fuses bear format help

all: $(HEX)
	@echo ""
	@echo "=== $(TARGET) size ==="
	$(SIZE) --format=avr --mcu=$(MCU) $(ELF)

size: all

# Unoptimized, symbol-rich build for disassembly/debugging (-Os aggressively
# CSEs and unrolls small loops, which makes objdump output hard to trace by
# hand). Lands in build/debug/ so it never clobbers a normal build's objects;
# `avr-objdump -d -S build/debug/MiyoCalc40_firmware.elf` interleaves source.
debug:
	$(MAKE) OPT="-O0 -g" BUILD=$(BUILD)/debug all

$(ELF): $(OBJ)
	@mkdir -p $(dir $@)
	@echo "[LD]  $@"
	$(CXX) $(LDFLAGS) $^ -o $@

# .fuse is deliberately NOT stripped (only .eeprom is) — flash_fuses below
# reads the fuse row straight out of this same hex, per avrdude 8.x's
# multi-memory hex support.
$(HEX): $(ELF)
	@echo "[HEX] $@"
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "[CXX] $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "[CC]  $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.S
	@mkdir -p $(dir $@)
	@echo "[AS]  $<"
	$(AS) $(ASFLAGS) -c $< -o $@

# Include GCC-generated header dependency files (-MMD -MP; C/C++ only, .S has none).
# The leading dash silently skips missing files (first build, after make clean).
-include $(CXX_OBJ:.o=.d)
-include $(C_OBJ:.o=.d)

# ------------------------------------------------------------------------------
# Flashing (UPDI via avrdude's serialupdi programmer)
# ------------------------------------------------------------------------------
flash: $(HEX)
	@echo "Flashing $(TARGET) via UPDI..."
	$(AVRDUDE) -c $(PROG_ID) -p $(MCU) -P $(PROG_PORT) -U flash:w:$<:i

# Fuse row comes from avrducore/fuses.c's FUSES struct, linked into the same
# image and living in $(HEX)'s (unstripped) .fuse section — avrdude 8.x
# recognises the multi-memory hex and writes only the fuse segment.
# NOT run as part of `all`/`flash` — fuses are a separate, deliberate step.
flash_fuses: $(HEX)
	@echo "Flashing fuse row via UPDI..."
	$(AVRDUDE) -c $(PROG_ID) -p $(MCU) -P $(PROG_PORT) -U fuses:w:$<:i

clean:
	@echo "Removing build artefacts..."
	rm -rf $(BUILD)
	@echo "Done."

# ------------------------------------------------------------------------------
# Dev tooling
# ------------------------------------------------------------------------------

# Regenerate compile_commands.json for clangd (Kate's LSP client). -k keeps
# `make` going past the first compile error so bear still records an
# invocation for every source file, not just the ones before the first
# Arduino.h failure. See .clangd for the flag substitutions clangd needs.
bear:
	bear -- $(MAKE) -k all

FORMAT_SRC = $(wildcard *.cpp) $(wildcard *.h) $(wildcard avrducore/*.c) $(wildcard avrducore/*.h)

# Uses .astylerc (run-in braces, 2-space indent).
format:
	astyle --project --suffix=none $(FORMAT_SRC)

help:
	@echo "MiyoCalc40 firmware — make targets"
	@echo ""
	@echo "Build:"
	@echo "  make              - build $(TARGET).hex ($(OPT))"
	@echo "  make size         - build and print avr-size summary"
	@echo "  make debug        - -O0 -g build under $(BUILD)/debug/, for readable disassembly"
	@echo "  make clean        - remove $(BUILD)/"
	@echo ""
	@echo "Flash:"
	@echo "  make flash        - write flash via UPDI ($(PROG_ID) on $(PROG_PORT))"
	@echo "  make flash_fuses  - write the fuse row (avrducore/fuses.c) via UPDI"
	@echo ""
	@echo "Dev:"
	@echo "  make bear         - regenerate compile_commands.json for clangd"
	@echo "  make format       - astyle the tree per .astylerc (see note above)"
	@echo ""
	@echo "Configuration: MCU=$(MCU)  F_CPU=$(F_CPU)  CXX=$(CXX)  OPT=$(OPT)  $(PROG_ID) on $(PROG_PORT)  BUILD=$(BUILD)"
