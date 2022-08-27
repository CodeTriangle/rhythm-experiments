// Copyright 2021 CodeTriangle (Richard Snider)
// 
// BSD 2-clause License
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     (1) Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer. 
// 
//     (2) Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided with
//     the distribution.  
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

typedef struct {
    unsigned int pressed: 1;
    signed int value: 15;
} Beat;

int main(int argc, char **argv) {
    float bpm = BPM;
    int beatProximity = BEAT_PROX;
    int soundAnticipation = ANTICIPATION;
    int drawing = 0;
    int playMusic = 0;
    char *musPath = "assets/music.mp3";
    int musDelay = 0;
    int playMet = 0;
    char *metPath = "assets/metronome.wav";
    char *pattern = "1";
    int patternLength = 1;

    int i;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            char *option = argv[i]+1;
            if (strcmp(option, "bpm") == 0) {
                bpm = atof(argv[++i]);
            } else if (strcmp(option, "prox") == 0) {
                beatProximity = atoi(argv[++i]);
            } else if (strcmp(option, "draw") == 0) {
                drawing = 1;
            } else if (strcmp(option, "mus") == 0) {
                playMusic = 1;
            } else if (strcmp(option, "musp") == 0) {
                musPath = argv[++i];
            } else if (strcmp(option, "musd") == 0) {
                musDelay = atoi(argv[++i]);
            } else if (strcmp(option, "met") == 0) {
                playMet = 1;
            } else if (strcmp(option, "metp") == 0) {
                metPath = argv[++i];
            } else if (strcmp(option, "metant") == 0) {
                soundAnticipation = atoi(argv[++i]);
            } else if (strcmp(option, "pat") == 0) {
                pattern = argv[++i];
                patternLength = strlen(pattern);
            } else {
                printf("Unexpected argument (%d): %s\n", i, option);
            }
        } else {
            printf("Unexpected bare argument (%d): %s\n", i, argv[i]);
        }
    }

    printf("%d\n\n", patternLength);

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
    if (playMet && !(met = Mix_LoadWAV(metPath))) {
        SDL_Log("Failed to load sound effect: %s\n", Mix_GetError());
    }

    Mix_Music *mus = NULL;
    if (playMusic && !(mus = Mix_LoadMUS(musPath))) {
        SDL_Log("Failed to load music: %s\n", Mix_GetError());
    }

    SDL_SetRenderTarget(renderer, NULL);
    
    Mix_AllocateChannels(16);

    int screenW = WINDOW_WIDTH;
    int screenH = WINDOW_HEIGHT;

    float beatdelay = 1.0 / bpm * 60;
    float beatdelayms = beatdelay * 1000;

    uint32_t beatNum = 0;
    int advanceBeat = 0;
    int isBeat = 0, shouldPlayBeat = 1;

    uint32_t previousBeat, nextBeat;
    uint32_t sincePrevBeat, tillNextBeat;

    uint32_t nextSound;

    uint32_t currentTime;

    int hasKeypress = 0;
    SDL_KeyboardEvent kbEvent;

    Beat keypresses[HISTORY_LENGTH] = {0};

    uint32_t prevToKeypress, nextToKeypress;

    int x, y, lastX, lastY;

    int loop = 1;

    previousBeat = SDL_GetTicks();
    nextBeat = previousBeat + beatdelayms;
    nextSound = nextBeat - soundAnticipation;

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

        SDL_GetWindowSize(window, &screenW, &screenH);

        currentTime = SDL_GetTicks();

        sincePrevBeat = currentTime - previousBeat;
        tillNextBeat  = nextBeat - currentTime;
        isBeat = sincePrevBeat <= beatProximity || tillNextBeat <= beatProximity;

        if (sincePrevBeat >= beatdelayms) {
            previousBeat = nextBeat;
            nextBeat = previousBeat + beatdelayms;

            if (beatNum >= HISTORY_LENGTH) {
                advanceBeat = 0;
            }

            sincePrevBeat = currentTime - previousBeat;
            tillNextBeat  = nextBeat - currentTime;

            if (advanceBeat) beatNum++;
        }

        if (advanceBeat) {
            int patternNum = beatNum;

            if (sincePrevBeat < tillNextBeat) {
                patternNum -= 1;
            }

            patternNum %= patternLength;

            if (patternNum < 0) {
                patternNum = 0;
            }

            switch (pattern[patternNum]) {
                case '1':
                    shouldPlayBeat = 1;
                    break;
                case '0':
                    shouldPlayBeat = 0;
                    break;
            }
        }

        if (playMet && currentTime >= nextSound) {
            nextSound = nextSound + beatdelayms;
            
            if (shouldPlayBeat) {
                Mix_PlayChannel(-1, met, 0);
            }
        }

        if (playMusic && !Mix_PlayingMusic() && currentTime >= musDelay) {
            Mix_PlayMusic(mus, -1);
        }

        if (hasKeypress) {
            if (!advanceBeat && beatNum == 0) advanceBeat = 1;

            prevToKeypress = DIFF(kbEvent.timestamp, previousBeat);
            nextToKeypress = DIFF(nextBeat, kbEvent.timestamp);

            if (prevToKeypress < nextToKeypress) {
                keypresses[beatNum].pressed = 1;
                keypresses[beatNum].value = prevToKeypress;
            } else if (beatNum + 1 < HISTORY_LENGTH) {
                int index = beatNum == 0 ? 0 : beatNum + 1;
                keypresses[index].pressed = 1;
                keypresses[index].value = -nextToKeypress;
            }

            printf(
                "beat number %3d at time %5d: %5dms from last beat, %5dms from next beat\n",
                beatNum,
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

        for (i = 0; i < beatNum; i++) {
            if (!drawing) {
                break;
            }

            if (!keypresses[i].pressed) {
                continue;
            }

            x = screenW - screenW * (beatNum - i - 1) / (HISTORY_LENGTH - 1);
            y = (float) screenH / 2 + keypresses[i].value / beatdelayms * screenH;

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
