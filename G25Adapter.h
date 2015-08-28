#include "USBAPI.h"


#define MIN 0
#define MAX 65535
#define CENTER 32768

// Pins
#define CLOCK_PIN          6
#define DATA_IN_PIN        5
#define MODE_PIN           7
#define SHIFTERX_PIN       8
#define SHIFTERY_PIN       9
#define ACCELERATOR_PIN    20
#define BRAKE_PIN          19
#define CLUTCH_PIN         18

// Shift register pins
#define SR_REVERSE 0x02
#define SR_MODE 0x08

// Analog axes
#define SHIFTERX 0
#define SHIFTERY 1
#define ACCELERATOR 2
#define BRAKE 3
#define CLUTCH 4

// Analog deadzone
#define DEADZONE 10

// Buttons
#define BUTTON_RED1 0
#define BUTTON_RED2 1
#define BUTTON_RED3 2
#define BUTTON_RED4 3
#define BUTTON_BLACK_UP 4
#define BUTTON_BLACK_RIGHT 5
#define BUTTON_BLACK_DOWN 6
#define BUTTON_BLACK_LEFT 7
#define BUTTON_DPAD_UP 8
#define BUTTON_DPAD_RIGHT 9
#define BUTTON_DPAD_DOWN 10
#define BUTTON_DPAD_LEFT 11
#define BUTTON_GEAR_DOWN 12
#define BUTTON_GEAR_UP 13
#define BUTTON_NEUTRAL 14

// Analog helpers
#define V_SHIFTERX input.analog[SHIFTERX]
#define V_SHIFTERY input.analog[SHIFTERY]
#define V_ACCELERATOR input.analog[ACCELERATOR]
#define V_ACCELERATOR input.analog[ACCELERATOR]
#define V_BRAKE input.analog[BRAKE]
#define V_CLUTCH input.analog[CLUTCH]

// Gears
#define GEAR_FAIL -1
#define GEAR_1 0
#define GEAR_2 1
#define GEAR_3 2
#define GEAR_4 3
#define GEAR_5 4
#define GEAR_6 5
#define GEAR_NEUTRAL 6
#define GEAR_REVERSE 7

// Modes
#define MODE_NORMAL 0
#define MODE_HANDBRAKE 1
#define MODE_DEBUG 2
#define MODE_BLIP 3

// USB interval (ms)
#define USB_UPDATE_INTERVAL 2

//#define FILTER_DISABLED

#define ANALOGAXES 5
#define FILTERSIZE 10
#define LOOPANALOG(V) for(V = 0; V < ANALOGAXES; V++)
#define LOOPFILTER(V) for(V = 0; V < FILTERSIZE; V++)

// Device input data
typedef struct {
  uint16_t shiftRegister;
  uint8_t pin[ANALOGAXES];
  int32_t analog[ANALOGAXES];
  int32_t filter[ANALOGAXES][FILTERSIZE];
  uint8_t filterIndex;
} DeviceInput;

// Device calibratiopn data
typedef struct {
  int32_t max[ANALOGAXES];
  int32_t min[ANALOGAXES];
} DeviceCalibration;


// Configuration data
typedef struct {

  // Shifter gear positions
  int32_t shifterMap[28] = {
    // X, Y, Width, Height
    0,     0,   5000, 15000, // 1
    0,     MAX, 5000, 15000, // 2
    32768, 0,   15000, 15000, // 3
    32768, MAX, 15000, 15000, // 4
    MAX, 0,     5000, 15000, // 5
    MAX, MAX,   5000, 15000, // 6 & R
    32768, 32768, 32800, 15000, // N
  };

  // Sequential limits
  int32_t sequentialMin = 27000;
  int32_t sequentialMax = 43000;

  // Handbrake range
  int32_t handbrakeMin = 36500;
  int32_t handbrakeMax = 47000;
  int32_t handbrakeClutchMin = 23000;
  int32_t handbrakeClutchMax = 32000;
  

  

  // Throttle blip
  int32_t blipLength = 120;
  int32_t blipRiseTime = 30;
  int32_t blipFallTime = 30;
  

  // Brake Map (Gamma 3.0)
  int32_t brakeMap[20] = {
    5957, 49,
    11915, 393,
    17873, 1329,
    23830, 3151,
    29788, 6154,
    35746, 10635,
    41704, 16888,
    47661, 25209,
    53619, 35894,
    59577, 49237,
  };

  // Shifter shift register buttons
  int8_t shiftRegisterMap[16] = {
    // Chip 1
    -1, // NC
    -1, // Reverse
    -1, // NC
    -1, // Sequential Mode
    BUTTON_RED3, // Red #3
    BUTTON_RED2, // Red #2
    BUTTON_RED4, // Red #4
    BUTTON_RED1, // Red #1

    // Chip 2
    BUTTON_BLACK_UP,    // Black Up
    BUTTON_BLACK_RIGHT, // Black Right
    BUTTON_BLACK_LEFT,  // Black Left
    BUTTON_BLACK_DOWN,  // Black Down
    BUTTON_DPAD_RIGHT,  // Dpad Right
    BUTTON_DPAD_LEFT,   // Dpad Left
    BUTTON_DPAD_DOWN,   // Dpad Down
    BUTTON_DPAD_UP      // Dpad Up
  };
} Configuration;

