#ifndef RHYTHM_EXP_GRAPHICS_H
#define RHYTHM_EXP_GRAPHICS_H

#include <stdbool.h>

#include <SDL.h>

#include <pthread.h>

#include "level.h"

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;
	pthread_mutex_t lock;
} GraphicsContext;

typedef struct {
	LevelContext level;
	GraphicsContext graphics;
} GraphicsArgs;

void *graphics_work_function(void *arg);

void update_graphics(GraphicsArgs *arg);

#endif
