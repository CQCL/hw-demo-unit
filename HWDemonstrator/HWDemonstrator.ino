/*****************************************************************************
*
* (c) 2024-2025 @ Quantinuum LLC. All Rights Reserved.
* This software and all information and expression are the property of
* Quantinuum LLC, are Quantinuum LLC Confidential & Proprietary,
* contain trade secrets and may not, in whole or in part, be licensed,
* used, duplicated, disclosed, or reproduced for any purpose without prior
* written permission of Quantinuum LLC.
*
*****************************************************************************/
// Version 1.1.0

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// INITIALIZING DIGITAL IN/OUT PINS (BUTTONS)
const int LOAD_PIN = 22;        // 1
const int INITIALIZE_PIN = 23;  // 2
const int COOL_PIN = 24;        // 3
const int TQGATE_PIN = 25;      // 4
const int SQGATE_PIN = 26;      // 5
const int MEASURE_PIN = 27;     // 6
const int BUTTON6_RED_PIN = 9;  // button 6 is wired to different board and requires more pins
const int BUTTON6_GREEN_PIN = 10;
const int BUTTON6_BLUE_PIN = 11;

// INITIALIZING LED STRIP INFO
enum Strips {
  STRIP45,
  STRIP90,
  STRIP135,
  STRIP225,
  STRIP270,
  STRIP315,
  RING
};
const int stripPins[] = { 2, 3, 4, 5, 6, 7, 8 };
const int numPixels[] = { 145, 145, 145, 145, 145, 145, 60 };
Adafruit_NeoPixel *stripList[7];

// INITIALIZE DEVICE STATES
enum DeviceState {
  IDLE,
  LOAD,
  INITIALIZE,
  COOL,
  SQGATE,
  TQGATE,
  MEASURE,
  NONE
};

// INITIALIZING CONTROL VARIABLES
const int DELAY = 500;
DeviceState stateChangeRequest = NONE;
DeviceState currentState = IDLE;

int buttonColors[6][3] = {
  { 0, 255, 150 },    // TEAL
  { 255, 255, 255 },  // WHITE
  { 240, 65, 90 },    // PINK
  { 50, 0, 255 },     // PURPLE
  { 255, 130, 50 },   // ORANGE
  { 50, 245, 215 },   // LIGHT TEAL
};

int laserColors[8][3] = {
  { 97, 0, 200 },    // PURPLE
  { 255, 0, 0 },     // RED
  { 0, 0, 255 },     // BLUE
  { 0, 255, 0 },     // GREEN
  { 0, 255, 0 },     // GREEN
  { 255, 225, 0 },   // YELLOW
  { 0, 0, 0 },       // BLACK
  { 255, 255, 255 }  // WHITE
};

int idleColors[7][3] = {
  { 0, 150, 100 },    // GREEN
  { 170, 50, 0 },     // RED
  { 75, 0, 200 },     // INDIGO
  { 115, 250, 200 },  // LIGHT GREEN
  { 170, 0, 50 },     // RED
  { 225, 246, 242 },  // PALE GREEN
  { 170, 170, 170 },  // GREY
};

