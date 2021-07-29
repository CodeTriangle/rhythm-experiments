#include <stdlib.h>
#include <sys/time.h>
#include <SDL.h>

#include "../include/constants.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("need two arguments");
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
            SDL_RENDERER_TARGETTEXTURE
    );

    if (renderer == NULL) {
        SDL_Log("Could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetRenderTarget(renderer, NULL);

    float bpm = atof(argv[2]);
    int beatProximity = 40;
    float beatdelay = 1.0 / bpm * 60;
    float beatdelayms = beatdelay * 1000;

    int isBeat = 0;

    int fps = atof(argv[1]);
    int framedelay = 1.0 / fps;
    int framedelayms = framedelay * 1000;

    float previousBeat = 0.0, nextBeat = beatdelayms;
    float sincePrevBeat, tillNextBeat;

    float previousFrame = 0.0, nextFrame = framedelayms;
    float currentTime;

    int loop = 1;

    while (loop) {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    loop = 0;
                }
            }
            else if (e.type == SDL_QUIT) {
                loop = 0;
            }
        }

        currentTime = (float) SDL_GetTicks();

        printf("%10.3f %10.3f %10.3f %10.3f\n", previousBeat, nextBeat, previousFrame, nextFrame);

        sincePrevBeat = currentTime - previousBeat;
        tillNextBeat  = nextBeat - currentTime;
        if (abs(sincePrevBeat) <= beatProximity || abs(tillNextBeat) <= beatProximity) {
            isBeat = 1;
        }

        if (sincePrevBeat >= beatdelayms) {
            previousBeat = previousBeat + beatdelayms;
            nextBeat = previousBeat + beatdelayms;
        }

        if (currentTime - previousFrame >= framedelayms) {
            previousFrame = currentTime;
            nextFrame = previousFrame + framedelayms;

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

            //if (isBeat) {
            //    SDL_SetRenderDrawColor(renderer, 200, 10, 10, 255);
            //} else {
                SDL_SetRenderDrawColor(renderer, 250, 250, 200, 255);
            //}

            SDL_RenderFillRect(renderer, &mouseRect);

            SDL_RenderPresent(renderer);

            isBeat = 0;
        }
            
        SDL_Delay(5);
    }

    return 0;
}
