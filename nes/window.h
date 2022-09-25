#pragma once

#include "def.h"
#pragma warning(push, 0)
#include <SDL.h>
#undef main
#pragma warning(pop)

//#include <imgui_impl_sdl.h>
//#include <imgui_impl_sdlrenderer.h>

struct window {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Event event;
	int quit;

};

int window_init(struct window*);
void window_destroy(struct window*);
void window_run(struct window*);
void window_draw(struct window*);

#include "nes.h"