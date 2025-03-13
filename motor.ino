Sequence *motorInit(char pin1, char pin2, char pin3, char pin4, char pin5, char pin6, enum SequenceType type)
{
  Sequence *result = calloc(1, sizeof(Sequence));

  // store sequence data and size
  switch (type) {
    case SEQUENCE_OVERLAP:
      result->phase1 = 0b11000001;
      result->phase2 = 0b01110000;
      result->phase3 = 0b00011100;
      result->phase4 = 0b00000111;

      result->phase5 = 0b11011101;
      result->phase6 = 0b01110111;

      result->steps = 8;
      break;
    case SEQUENCE_SIMPLE:
    default:
      result->phase1 = 0b00000001;
      result->phase2 = 0b00000010;
      result->phase3 = 0b00000100;
      result->phase4 = 0b00001000;

      result->phase5 = 0b00000101;
      result->phase6 = 0b00001010;

      result->steps = 4;
      break;
  }

	// configure pins
	result->pin1 = pin1;
	result->pin2 = pin2;
	result->pin3 = pin3;
	result->pin4 = pin4;
	result->pin5 = pin5;
	result->pin6 = pin6;

	// set initial position
	result->limit = 1 << result->steps;
	result->position = 1;  // we will use bits in char to access phase state
	result->running = false;
	result->speedControl = false;
	motorSetSpeed(result, 84); // speed in rpm

	// initialize micro-controller pins
	pinMode(pin1, OUTPUT);
	pinMode(pin2, OUTPUT);
	pinMode(pin3, OUTPUT);
	pinMode(pin4, OUTPUT);
	pinMode(pin5, OUTPUT);
	pinMode(pin6, OUTPUT);

	return result;
}

// Release memory taken by the configuration
void motorFree(Sequence *sequence)
{
	free(sequence);
}

// Turn on or off speed control of the motor
void motorSetSpeedControl(Sequence *sequence, bool status)
{
	sequence->speedControl = status;
}

// Set desired number of revolutions per minute of the motor
void motorSetSpeed(Sequence *sequence, int speed)
{
  sequence->reverse = (speed < 0);
	sequence->timeStep = (60000000 / abs(speed * polePairs)) / sequence->steps;
	sequence->nextUpdate = micros() + sequence->timeStep;
}

// Check if sequence cycle should be updated
bool motorShouldUpdate(Sequence *sequence)
{
	bool result;

	// check if we should update cycle
	if (sequence->speedControl) {
		result = micros() >= sequence->nextUpdate;

		if (result) sequence->nextUpdate += sequence->timeStep;

	} else {
		// speed control is off
		result = true;
	}

	return result;
}

// Make a single forward step. Precision (angle of rotation) depends on sequence type
bool motorStep(Sequence *sequence, bool startup)
{
	// check if cycle can be updated
	if (!motorShouldUpdate(sequence))
		return false;

	// change position
  if (sequence->reverse) {
    if (startup) {
      sequence->position >>= 1;
    }else{
      sequence->position = rotorState >> 1;
    }
    if (sequence->position < 1)
      sequence->position = sequence->limit >> 1;
  }else{
    if (startup) {
      sequence->position <<= 1;
    }else{
      sequence->position = rotorState << 1;
    }
    if (sequence->position == sequence->limit)
      sequence->position = 1;
  }

	// set pin states
	motorApply(sequence);

	return true;
}

// Apply voltage to configured pins
void motorApply(Sequence *sequence)
{
	if (!sequence->running)
		return;

	digitalWrite(sequence->pin1, sequence->phase1 & sequence->position ? HIGH : LOW);
	digitalWrite(sequence->pin2, sequence->phase2 & sequence->position ? HIGH : LOW);
  digitalWrite(sequence->pin3, sequence->phase3 & sequence->position ? LOW : HIGH);
	digitalWrite(sequence->pin4, sequence->phase4 & sequence->position ? LOW : HIGH);

	digitalWrite(sequence->pin5, ((sequence->phase1 & sequence->position) | (sequence->phase3 & sequence->position)) ? HIGH : LOW);
	digitalWrite(sequence->pin6, ((sequence->phase2 & sequence->position) | (sequence->phase4 & sequence->position)) ? HIGH : LOW);
}

// This function will stop applying voltage to select pins and motor will be in free rotation
void motorRelease(Sequence *sequence)
{
	sequence->running = false;
	digitalWrite(sequence->pin1, LOW);
	digitalWrite(sequence->pin2, LOW);
	digitalWrite(sequence->pin3, LOW);
	digitalWrite(sequence->pin4, LOW);
	digitalWrite(sequence->pin5, HIGH);
	digitalWrite(sequence->pin6, HIGH);
}

// Resume motor rotation
void motorResume(Sequence *sequence)
{
	sequence->running = true;
}