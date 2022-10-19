#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define L(m,...) printf("%3d, %3d, %3d: " m "\n", nes.ppu.frames + 1, nes.ppu.scanline, nes.ppu.dot, __VA_ARGS__);
#undef L
#define L(m,...) do {}while(0)

#define B(c) if(c) __asm{int 3}
#define W(l,h,a) if(addr >= l && addr <= h) { a; };
#define R(l,h,a) if(addr >= l && addr <= h) { return a; };

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;
typedef char s8;
typedef short s16;
typedef long s32;
typedef long long s64;