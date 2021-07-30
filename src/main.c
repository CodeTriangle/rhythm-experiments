#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <SDL.h>

#include "../include/constants.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("need 2 argument");
        return 1;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL2: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
            WINDOW_TITLE,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            0
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

    SDL_SetRenderTarget(renderer, NULL);

    float bpm = atof(argv[1]);
    int beatProximity = atoi(argv[2]);
    float beatdelay = 1.0 / bpm * 60;
    float beatdelayms = beatdelay * 1000;

    int isBeat = 0;

    unsigned int previousBeat = 0.0, nextBeat = beatdelayms;
    unsigned int sincePrevBeat, tillNextBeat;

    unsigned int currentTime;

    int hasKeypress = 0;
    SDL_KeyboardEvent kbEvent;

    int loop = 1;

    while (loop) {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN) {
                hasKeypress = 1;
                kbEvent = e.key;
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    loop = 0;
                }
            }
            else if (e.type == SDL_QUIT) {
                loop = 0;
            }
        }

        currentTime = SDL_GetTicks();

        sincePrevBeat = currentTime - previousBeat;
        tillNextBeat  = nextBeat - currentTime;
        isBeat = abs(sincePrevBeat) <= beatProximity || abs(tillNextBeat) <= beatProximity;

        if (sincePrevBeat >= beatdelayms) {
            previousBeat = previousBeat + beatdelayms;
            nextBeat = previousBeat + beatdelayms;
        }

        if (hasKeypress) {
            printf(
                "keypress at time %5d: %5dms from last beat, %5dms from next beat\n",
                kbEvent.timestamp,
                kbEvent.timestamp - previousBeat,
                nextBeat - kbEvent.timestamp
            );

            hasKeypress = 0;
        }

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        if (isBeat) {
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

        SDL_RenderPresent(renderer);
    }

    SDL_Quit();

    return 0;
}
