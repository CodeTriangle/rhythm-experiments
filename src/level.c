#include "../include/constants.h"
#include "../include/level.h"
#include <stdlib.h>

void init_default_level_definition(LevelDefinition *target) {
	target->bpm = BPM;
	target->beatProximity = BEAT_PROX;
	target->soundAnticipation = ANTICIPATION;
	target->drawing = 0;
	target->playMusic = 0;
	target->musPath = "assets/music.mp3";
	target->musDelay = 0;
	target->playMet = 0;
	target->metPath = "assets/metronome.wav";
	target->pattern = "1";
	target->patternLength = 1;
	target->historyLength = HISTORY_LENGTH;
}

void calculate_level_attributes(LevelDefinition *def, LevelAttributes *att) {
	att->beatdelay = 1.0 / def->bpm * 60;
	att->beatdelayms = att->beatdelay * 1000;
}

void init_default_level_progress(LevelDefinition *def, LevelProgress *prog) {
	prog->beatNum = 0;
	prog->advanceBeat = true;
	prog->history = malloc(sizeof(Beat) * def->historyLength);
}
