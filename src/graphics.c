#include <stdlib.h>

#include "../include/graphics.h"

void *graphics_work_function(void *arg) {
	GraphicsArgs *gArg = arg;

	while (true) {
		update_graphics(gArg);
	}
	return NULL;
}

void update_graphics(GraphicsArgs *arg) {
	GraphicsContext *graphics = &arg->graphics;
	LevelContext *level = &arg->level;

	int screenW, screenH;

	SDL_GetWindowSize(graphics->window, &screenW, &screenH);

	SDL_RenderClear(graphics->renderer);

	SDL_SetRenderDrawColor(graphics->renderer, 250, 250, 200, 255);

	SDL_RenderDrawLine(graphics->renderer, 0, screenH/2, screenW, screenH/2);

	int lastX, lastY;
	for (int i = 0; i < level->prog.beatNum; i++) {
		if (!level->def.drawing) {
			break;
		}

		if (!level->prog.history[i].pressed) {
			continue;
		}

		int x = screenW - screenW * (level->prog.beatNum - i - 1) / (level->def.historyLength - 1);
		int y = (float) screenH / 2 + level->prog.history[i].value / level->att.beatdelayms * screenH;

		if (i != 0) {
			SDL_RenderDrawLine(graphics->renderer, lastX, lastY, x, y);
		}

		lastX = x;
		lastY = y;
	}

	SDL_RenderPresent(graphics->renderer);
}
