#ifndef Globals_h
#define Globals_h
#include <arduino.h>

#define STEP_PIN   11
#define DIR_PIN    12
#define STEP_ON    HIGH
#define STEP_OFF   LOW
#define DIR_FWD    LOW
#define DIR_BCK    HIGH

#define DIR_SETUP_TIME   500
#define STEP_PULSE_WIDTH 100

#define MAX_TOOL_POSITIONS  8
#endif
