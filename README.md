# an NES emulator

a work-in-progress NES emulator written for Windows

## usage

`nes.exe [ROM file]`

## features

- cycle accurate CPU and PPU emulation
- support for NROM games (tbd)

## keys

```
-----------------------+
|                +---+ |
|   ↑        +---| a | |
| ← . →      | s |---+ |
|   ↓        +---+     |
|    lshift    enter   |
-----------------------+
```

(arrows are arrow keys)

## tests passed

- nestest
- instr_test-v5
- ppu_vbl_nmi & vbl_nmi_timing
- ppu_open_bus
- scanline.nes
- sprite_overflow_tests
- oam_read
- oam_stress
- sprite_hit_tests_2005.10.05
- blargg_ppu_tests_2005.09.15b