void setup() {

  pwm.begin();
  pwm.setPWMFreq(1000);
  Serial.begin(9600);

  // SETTING BUTTON PIN MODES
  pinMode(LOAD_PIN, INPUT_PULLUP);
  pinMode(INITIALIZE_PIN, INPUT_PULLUP);
  pinMode(MEASURE_PIN, INPUT_PULLUP);
  pinMode(SQGATE_PIN, INPUT_PULLUP);
  pinMode(TQGATE_PIN, INPUT_PULLUP);
  pinMode(COOL_PIN, INPUT_PULLUP);
  pinMode(BUTTON6_RED_PIN, OUTPUT);
  pinMode(BUTTON6_GREEN_PIN, OUTPUT);
  pinMode(BUTTON6_BLUE_PIN, OUTPUT);

  // ATTACH INTERRUPTS FOR DYNAMIC BUTTON HANDLING
  attachInterrupt(digitalPinToInterrupt(LOAD_PIN), loadInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(INITIALIZE_PIN), initializeInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(COOL_PIN), coolInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(SQGATE_PIN), SQGateInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(TQGATE_PIN), TQGateInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(MEASURE_PIN), measureInterrupt, FALLING);

  // INITIALIZING BUTTON COLORS
  defaultButtons();

  // INITIALIZING LED STRIPS
  stripList[STRIP45] = new Adafruit_NeoPixel(numPixels[STRIP45], stripPins[STRIP45], NEO_GRB + NEO_KHZ800);
  stripList[STRIP90] = new Adafruit_NeoPixel(numPixels[STRIP90], stripPins[STRIP90], NEO_GRB + NEO_KHZ800);
  stripList[STRIP135] = new Adafruit_NeoPixel(numPixels[STRIP135], stripPins[STRIP135], NEO_GRB + NEO_KHZ800);
  stripList[STRIP225] = new Adafruit_NeoPixel(numPixels[STRIP225], stripPins[STRIP225], NEO_GRB + NEO_KHZ800);
  stripList[STRIP270] = new Adafruit_NeoPixel(numPixels[STRIP270], stripPins[STRIP270], NEO_GRB + NEO_KHZ800);
  stripList[STRIP315] = new Adafruit_NeoPixel(numPixels[STRIP315], stripPins[STRIP315], NEO_GRB + NEO_KHZ800);
  stripList[RING] = new Adafruit_NeoPixel(numPixels[RING], stripPins[RING], NEO_GRB + NEO_KHZ800);

  for (int i = 0; i < 7; i++) {
    stripList[i]->begin();
    stripList[i]->show();
    stripList[i]->setBrightness(120);
  }
  stripList[RING]->setBrightness(170);

  currentState = IDLE;
}

void loop() {

  if (stateChangeRequest != NONE) {
    currentState = stateChangeRequest;
    stateChangeRequest = NONE;

    // clear all pixels before switching states
    for (int i = 0; i < 7; i++) {
      stripList[i]->clear();
      stripList[i]->show();
    }
    delay(DELAY);
  }

  switch (currentState) {
    case IDLE:
      delay(DELAY);
      colorWipeRing(idleColors[6]);
      idle();
      break;
    case LOAD:
      load();
      break;
    case INITIALIZE:
      initialize();
      break;
    case COOL:
      cool();
      break;
    case SQGATE:
      SQGate();
      break;
    case TQGATE:
      TQGate();
      break;
    case MEASURE:
      measure();
      break;
  }

  currentState = IDLE;
}


/////////////////////////////////////////////////////////////////////
// Functions for LED behaviors
/////////////////////////////////////////////////////////////////////

void setButtonColor(int buttonNum, int color[]) {
  if (buttonNum < 1 || buttonNum > 5) return;  // Validate button number
  // Channel assignments: 3 channels per LED (R, G, B)
  int redChannel = (buttonNum - 1) * 3;        // 0, 3, 6, 9, 12
  int greenChannel = (buttonNum - 1) * 3 + 1;  // 1, 4, 7, 10, 13
  int blueChannel = (buttonNum - 1) * 3 + 2;   // 2, 5, 8, 11, 14

  int scaledRed = (int)(color[0] * 0.8);
  int redPWM = map(255 - scaledRed, 0, 255, 0, 4095);
  int greenPWM = map(255 - color[1], 0, 255, 0, 4095);
  int bluePWM = map(255 - color[2], 0, 255, 0, 4095);

  pwm.setPWM(redChannel, 0, redPWM);
  pwm.setPWM(greenChannel, 0, greenPWM);
  pwm.setPWM(blueChannel, 0, bluePWM);
}

void setButtonSix(int color[]) {
  if (color == laserColors[6]) {
    pinMode(BUTTON6_RED_PIN, INPUT);
    pinMode(BUTTON6_GREEN_PIN, INPUT);
    pinMode(BUTTON6_BLUE_PIN, INPUT);
  } else {
    pinMode(BUTTON6_RED_PIN, OUTPUT);
    pinMode(BUTTON6_GREEN_PIN, OUTPUT);
    pinMode(BUTTON6_BLUE_PIN, OUTPUT);
    analogWrite(BUTTON6_RED_PIN, 255-color[0]);
    analogWrite(BUTTON6_GREEN_PIN, 255-color[1]);
    analogWrite(BUTTON6_BLUE_PIN, 255-color[2]);
  }
}

