#include "window.h"

int window_init(struct window* w)
{
	if (SDL_Init(SDL_INIT_EVERYTHING))
		return -1;

	w->window = SDL_CreateWindow(
		"nice",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		512,
		480,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (!w->window) return -2;

	w->renderer = SDL_CreateRenderer(
		w->window,
		-1,
		SDL_RENDERER_ACCELERATED
	);

	if (!w->renderer) return -3;

	w->texture = SDL_CreateTexture(
		w->renderer,
		SDL_PIXELFORMAT_RGB888,
		SDL_TEXTUREACCESS_STREAMING,
		256,
		240
	);

	if (!w->texture) return -4;

	w->quit = 0;
	return 0;
}

void window_destroy(struct window* w)
{
	if (SDL_WasInit(SDL_INIT_EVERYTHING) != 0) {
		SDL_DestroyTexture(w->texture);
		SDL_DestroyRenderer(w->renderer);
		SDL_DestroyWindow(w->window);
		SDL_Quit();
	}
}

void window_run(struct window* w)
{
	while (!w->quit) {
		while (nes.cpu.cycles < 29781) {
			cpu_step(&nes.cpu);
		}

		nes.cpu.cycles -= 29781;

		SDL_PollEvent(&w->event);
		switch (w->event.type) {
		case SDL_QUIT: w->quit = 1; break;
		case SDL_KEYDOWN: case SDL_KEYUP: 
			window_kbd(w, w->event.type, w->event.key.keysym.sym);
			break;
		}
	}
}

void window_kbd(struct window* w, Uint32 t, SDL_Keycode c)
{
	if (t == SDL_KEYDOWN) {
		switch (c) {
		case SDLK_UP: nes.joyp |= K_UP; break; // up
		case SDLK_DOWN: nes.joyp |= K_DOWN; break; // down
		case SDLK_LEFT: nes.joyp |= K_LEFT; break; // left
		case SDLK_RIGHT: nes.joyp |= K_RIGHT; break; // right
		case SDLK_a: nes.joyp |= K_B; break; // B
		case SDLK_s: nes.joyp |= K_A; break; // A
		case SDLK_LSHIFT: nes.joyp |= K_SEL; break; // select
		case SDLK_RETURN: nes.joyp |= K_STT; break; // start
		}
	}
	else if (t == SDL_KEYUP) {
		switch (c) {
		case SDLK_UP: nes.joyp &= ~K_UP; break; // up
		case SDLK_DOWN: nes.joyp &= ~K_DOWN; break; // down
		case SDLK_LEFT: nes.joyp &= ~K_LEFT; break; // left
		case SDLK_RIGHT: nes.joyp &= ~K_RIGHT; break; // right
		case SDLK_a: nes.joyp &= ~K_B; break; // B
		case SDLK_s: nes.joyp &= ~K_A; break; // A
		case SDLK_LSHIFT: nes.joyp &= ~K_SEL; break; // select
		case SDLK_RETURN: nes.joyp &= ~K_STT; break; // start
		}
	}
}

void window_draw(struct window* w)
{
	SDL_RenderClear(w->renderer);
	SDL_UpdateTexture(w->texture, NULL, nes.ppu.pixels, 256 * sizeof(u32));
	SDL_RenderCopy(w->renderer, w->texture, NULL, NULL);
	SDL_RenderPresent(w->renderer);
}