#include "USBAPI.h"

#include <TimerOne.h>
#include "G25Adapter.h"

GameControllerData_t controllerData;
GameControllerData_t controllerDataOld;
int32_t currentGear, lastGear;

Configuration config;
DeviceInput input;
DeviceCalibration calibration;

uint8_t shifterMode;
int32_t throttleBlipTimer;
int32_t time;

uint8_t vendorData[32];
uint32_t externalButtons;

//
// Reset input
//
void resetInput(DeviceInput& input) {
  int i, j;
  input.shiftRegister = 0;

  input.pin[SHIFTERX] = SHIFTERX_PIN;
  input.pin[SHIFTERY] = SHIFTERY_PIN;
  input.pin[ACCELERATOR] = ACCELERATOR_PIN;
  input.pin[BRAKE] = BRAKE_PIN;
  input.pin[CLUTCH] = CLUTCH_PIN;

  input.filterIndex = 0;
  LOOPANALOG(i) {
    input.analog[i] = CENTER;
    LOOPFILTER(j) {
      input.filter[i][j] = CENTER;
    }
  }
}

//
// Reset calibration
//
void resetCalibration(DeviceCalibration& calibration) {
  int i;
  LOOPANALOG(i) {
    calibration.max[i] = CENTER;
    calibration.min[i] = CENTER;
  }

  // Calibration Defaults
  calibration.max[SHIFTERX] = 51000;
  calibration.min[SHIFTERX] = 43000;
  calibration.max[SHIFTERY] = 32000;
  calibration.min[SHIFTERY] = 300;
  calibration.max[ACCELERATOR] = 39000;
  calibration.min[ACCELERATOR] = 6000;
  calibration.max[BRAKE] = 39000;
  calibration.min[BRAKE] = 5500;
  calibration.max[CLUTCH] = 39000;
  calibration.min[CLUTCH] = 7000;

}

//
// Auto-Calibration adjustment
//
void autoCalibration(DeviceCalibration& calibration, DeviceInput& input) {
  int i;
  if (shifterMode == MODE_DEBUG && (controllerData.buttons & 0x01)) {
    return;
  }
  LOOPANALOG(i) {

    // Move min/max positions
    if (input.analog[i] > calibration.max[i]) {
      //calibration.max[i] = input.analog[i];
    }
    if (input.analog[i] < calibration.min[i]) {
      //calibration.min[i] = input.analog[i];
    }

    // Validate calibration and correct the analog value
    if (true /* || calibration.max[i] > CENTER && calibration.min[i] < CENTER */) {

      if (input.analog[i] > calibration.max[i] - DEADZONE) {
        input.analog[i] = calibration.max[i];
      }
      if (input.analog[i] < calibration.min[i] + DEADZONE) {
        input.analog[i] = calibration.min[i];
      }

      input.analog[i] = (1024 * (input.analog[i] - calibration.min[i] - DEADZONE) / (calibration.max[i] - calibration.min[i] - DEADZONE - DEADZONE)) << 6;
      if (input.analog[i] < MIN) input.analog[i] = 0;
      else if (input.analog[i] > MAX) input.analog[i] = MAX;
    }
  }
}


//
// Read Input
//
void readDeviceInput(DeviceInput& input) {
  int i, j;
  int measurement;
  uint32_t sum;

  //
  // Read shift registers
  //
  input.shiftRegister = 0;
  digitalWrite(MODE_PIN, LOW);         // Parallel mode: inputs are read into shift register
  delayMicroseconds(10);               // Wait for signal to settle
  digitalWrite(MODE_PIN, HIGH);        // Serial mode: data bits are output on clock falling edge

  for (i = 0; i < 16; i++)             // Iteration over both 8 bit registers
  {
    digitalWrite(CLOCK_PIN, LOW);      // Generate clock falling edge
    delayMicroseconds(10);             // Wait for signal to settle

    input.shiftRegister |= digitalRead(DATA_IN_PIN) << i ;  // Read data

    digitalWrite(CLOCK_PIN, HIGH);     // Generate clock rising edge
    delayMicroseconds(10);             // Wait for signal to settle
  }

  if ((input.shiftRegister & 0x01) > 0 && (input.shiftRegister & 0x04) > 0) {
    input.shiftRegister = 0;
  }

  // Read analog axes
  LOOPANALOG(i) {

#ifdef FILTER_DISABLED
    input.analog[i] = (analogRead(input.pin[i]) << 6) & 0xFFFF;
#else
    // Read analog
    input.filter[i][input.filterIndex] = (analogRead(input.pin[i]) << 6) & 0xFFFF;

    // Sum filter values
    sum = 0;
    LOOPFILTER(j) {
      sum += input.filter[i][j];
    }
    input.analog[i] = sum / FILTERSIZE;
#endif

    // Invert analog inputs 2 to 5
    if (i > 0) input.analog[i] = 65535 - input.analog[i];
  }

#ifndef FILTER_DISABLED
  // Rotate filter index
  if (++input.filterIndex >= FILTERSIZE) {
    input.filterIndex = 0;
  }
#endif

  // Shifter is not connected
  if (V_SHIFTERY == 0) {
    V_SHIFTERX = 32768;
    V_SHIFTERY = 32768;
  }

}

