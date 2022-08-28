#ifndef RHYTHM_EXP_LEVEL_H
#define RHYTHM_EXP_LEVEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
	unsigned int pressed: 1;
	signed int value: 15;
} Beat;

typedef struct {
	float bpm;
	int beatProximity;
	int soundAnticipation;
	bool drawing;
	bool playMusic;
	char *musPath;
	int musDelay;
	bool playMet;
	char *metPath;
	char *pattern;
	size_t patternLength;
	size_t historyLength;
} LevelDefinition;

typedef struct {
	float beatdelay;
	float beatdelayms;
} LevelAttributes;

typedef struct {
	uint32_t beatNum;
	bool advanceBeat;
	bool isBeat, shouldPlayBeat;
	uint32_t previousBeat, nextBeat;
	uint32_t sincePrevBeat, tillNextBeat;
	uint32_t nextSound;
	uint32_t currentTime;
	Beat* history;
} LevelProgress;

typedef struct {
	LevelDefinition def;
	LevelAttributes att;
	LevelProgress prog;
} LevelContext;

void init_default_level_definition(LevelDefinition *target);

void calculate_level_attributes(LevelDefinition *def, LevelAttributes *att);

void init_default_level_progress(LevelDefinition *def, LevelProgress *prog);

#endif