void spotlightButton(int buttonNum) {
  if (buttonNum < 1 || buttonNum > 6) return;
  for (int i = 1; i < 6; i++) {
    if (i == buttonNum) {
      setButtonColor(i, laserColors[i - 1]);
    } else {
      setButtonColor(i, laserColors[6]);
    }
  }
  if (buttonNum == 6) {
    int color[] = {145, 225, 0};
    setButtonSix(color);
  } else {
    setButtonSix(laserColors[6]);
  }
}

void defaultButtons() {
  for (int i = 1; i < 6; i++) {
    setButtonColor(i, buttonColors[i - 1]);
  }
  setButtonSix(buttonColors[5]);
}

void colorWipeStrips(int color[]) {
  for (int i = 150; i >= 0; i--) {
    if (stateChangeRequest != NONE) { return; }
    for (int j = 0; j < 6; j++) {
      stripList[j]->setPixelColor(i, color[0], color[1], color[2]);
      stripList[j]->show();
    }
  }
}

void laserPulse(int color[], Adafruit_NeoPixel *strip) {
  for (int i = 166; i >= 0; i--) {
    strip->setPixelColor(i, color[0], color[1], color[2]);
    strip->show();
  }
}

void simultaneousPulse(int color1[], Adafruit_NeoPixel *strip1, int color2[], Adafruit_NeoPixel *strip2) {
  for (int i = 166; i >= 0; i--) {
    strip1->setPixelColor(i, color1[0], color1[1], color1[2]);
    strip1->show();
    strip2->setPixelColor(i, color2[0], color2[1], color2[2]);
    strip2->show();
  }
}

void colorWipeRing(int color[]) {
  for (int i = 0; i < numPixels[6]; i++) {
    stripList[RING]->setPixelColor(i, color[0], color[1], color[2]);
    stripList[RING]->show();
  }
}

void idle() {
  colorWipeStrips(idleColors[0]);
  colorWipeStrips(idleColors[1]);
  colorWipeStrips(idleColors[2]);
  colorWipeStrips(idleColors[3]);
  colorWipeStrips(idleColors[4]);
  colorWipeStrips(idleColors[5]);
}

/////////////////////////////////////////////////////////////////////
// Functions to handle state behavior
/////////////////////////////////////////////////////////////////////
void colorWipeAllClear() {
  colorWipeStrips(laserColors[6]);
  colorWipeRing(laserColors[6]);
  defaultButtons();
}

void load() {
  spotlightButton(1);
  laserPulse(laserColors[0], stripList[STRIP135]);
  colorWipeRing(laserColors[0]);
  colorWipeAllClear();
}

void initialize() {
  spotlightButton(2);
  laserPulse(laserColors[1], stripList[STRIP90]);
  colorWipeRing(laserColors[1]);
  colorWipeAllClear();
}

void cool() {
  spotlightButton(3);
  simultaneousPulse(laserColors[2], stripList[STRIP135], laserColors[2], stripList[STRIP315]);
  colorWipeRing(laserColors[2]);
  colorWipeAllClear();
}

void TQGate() {
  spotlightButton(4);
  simultaneousPulse(laserColors[3], stripList[STRIP135], laserColors[3], stripList[STRIP45]);
  colorWipeRing(laserColors[3]);
  colorWipeAllClear();
}

void SQGate() {
  spotlightButton(5);
  laserPulse(laserColors[4], stripList[STRIP225]);
  colorWipeRing(laserColors[4]);
  colorWipeAllClear();
}

void measure() {
  spotlightButton(6);
  laserPulse(laserColors[5], stripList[STRIP270]);
  colorWipeRing(laserColors[5]);
  colorWipeAllClear();
}


/////////////////////////////////////////////////////////////////////
// Functions to handle interrupt boolean setting
/////////////////////////////////////////////////////////////////////

void loadInterrupt() {
  stateChangeRequest = LOAD;
}

void initializeInterrupt() {
  stateChangeRequest = INITIALIZE;
}

void coolInterrupt() {
  stateChangeRequest = COOL;
}

void TQGateInterrupt() {
  stateChangeRequest = TQGATE;
}

void SQGateInterrupt() {
  stateChangeRequest = SQGATE;
}

void measureInterrupt() {
  stateChangeRequest = MEASURE;
}