//
// Get mapped brake value
//
int32_t getBrakeMapValue(int32_t value) {
  int i;
  int32_t max, min, mapMax, mapMin;

  // Start
  if (value <= config.brakeMap[0]) {
    min = 0;
    mapMin = 0;
    max = config.brakeMap[0];
    mapMax = config.brakeMap[1];

    // End
  } else if (value >= config.brakeMap[18]) {
    min = config.brakeMap[18];
    mapMin = config.brakeMap[19];
    max = 65535;
    mapMax = 65535;

    // Others
  } else {
    for (i = 0; i < 18; i += 2) {
      if (value >= config.brakeMap[i] && value <= config.brakeMap[i + 2]) {
        min = config.brakeMap[i];
        mapMin = config.brakeMap[i + 1];
        max = config.brakeMap[i + 2];
        mapMax = config.brakeMap[i + 3];
      }
    }
  }

  value = mapMin + ((mapMax - mapMin) * (value - min)) / (max - min);

  // Limits
  if (value < MIN) value = MIN;
  if (value > MAX) value = MAX;

  return value;
}

//
// Get selected shifter gear
//
int8_t getSelectedGear(int32_t x, int32_t y) {
  int i;
  int8_t gear;

  gear = GEAR_FAIL;
  for (i = 0; i < 28; i += 4) {
    if (
      x > config.shifterMap[i] - config.shifterMap[i + 2] &&
      x < config.shifterMap[i] + config.shifterMap[i + 2] &&
      y > config.shifterMap[i + 1] - config.shifterMap[i + 3] &&
      y < config.shifterMap[i + 1] + config.shifterMap[i + 3]
    ) {
      gear = i / 4;
    }
  }
  return gear;
}


//
// Check if button is pressed
//
bool isButtonPressed(uint32_t button) {
  button = 1 << (button);
  if (
    (controllerData.buttons & button) > 0 &&
    (controllerData.buttons & button) != (controllerDataOld.buttons & button)
  ) {
    return true;
  }
  return false;
}

//
// Check if button is released
//
bool isButtonReleased(uint32_t button) {
  button = 1 << (button);
  if (
    (controllerData.buttons & button) == 0 &&
    (controllerData.buttons & button) != (controllerDataOld.buttons & button)
  ) {
    return true;
  }
  return false;
}

//
// Check if button is down
//
bool isButtonDown(uint32_t button) {
  button = 1 << (button);
  if ((controllerData.buttons & button) > 0) {
    return true;
  }
  return false;
}



//--------------------------------------------
//            SETUP
//-------------------------------------------
void setup() {
  int i;
  controllerData = getBlankDataForController();
  controllerDataOld = getBlankDataForController();

  controllerData.vendorData = vendorData;
  controllerDataOld.vendorData = vendorData;

  

  resetInput(input);
  resetCalibration(calibration);

  // G25 shifter analog inputs configuration
  pinMode(SHIFTERX_PIN, INPUT_PULLUP);    // X axis
  pinMode(SHIFTERY_PIN, INPUT_PULLUP);    // Y axis
  pinMode(ACCELERATOR_PIN, INPUT_PULLUP); // Accelerator
  pinMode(BRAKE_PIN, INPUT_PULLUP);       // Brake
  pinMode(CLUTCH_PIN, INPUT_PULLUP);      // Clutch


  // G25 shift register interface configuration
  pinMode(CLOCK_PIN, OUTPUT);          // Clock
  pinMode(DATA_IN_PIN, INPUT_PULLUP);  // Data in
  pinMode(MODE_PIN, OUTPUT);           // Parallel/serial mode
  digitalWrite(MODE_PIN, HIGH);
  digitalWrite(CLOCK_PIN, HIGH);


  currentGear = GEAR_NEUTRAL;
  lastGear = GEAR_NEUTRAL;
  shifterMode = MODE_NORMAL;

  externalButtons = 0;

  // Timers
  throttleBlipTimer = 0;
  time = 0;
  Timer1.initialize(1000);
  Timer1.attachInterrupt(onTimer);
}

