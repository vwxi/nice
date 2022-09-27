#pragma once

#include "def.h"
#pragma warning(push, 0)
#include <SDL.h>
#undef main
#pragma warning(pop)

#define K_A (1 << 0)
#define K_B (1 << 1)
#define K_SEL (1 << 2)
#define K_STT (1 << 3)
#define K_UP (1 << 4)
#define K_DOWN (1 << 5)
#define K_LEFT (1 << 6)
#define K_RIGHT (1 << 7)

struct window {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Event event;
	int quit;
	u32 ticks;
};

int window_init(struct window*);
void window_destroy(struct window*);
void window_run(struct window*);
void window_draw(struct window*);
void window_kbd(struct window*, u32, SDL_Keycode);

#include "nes.h"