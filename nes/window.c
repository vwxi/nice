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
			// todo
			break;
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