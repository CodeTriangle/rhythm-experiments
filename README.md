# Project State as of commit ``9685c4d``

As of right now, this is a very simple rhythm test program built on SDL.
I've done quite a bit of tweaking to make this a solid baseline to build
more complicated rhythm games on.

To build this program all you need do is run `make` in the root
directory of the project. The binary will be created as `target/game`.

As of now, the program accepts different command-line arguments:
* `-bpm <f32>`: the number of beats per minute the game should play at.
* `-prox <i32>`: the number of milliseconds before and after the beat that
   you'd like the screen to flash to indicate a beat.
* `-ant <i32>`: `SDL_mixer` runs audio with a slight delay. This
  argument controls how many milliseconds before the beat the program
  should queue the sound to play (in testing my own rhythm, anywhere
  from 15ms to 25ms has made me more accurate).
* `-draw`: whether or not to draw a graph of your progress.

While you're in the game itself, `Escape` will cause the program to
exit, `Space` will toggle a graph of how close you were to the correct
beat, and every other key will count as a beat in the test. There is
currently no way to detect missed beats.
