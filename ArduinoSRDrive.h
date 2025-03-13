#include "stdbool.h"
#include <TimerOne.h>

#define polePairs 4.0f

uint8_t rotorState = 1;

enum SequenceType {
	SEQUENCE_SIMPLE,
	SEQUENCE_OVERLAP
};

typedef struct {
	// pins to set
	uint8_t pin1;
	uint8_t pin2;
	uint8_t pin3;
  uint8_t pin4;
  uint8_t pin5;
  uint8_t pin6;

	// programmed phase
	uint8_t phase1;
	uint8_t phase2;
	uint8_t phase3;
	uint8_t phase4;
	uint8_t phase5;
	uint8_t phase6;
	uint8_t limit;
	uint8_t steps;

	// current positing in chase
	uint8_t position;
	bool running;
  bool reverse;

	// frequency control
	bool speedControl;
	uint32_t timeStep;
	uint32_t nextUpdate;  // timestamp
} Sequence;

Sequence *sequence;
uint16_t timerSerial = 0;
uint16_t timerStartup = 0;
uint32_t timeBetweenISR;
bool rotorInReverse;
uint32_t periode;
int16_t rotorSpeedRpmDesired;
float rotorSpeedRpm;