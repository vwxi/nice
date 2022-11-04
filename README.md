# an NES emulator

a work-in-progress NES emulator written for Windows

## screenshots

![image](https://user-images.githubusercontent.com/98568521/193934396-71b05049-3962-4f2e-b497-e31345701b8e.png)
![image](https://user-images.githubusercontent.com/98568521/193934546-eccd8b55-351a-494e-b27f-45e844bac69d.png)
![image](https://user-images.githubusercontent.com/98568521/193934605-9a5e7401-0f49-4616-9ca1-0c5c8dda133d.png)
![image](https://user-images.githubusercontent.com/98568521/193934747-5e5d01ce-0cad-41f4-9ba7-70c9655cb65b.png)

## usage

`nes.exe [ROM file]`

## features

- cycle accurate CPU and PPU emulation
- support for mapper 0, 1, 4* games (tbd)

*rad racer 2 doesn't work

## keys

```
-----------------------+
|                +---+ |
|   ↑        +---| s | |
| ← . →      | a |---+ |
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
- mmc3_test_2 (except 4-scanline_timing (#8) and 6-MMC3_alt (#2))

### ☮️

thank you binji for fixing some bugs