//-------------------------------------------
//            TIMER (1ms)
//-------------------------------------------
void onTimer(void) {
  if (throttleBlipTimer > 0) {
    throttleBlipTimer--;
  }
  time++;
}


//-------------------------------------------
//            LOOP
//-------------------------------------------
void loop() {
  int i;
  int32_t selectedGear = 0;
  int32_t handbrake = 0;
  int32_t throttle = 0;
  uint8_t data[4];

  uint8_t reportId;

  //
  // Input
  //
  readDeviceInput(input);
  autoCalibration(calibration, input);



  //
  // Shifter gears
  //
  selectedGear = getSelectedGear(V_SHIFTERX, V_SHIFTERY);

  // Reverse gear
  if (selectedGear == GEAR_6 && input.shiftRegister & SR_REVERSE) {
    selectedGear = GEAR_REVERSE;
  }

  // Prevent changing gear to gear without changing to neutral first
  if (selectedGear != currentGear && selectedGear != GEAR_FAIL) {

    // Neutral To Gear
    if (currentGear == GEAR_NEUTRAL && selectedGear >= GEAR_1) {
      currentGear = selectedGear;
      if (shifterMode == MODE_BLIP && currentGear < lastGear) {
        throttleBlipTimer = config.blipLength;
      }
      lastGear = currentGear;

      // Gear To Neutral
    } else if (currentGear >= GEAR_1 && selectedGear == GEAR_NEUTRAL) {
      currentGear = selectedGear;
    }
  }

  // Reset buttons
  controllerData.buttons = 0;

  //
  // Sequential selected on the shifter
  //
  if ((input.shiftRegister & SR_MODE) > 0) {

    // Handbrake Mode
    if (shifterMode == MODE_HANDBRAKE) {
      handbrake = 0;
      
      // Handbrake
      if (V_SHIFTERY > config.handbrakeMin) {
        handbrake = (MAX * (V_SHIFTERY - config.handbrakeMin)) / (config.handbrakeMax - config.handbrakeMin) ;
        if (handbrake > MAX) handbrake = MAX;

        // Clutch
      } else if (V_SHIFTERY < config.handbrakeClutchMax) {
        V_CLUTCH = (MAX * (config.handbrakeClutchMax - V_SHIFTERY)) / (config.handbrakeClutchMax - config.handbrakeClutchMin) ;
        if (V_CLUTCH > MAX) V_CLUTCH = MAX;
      }

      // Sequential Mode (Up = Gear Down, Down = Gear Up)
    } else {
      if (input.shiftRegister & SR_REVERSE) {
        controllerData.buttons = ((uint32_t)1 << BUTTON_NEUTRAL);
      } else if (V_SHIFTERY < config.sequentialMin) {
        controllerData.buttons = ((uint32_t)1 << BUTTON_GEAR_DOWN);
      } else if (V_SHIFTERY > config.sequentialMax) {
        controllerData.buttons = ((uint32_t)1 << BUTTON_GEAR_UP);
      }
    }
  } else {
    controllerData.buttons = currentGear != GEAR_NEUTRAL ? ((uint32_t)1 << (currentGear) << 24) : 0;
  }

  // Shift register to buttons
  for (i = 0; i < 16; i++) {
    if (config.shiftRegisterMap[i] >= 0) {
      if ((input.shiftRegister & (1 << i)) > 0) {
        controllerData.buttons |= (1 << config.shiftRegisterMap[i]);
      }
    }
  }

  // Handbrake button
  if (isButtonDown(BUTTON_RED4)) {
    controllerData.z = MAX;
  } else {
    controllerData.z = handbrake;
  }

  // Debug mode
  if (shifterMode == MODE_DEBUG) {
    controllerData.x = V_SHIFTERX;
    controllerData.y = V_SHIFTERY;
    controllerData.z = input.shiftRegister;
    controllerData.rx = V_ACCELERATOR;
    controllerData.ry = V_BRAKE;
    controllerData.rz = V_CLUTCH;


    // Normal mode
  } else {
    controllerData.x = 0;
    controllerData.y = 0;

    // Throttle Cut Off
    if (shifterMode == MODE_BLIP
        && (
          selectedGear == GEAR_FAIL
          || selectedGear == GEAR_NEUTRAL
          || (currentGear >= GEAR_2 && currentGear <= GEAR_6)
        )
        && V_CLUTCH > 45000
        && V_ACCELERATOR > 62000
       ) {
      controllerData.rx = 0;
    } else {
      controllerData.rx = V_ACCELERATOR;
    }

    controllerData.ry = getBrakeMapValue(V_BRAKE);
    if (shifterMode == MODE_BLIP && (currentGear == GEAR_NEUTRAL)) {
      controllerData.rz = MAX;
    } else {
      controllerData.rz = V_CLUTCH;
    }
  }


  //
  // Throttle Blip
  //

  // No blip when clutch is engaged and no blip when using accelerator
  if (throttleBlipTimer > 0 && V_CLUTCH > 1000 && V_ACCELERATOR < 1000) {


    // Rise
    if (throttleBlipTimer > (config.blipLength - config.blipRiseTime)) {
      throttle = (config.blipLength - throttleBlipTimer) * (MAX / config.blipRiseTime);

      // Middle & Fall
    } else {
      throttle = throttleBlipTimer * (MAX / config.blipFallTime);
    }
    // Limits
    if (throttle > MAX) throttle = MAX;
    if (throttle < MIN) throttle = MIN;

    controllerData.rx = throttle;
  } else {
    throttleBlipTimer = 0;
  }


  //
  // Receive USB (Non-Blocking)
  //
  if (USB_Available(HID_RX)) {
    USB_Recv(HID_RX, &reportId, 1);

    // Raw HID report ID == 3
    if(reportId == 3) {
      USB_Recv(HID_RX, &vendorData, 32);

      // Buttons
      if(vendorData[0] == 1) {
        externalButtons = (uint32_t)vendorData[1];
        externalButtons |= (uint32_t)vendorData[2] << 8;
        externalButtons |= (uint32_t)vendorData[3] << 16;
        externalButtons |= (uint32_t)vendorData[4] << 24;
      }
    }
  }
  if ((input.shiftRegister & SR_MODE) > 0) {
    controllerData.buttons |= externalButtons;
  } else {
    if((externalButtons & ((uint32_t)1 << 12)) > 0) {
      controllerData.rz = MAX;
    }
    if((externalButtons & ((uint32_t)1 << 13)) > 0) {
      controllerData.z = MAX;
    }
    
  }


  // Check if controller data is changed
  if (    controllerData.buttons != controllerDataOld.buttons ||
          controllerData.x != controllerDataOld.x ||
          controllerData.y != controllerDataOld.y ||
          controllerData.z != controllerDataOld.z ||
          controllerData.rx != controllerDataOld.rx ||
          controllerData.ry != controllerDataOld.ry ||
          controllerData.rz != controllerDataOld.rz  )
  {

    if (shifterMode == MODE_DEBUG) {
      vendorData[1] = (V_SHIFTERX) & 0xFF;
      vendorData[2] = (V_SHIFTERX) >> 8 & 0xFF;     
    }
    
    // Send controller data
    GameController.set(controllerData);
    

    // Shifter Mode Change
    if (isButtonDown(BUTTON_RED1) && isButtonDown(BUTTON_RED2)) {

      // Normal Mode
      if (isButtonDown(BUTTON_BLACK_UP)) {
        shifterMode = MODE_NORMAL;

        // Handbrake Mode
      } else if (isButtonDown(BUTTON_BLACK_RIGHT)) {
        shifterMode = MODE_HANDBRAKE;

        // Debug Mode
      } else if (isButtonDown(BUTTON_BLACK_DOWN)) {
        shifterMode = MODE_DEBUG;

        // Blip Mode
      } else if (isButtonDown(BUTTON_BLACK_LEFT)) {
        shifterMode = MODE_BLIP;
      }
    }


    // Red Button 3 == ESC
    if (isButtonPressed(BUTTON_RED3)) {
      Keyboard.press(KEY_ESC);
    } else if (isButtonReleased(BUTTON_RED3)) {
      Keyboard.release(KEY_ESC);
    }

    // DPad Up == Page Up
    if (isButtonPressed(BUTTON_DPAD_UP)) {
      Keyboard.press(KEY_PAGE_UP);
    } if (isButtonReleased(BUTTON_DPAD_UP)) {
      Keyboard.release(KEY_PAGE_UP);
    }

    // DPad Down == Page Down
    if (isButtonPressed(BUTTON_DPAD_DOWN)) {
      Keyboard.press(KEY_PAGE_DOWN);
    } if (isButtonReleased(BUTTON_DPAD_DOWN)) {
      Keyboard.release(KEY_PAGE_DOWN);
    }

  } else {
    delay(1);
  }
  controllerDataOld.buttons = controllerData.buttons;
  controllerDataOld.x = controllerData.x;
  controllerDataOld.y = controllerData.y;
  controllerDataOld.z = controllerData.z;
  controllerDataOld.rx = controllerData.rx;
  controllerDataOld.ry = controllerData.ry;
  controllerDataOld.rz = controllerData.rz;

}


