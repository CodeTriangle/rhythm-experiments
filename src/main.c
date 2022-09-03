// Copyright 2021 CodeTriangle (Richard Snider)
// 
// BSD 2-clause License
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//	   (1) Redistributions of source code must retain the above copyright
//	   notice, this list of conditions and the following disclaimer. 
// 
//	   (2) Redistributions in binary form must reproduce the above
//	   copyright notice, this list of conditions and the following
//	   disclaimer in the documentation and/or other materials provided with
//	   the distribution.  
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE. 

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include "../include/constants.h"
#include "../include/level.h"

int main(int argc, char **argv) {
	LevelContext context;

	init_default_level_definition(&context.def);

	int i;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			char *option = argv[i]+1;
			if (strcmp(option, "bpm") == 0) {
				context.def.bpm = atof(argv[++i]);
			} else if (strcmp(option, "prox") == 0) {
				context.def.beatProximity = atoi(argv[++i]);
			} else if (strcmp(option, "draw") == 0) {
				context.def.drawing = 1;
			} else if (strcmp(option, "mus") == 0) {
				context.def.playMusic = 1;
			} else if (strcmp(option, "musp") == 0) {
				context.def.musPath = argv[++i];
			} else if (strcmp(option, "musd") == 0) {
				context.def.musDelay = atoi(argv[++i]);
			} else if (strcmp(option, "met") == 0) {
				context.def.playMet = 1;
			} else if (strcmp(option, "metp") == 0) {
				context.def.metPath = argv[++i];
			} else if (strcmp(option, "metant") == 0) {
				context.def.soundAnticipation = atoi(argv[++i]);
			} else if (strcmp(option, "pat") == 0) {
				context.def.pattern = argv[++i];
				context.def.patternLength = strlen(context.def.pattern);
			} else {
				printf("Unexpected argument (%d): %s\n", i, option);
			}
		} else {
			printf("Unexpected bare argument (%d): %s\n", i, argv[i]);
		}
	}

	calculate_level_attributes(&context.def, &context.att);
	init_default_level_progress(&context.def, &context.prog);

	SDL_Window *window;
	SDL_Renderer *renderer;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
		SDL_Log("Unable to initialize SDL2: %s\n", SDL_GetError());
		return 1;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
		SDL_Log("Unable to initialize SDL_mixer: %s\n", Mix_GetError());
		return 1;
	}

	window = SDL_CreateWindow(
		WINDOW_TITLE,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_RESIZABLE
	);

	if (window == NULL) {
		SDL_Log("Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED |
		SDL_RENDERER_PRESENTVSYNC |
		SDL_RENDERER_TARGETTEXTURE
	);

	if (renderer == NULL) {
		SDL_Log("Could not create renderer: %s\n", SDL_GetError());
		return 1;
	}

	Mix_Chunk *met = NULL;
	if (context.def.playMet && !(met = Mix_LoadWAV(context.def.metPath))) {
		SDL_Log("Failed to load sound effect: %s\n", Mix_GetError());
	}

	Mix_Music *mus = NULL;
	if (context.def.playMusic && !(mus = Mix_LoadMUS(context.def.musPath))) {
		SDL_Log("Failed to load music: %s\n", Mix_GetError());
	}

	SDL_SetRenderTarget(renderer, NULL);
	
	Mix_AllocateChannels(16);

	int screenW = WINDOW_WIDTH;
	int screenH = WINDOW_HEIGHT;

	int hasKeypress = 0;
	SDL_KeyboardEvent kbEvent;

	Beat keypresses[HISTORY_LENGTH] = {0};

	int x, y, lastX, lastY;

	bool loop = true;

	context.prog.previousBeat = SDL_GetTicks();
	context.prog.nextBeat = context.prog.previousBeat + context.att.beatdelayms;
	context.prog.nextSound = context.prog.nextBeat - context.def.soundAnticipation;

	while (loop) {
		if (context.def.playMusic && !Mix_PlayingMusic() && context.prog.currentTime >= context.def.musDelay) {
			Mix_PlayMusic(mus, -1);
		}

		SDL_Event e;

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_ESCAPE:
					loop = 0;
					break;
				case SDLK_SPACE:
					context.def.drawing = !context.def.drawing;
					break;
				default:
					hasKeypress = 1;
					kbEvent = e.key;
					break;
				}
			}
			else if (e.type == SDL_QUIT) {
				loop = 0;
			}
		}

		SDL_GetWindowSize(window, &screenW, &screenH);

		context.prog.currentTime = SDL_GetTicks();

		uint32_t sincePrevBeat = context.prog.currentTime - context.prog.previousBeat;
		uint32_t tillNextBeat  = context.prog.nextBeat - context.prog.currentTime;
		bool isBeat = sincePrevBeat <= context.def.beatProximity || tillNextBeat <= context.def.beatProximity;

		if (sincePrevBeat >= context.att.beatdelayms) {
			context.prog.previousBeat = context.prog.nextBeat;
			context.prog.nextBeat = context.prog.previousBeat + context.att.beatdelayms;

			if (context.prog.beatNum >= HISTORY_LENGTH) {
				context.prog.advanceBeat = 0;
			}

			sincePrevBeat = context.prog.currentTime - context.prog.previousBeat;
			tillNextBeat  = context.prog.nextBeat - context.prog.currentTime;

			if (context.prog.advanceBeat) context.prog.beatNum++;
		}

		bool shouldPlayBeat = true;

		if (context.prog.advanceBeat) {
			int patternNum = context.prog.beatNum;

			if (sincePrevBeat < tillNextBeat) {
				patternNum -= 1;
			}

			patternNum %= context.def.patternLength;

			if (patternNum < 0) {
				patternNum = 0;
			}

			if (0 == context.def.pattern[patternNum]) {
				shouldPlayBeat = 0;
			}
		}

		if (context.def.playMet && context.prog.currentTime >= context.prog.nextSound) {
			context.prog.nextSound = context.prog.nextSound + context.att.beatdelayms;
			
			if (shouldPlayBeat) {
				Mix_PlayChannel(-1, met, 0);
			}
		}

		if (hasKeypress) {
			if (!context.prog.advanceBeat && context.prog.beatNum == 0) context.prog.advanceBeat = 1;

			int prevToKeypress = DIFF(kbEvent.timestamp, context.prog.previousBeat);
			int nextToKeypress = DIFF(context.prog.nextBeat, kbEvent.timestamp);

			if (prevToKeypress < nextToKeypress) {
				keypresses[context.prog.beatNum].pressed = 1;
				keypresses[context.prog.beatNum].value = prevToKeypress;
			} else if (context.prog.beatNum + 1 < HISTORY_LENGTH) {
				int index = context.prog.beatNum == 0 ? 0 : context.prog.beatNum + 1;
				keypresses[index].pressed = 1;
				keypresses[index].value = -nextToKeypress;
			}

			printf(
				"beat number %3d at time %5d: %5dms from last beat, %5dms from next beat\n",
				context.prog.beatNum,
				kbEvent.timestamp,
				prevToKeypress,
				nextToKeypress
			);

			hasKeypress = 0;
		}

		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);

		if (isBeat && shouldPlayBeat) {
			SDL_SetRenderDrawColor(renderer, 0, 30, 120, 255);
		} else {
			SDL_SetRenderDrawColor(renderer, 20, 90, 120, 255);
		}

		SDL_RenderClear(renderer);

		SDL_Rect mouseRect = {
			.x = mouseX - 8,
			.y = mouseY - 8,
			.w = 16,
			.h = 16
		};

		SDL_SetRenderDrawColor(renderer, 250, 250, 200, 255);

		SDL_RenderFillRect(renderer, &mouseRect);

		SDL_RenderDrawLine(renderer, 0, screenH/2, screenW, screenH/2);

		for (i = 0; i < context.prog.beatNum; i++) {
			if (!context.def.drawing) {
				break;
			}

			if (!keypresses[i].pressed) {
				continue;
			}

			x = screenW - screenW * (context.prog.beatNum - i - 1) / (HISTORY_LENGTH - 1);
			y = (float) screenH / 2 + keypresses[i].value / context.att.beatdelayms * screenH;

			if (i != 0) {
				SDL_RenderDrawLine(renderer, lastX, lastY, x, y);
			}

			lastX = x;
			lastY = y;
		}

		SDL_RenderPresent(renderer);
	}

	if (met) Mix_FreeChunk(met);
	if (mus) Mix_FreeMusic(mus);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	Mix_Quit();
	SDL_Quit();

	return 0;
}
