#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include "../include/constants.h"

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("argument: %s <bpm> <pre/post flash> <anticipation for sound>", argv[0]);
        return 1;
    }

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

    Mix_Chunk *met;
    if (!(met = Mix_LoadWAV("assets/metronome.wav"))) {
        SDL_Log("Failed to load sound effect: %s\n", Mix_GetError());
    }

    SDL_SetRenderTarget(renderer, NULL);
    
    Mix_AllocateChannels(16);

    float bpm = atof(argv[1]);
    int beatProximity = atoi(argv[2]);
    float beatdelay = 1.0 / bpm * 60;
    float beatdelayms = beatdelay * 1000;

    int isBeat = 0;

    uint32_t previousBeat = 0.0, nextBeat = beatdelayms;
    uint32_t sincePrevBeat, tillNextBeat;

    int soundAnticipation = atoi(argv[3]);
    uint32_t nextSound = nextBeat - soundAnticipation;

    uint32_t currentTime;

    int hasKeypress = 0;
    SDL_KeyboardEvent kbEvent;

    int16_t keypresses[HISTORY_LENGTH];
    size_t i, end = 0;

    uint32_t prevToKeypress, nextToKeypress;
    int drawing = 0;

    int x, y, lastX, lastY;

    int loop = 1;

    while (loop) {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        loop = 0;
                        break;
                    case SDLK_SPACE:
                        drawing = !drawing;
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

        currentTime = SDL_GetTicks();

        sincePrevBeat = currentTime - previousBeat;
        tillNextBeat  = nextBeat - currentTime;
        isBeat = abs(sincePrevBeat) <= beatProximity || abs(tillNextBeat) <= beatProximity;

        if (sincePrevBeat >= beatdelayms) {
            previousBeat = previousBeat + beatdelayms;
            nextBeat = previousBeat + beatdelayms;
        }

        if (currentTime >= nextSound) {
            nextSound = nextSound + beatdelayms;
            
            Mix_PlayChannel(-1, met, 0);
        }

        if (hasKeypress) {
            prevToKeypress = DIFF(kbEvent.timestamp, previousBeat);
            nextToKeypress = DIFF(nextBeat, kbEvent.timestamp);

            if (end < HISTORY_LENGTH) {
                keypresses[end++] = prevToKeypress < nextToKeypress ?
                    prevToKeypress : -nextToKeypress;
            }

            printf(
                "keypress number %d at time %5d: %5dms from last beat, %5dms from next beat\n",
                end,
                kbEvent.timestamp,
                prevToKeypress,
                nextToKeypress
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

        SDL_RenderDrawLine(renderer, 0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT / 2);

        for (i = 0; i < end; i++) {
            if (!drawing) {
                break;
            }

            x = WINDOW_WIDTH - WINDOW_WIDTH * (end - i - 1) / (HISTORY_LENGTH - 1);
            y = WINDOW_HEIGHT / 2 + keypresses[i] / beatdelayms * WINDOW_HEIGHT / 2;

            if (i == 0) {
                SDL_RenderDrawPoint(renderer, x, y);
            } else {
                SDL_RenderDrawLine(renderer, lastX, lastY, x, y);
            }

            lastX = x;
            lastY = y;
        }

        SDL_RenderPresent(renderer);
    }

    Mix_FreeChunk(met);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Mix_Quit();
    SDL_Quit();

    return 0;
}
