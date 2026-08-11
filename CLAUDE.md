# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

MiyoCalc40 is a through-hole-only RPN calculator: a hardware + firmware project, not a typical software repo. The repository has two independent halves:

- `MiyoCalc40_firmware/` — C/C++ firmware for the calculator's AVR128DA28 MCU, built as an Arduino sketch.
- `pcb/` — KiCad hardware design (schematic, PCB layout, footprints, BOM).

There is no shared build system between the two; treat them as separate projects sharing a repo.

## Firmware: build & flash

The firmware is an Arduino sketch, not built from the command line in this repo:

- Toolchain: Arduino IDE + the **DxCore** board package (targets Microchip AVR-Dx parts).
- Target MCU: **AVR128DA28** at 24 MHz.
- To build/flash: open `MiyoCalc40_firmware/MiyoCalc40_firmware.ino` in the Arduino IDE, select the AVR128DA28 board (DxCore), and flash via a UPDI programmer (not a bootloader/USB upload).
- There is no unit test suite, linter, or CI for the firmware — verification is done on real hardware.

### Compiler / `double` width

DxCore's `platform.txt` has a deprecated `compiler.path` that Arduino IDE silently
redirects to the **system** avr-gcc (`/usr/bin/avr-gcc`), not the bundled
`DxCore/tools/avr-gcc/7.3.0-...`. Check which one actually ran in the verbose build log's
`Warning: platform.txt from core 'DxCore' contains deprecated compiler.path=...` line.

`double` is 32-bit (`== float`, avr-gcc's default `-mdouble=32`) on this toolchain, giving
~7.2 honest decimal digits of precision. `precision` in `calc.cpp`'s `calc_init()` is set
to `7` to match, deliberately staying inside that budget.

**Do not force `-mdouble=64` via a DxCore `platform.local.txt`.** This was tried and
reverted: the build compiles and links cleanly with no errors (`__adddf3`/`__muldf3`/
`__divdf3` all resolve fine — those come from libgcc, not avr-libc), but avr-libc 2.2.1's
`log10()`/`floor()`/`round()` for 64-bit doubles resolve to a software "libf7"
extended-precision library (`__f7_log10`, `__f7_floor`, `__f7_round`) that produced
garbage results on real AVR128DA28 hardware — e.g. `LCDNumber(3.0)` computed
`_exponent` around `-22003` instead of `0`. This is invisible to linkage checks and to
host-side testing (the host's own `double` math is correct; only the AVR target's is
broken), so it can only be caught by running on real hardware. DxCore's own bundled
compiler (7.3.0) doesn't even offer `-mdouble=64` as an option — this was only reachable
because Arduino IDE's `compiler.path` redirect silently substitutes a much newer,
untested system compiler. If 64-bit precision is revisited, verify `log10`/`pow`/`floor`/
`round` numerically **on real hardware** (e.g. print results over UART) before trusting a
clean build.

## Firmware architecture

The module breakdown and dependency order is documented in the header comment of `MiyoCalc40_firmware.ino` and `main.h`:

- **main.cpp/h** — `main()`, `setup()`/`loop()`, MCU/ADC/sleep setup, port ISRs. `loop()` is the core state machine: sleep the CPU, wake on a column-pin interrupt, debounce (10×2ms), scan the matrix, then dispatch the pressed key.
- **matrix.cpp/h** — keyboard matrix scanning (`scanKB()`), exposes the currently-pressed `keypos_r`/`keypos_c`.
- **cards.cpp/h** — maps matrix key positions to `action` structs (keycode + function pointer + display mnemonic). A "card" is a swappable keymap (e.g. algebra, trig, finance) with a base layer plus `f`/`g`/`h` shift layers (see `enum layers`). `keytoaction()` is the lookup entry point called from `main.cpp`'s loop.
- **calc.cpp/h** — the RPN number-crunching engine: stack entry/manipulation (`enter_number`, `enter_enter`, `enter_swap_xy`, `enter_rotate`, ...), 1-arg/2-arg function application (`apply_func_1to1`, `apply_func_2to1`), memory registers (`apply_memory_rcl/sto/clr`), shift-layer state, and LCD stack/input rendering (`LCDDrawStackAndMem`, `LCDDrawInput`, `LCDDrawCalcStatus`).
- **lcd.cpp/h** + **fonts.h** — driver for the ERM19264_UC1609 LCD (SPI), adapted from Gavin Lyons' ERM19264_UC1609 library. `fonts.h` holds the bitmap font table used for on-screen digits/letters/mnemonics.
- **programming.cpp/h** — "program mode" (record/replay keystrokes with control flow: IF/ELSE, CASE/WHEN, FOR/NEXT, WHILE/WEND, DO/LOOPWH). Currently **design notes only** in `programming.h`'s trailing comment block — not yet implemented (`programming.cpp` is a 1-line stub).
- **config.cpp/h** — "config mode" (battery voltage display, contrast, backlight, stack depth/mode, card switching, program import/export, debounce tuning). Also currently **design notes only** in comments — not yet implemented.
- **backlight.cpp/h** — PWM backlight on/off and duty cycle (TCA0 on `PIN_PA1`).
- **util.cpp/h** — small helpers (currently minimal/stub).

### Key architectural concepts

- **`action` struct** (`cards.h`): `{keycode, fct (function pointer), mnemonic[5]}`. `mnemonic` indexes into the font table for displaying the action's label in programming mode.
- **Modes**: `enum calc_prog_config_modes { calc_mode, prog_mode, config_mode }` — the calculator's top-level mode.
- **Shift layers**: `enum layers { baseLayer, fLayer, gLayer, hLayer }`, tracked in the global `shift` variable; a shift layer is exited back to `baseLayer` after any non-shift key is processed (see the shift-reset logic at the end of `loop()` in `main.cpp`).
- **Memory modes**: `mem_recall_mode` / `mem_store_mode` / `mem_clear_mode` booleans short-circuit normal action dispatch in `loop()` to instead call `apply_memory_rcl/sto/clr(r, c)` directly with the raw key position.
- Many design decisions are written as pseudocode/comments directly in the header files (`main.h`, `programming.h`, `config.h`, `cards.cpp`) before being implemented — read these comments first when extending a mode, they usually already describe the intended control flow.
- Power management is central to the design: the MCU sleeps between keypresses (`SLEEP_MODE_STANDBY`), wakes via port interrupts on column pins, and a full LCD/MCU power-down path exists (`power_down()` in `main.cpp`, `SLEEP_MODE_PWR_DOWN`).

## PCB (KiCad)

- `pcb/pcbcalc/` — the actual MiyoCalc40 calculator board (`miyocalc40.kicad_pro`/`.kicad_sch`/`.kicad_pcb`).
- `pcb/pcbbase/` — a base/reference board project.
- `pcb/lib_fp.pretty/` — project-specific KiCad footprints.
- `pcb/lib_sch/` — project-specific KiCad schematic symbol library.
- `pcb/bom/` — bill of materials.
- `pcb/printouts/` — rendered schematic/PCB/3D screenshots.

## Licensing

Two different licenses apply within this repo:

- **PCB work** (`pcb/`): CC BY-NC-SA 4.0.
- **Firmware** (`MiyoCalc40_firmware/`): GNU GPLv3.

Keep this split in mind when adding files or copying in third-party code (e.g. the LCD driver is adapted from an existing GPL/MIT-licensed project — check upstream license compatibility before pulling in more external code).
