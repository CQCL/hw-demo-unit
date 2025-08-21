// Version 1.0.0

/////////////////////////////////////////////////////////////////////////////
//
// (c) 2024-2025 Quantinuum LLC. All Rights Reserved.
// This software and all information and expression are the property of
// Quantinuum LLC, are Quantinuum LLC Confidential & Proprietary,
// contain trade secrets and may not, in whole or in part, be licensed,
// used, duplicated, disclosed, or reproduced for any purpose without prior
// written permission of Quantinuum LLC.
//
/////////////////////////////////////////////////////////////////////////////


#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// INTIALIZING DIGITAL IN/OUT PINS
const int LOAD_PIN = 22;        // 1
const int INITIALIZE_PIN = 23;  // 2
const int COOL_PIN = 24;        // 3
const int SQGATE_PIN = 26;      // 5
const int TQGATE_PIN = 25;      // 4
const int MEASURE_PIN = 27;     // 6
const int BUTTON6_RED_PIN = 9;
const int BUTTON6_GREEN_PIN = 10;
const int BUTTON6_BLUE_PIN = 11;

const float RED_SCALE = 0.8;  // correcting for red overpower

// INTITIALIZING LED STRIPS
Adafruit_NeoPixel strip270 = Adafruit_NeoPixel(145, 6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip315 = Adafruit_NeoPixel(145, 7, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip90 = Adafruit_NeoPixel(145, 3, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip45 = Adafruit_NeoPixel(145, 2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip135 = Adafruit_NeoPixel(145, 4, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip225 = Adafruit_NeoPixel(145, 5, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring = Adafruit_NeoPixel(60, 8, NEO_GRB + NEO_KHZ800);

// INITIALIZING INTERRUPT FLAGS FOR BUTTONS
bool loadFlag = false;
bool initializeFlag = false;
bool measureFlag = false;
bool SQGateFlag = false;
bool TQGateFlag = false;
bool coolFlag = false;

// INITIALIZING CONTROL VARIABLES
bool delayVar = false;
bool clear = false;
bool breakLoop = false;
const int DELAY = 500;

void setup() {

  pwm.begin();
  pwm.setPWMFreq(1000);
  Serial.begin(9600);  // necessary if using serial prints for debugging

  // SETTING BUTTON PINMODES
  pinMode(LOAD_PIN, INPUT_PULLUP);
  pinMode(INITIALIZE_PIN, INPUT_PULLUP);
  pinMode(MEASURE_PIN, INPUT_PULLUP);
  pinMode(SQGATE_PIN, INPUT_PULLUP);
  pinMode(TQGATE_PIN, INPUT_PULLUP);
  pinMode(COOL_PIN, INPUT_PULLUP);

  // ATTACH INTERRUPTS FOR DYNAMIC BUTTON HANDLING
  attachInterrupt(digitalPinToInterrupt(LOAD_PIN), loadInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(INITIALIZE_PIN), initializeInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(COOL_PIN), coolInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(SQGATE_PIN), SQGateInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(TQGATE_PIN), TQGateInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(MEASURE_PIN), measureInterrupt, FALLING);

  // ensure all buttons flags are still false after attaching interrupts
  loadFlag = false;
  initializeFlag = false;
  measureFlag = false;
  SQGateFlag = false;
  TQGateFlag = false;
  coolFlag = false;

  // INITIALIZING BUTTON COLORS
  setLEDColor(0, 0, 255, 150);    // LED 1: Teal (channels 0, 1, 2)
  setLEDColor(1, 255, 255, 255);  // LED 2: White (channels 3, 4, 5)
  setLEDColor(2, 240, 65, 90);    // LED 3: Pink (channels 6, 7, 8)
  setLEDColor(3, 50, 0, 255);     // LED 4: Purple (channels 9, 10, 11)
  setLEDColor(4, 255, 130, 50);   // LED 5: Orange (channels 12, 13, 14)

  // configure color of button 6, light teal
  pinMode(BUTTON6_RED_PIN, OUTPUT);
  pinMode(BUTTON6_GREEN_PIN, OUTPUT);
  pinMode(BUTTON6_BLUE_PIN, OUTPUT);
  analogWrite(BUTTON6_RED_PIN, 175); // note color is inverted
  analogWrite(BUTTON6_GREEN_PIN, 10);
  analogWrite(BUTTON6_BLUE_PIN, 40); 

  // INTIALIZING LED STRIPS
  strip270.begin();
  strip270.show();
  strip270.setBrightness(120);

  strip315.begin();
  strip315.show();
  strip315.setBrightness(120);

  strip225.begin();
  strip225.show();
  strip225.setBrightness(120);

  strip45.begin();
  strip45.show();
  strip45.setBrightness(120);

  strip90.begin();
  strip90.show();
  strip90.setBrightness(120);

  strip135.begin();
  strip135.show();
  strip135.setBrightness(120);

  ring.begin();
  ring.show();
  ring.setBrightness(170);
}

void loop() {

  //CLEAR EVERYTHING
  if (clear) {
    allClear();
    clear = false;
  }

  if (delayVar) {
    delay(DELAY);
    delayVar = false;
  }

  // CHECK FOR INTERRUPT FLAGS
  if (loadFlag) {
    load();
    loadFlag = false;
  }

  if (initializeFlag) {
    initialize();
    initializeFlag = false;
  }

  if (coolFlag) {
    cool();
    coolFlag = false;
  }

  if (SQGateFlag) {
    SQGate();
    SQGateFlag = false;
  }

  if (TQGateFlag) {
    TQGate();
    TQGateFlag = false;
  }

  if (measureFlag) {
    measure();
    measureFlag = false;
  }

  if (delayVar) {
    delay(DELAY);
    delayVar = false;
  }

  // IDLE SEQUENCE
  colorWipeRing(170, 170, 170);
  idle();
  breakLoop = false;
}

void setLEDColor(int buttonNum, int red, int green, int blue) {
  if (buttonNum < 0 || buttonNum > 4) return;  // Validate button number
  // Channel assignments: 3 channels per LED (R, G, B)
  int redChannel = buttonNum * 3;        // 0, 3, 6, 9, 12
  int greenChannel = buttonNum * 3 + 1;  // 1, 4, 7, 10, 13
  int blueChannel = buttonNum * 3 + 2;   // 2, 5, 8, 11, 14
  int scaledRed = (int)(red * RED_SCALE);
  int redPWM = map(255 - scaledRed, 0, 255, 0, 4095);
  int greenPWM = map(255 - green, 0, 255, 0, 4095);
  int bluePWM = map(255 - blue, 0, 255, 0, 4095);
  pwm.setPWM(redChannel, 0, redPWM);
  pwm.setPWM(greenChannel, 0, greenPWM);
  pwm.setPWM(blueChannel, 0, bluePWM);
}

void colorWipeAll(int r, int g, int b) {
  for (int i = 150; i >= 0; i--) {
    if (breakLoop) { break; }
    strip45.setPixelColor(i, r, g, b);
    strip45.show();
    strip225.setPixelColor(i, r, g, b);
    strip225.show();
    strip90.setPixelColor(i, r, g, b);
    strip90.show();
    strip270.setPixelColor(i, r, g, b);
    strip270.show();
    strip135.setPixelColor(i, r, g, b);
    strip135.show();
    strip315.setPixelColor(i, r, g, b);
    strip315.show();
  }
}

void simultaneousPulse(int r1, int g1, int b1, int stripNum1, int r2, int g2, int b2, int stripNum2, int wait) {
  Adafruit_NeoPixel* strip1;

  switch (stripNum1) {
    case 270: strip1 = &strip270; break;
    case 315: strip1 = &strip315; break;
    case 45: strip1 = &strip45; break;
    case 225: strip1 = &strip225; break;
    case 135: strip1 = &strip135; break;
    case 90: strip1 = &strip90; break;
    default: return;  // Invalid strip number
  }

  Adafruit_NeoPixel* strip2;

  switch (stripNum2) {
    case 270: strip2 = &strip270; break;
    case 315: strip2 = &strip315; break;
    case 45: strip2 = &strip45; break;
    case 225: strip2 = &strip225; break;
    case 135: strip2 = &strip135; break;
    case 90: strip2 = &strip90; break;
    default: return;  // Invalid strip number
  }

  for (int i = 166; i >= 0; i--) {
    strip1->setPixelColor(i, r1, g1, b1);
    strip1->show();
    strip2->setPixelColor(i, r2, g2, b2);
    strip2->show();
  }
}

void colorWipeRing(int r, int g, int b) {
  for (int i = 0; i < ring.numPixels(); i++) {
    ring.setPixelColor(i, r, g, b);
    ring.show();
  }
}

void laserPulse(int r, int g, int b, int stripNum, int wait) {
  Adafruit_NeoPixel* strip;

  switch (stripNum) {
    case 270: strip = &strip270; break;
    case 315: strip = &strip315; break;
    case 45: strip = &strip45; break;
    case 225: strip = &strip225; break;
    case 135: strip = &strip135; break;
    case 90: strip = &strip90; break;
    default: return;  // Invalid strip number
  }

  for (int i = 166; i >= 0; i--) {
    strip->setPixelColor(i, r, g, b);
    strip->show();
  }
}

void idle() {
  colorWipeAll(0, 150, 100);
  if (breakLoop) { return; }
  colorWipeAll(170, 50, 0);
  if (breakLoop) { return; }
  colorWipeAll(75, 0, 200);
  if (breakLoop) { return; }
  colorWipeAll(115, 250, 200);
  if (breakLoop) { return; }
  colorWipeAll(170, 0, 50);
  if (breakLoop) { return; }
  colorWipeAll(225, 246, 242);
}

void allClear() {
  strip45.clear();
  strip90.clear();
  strip135.clear();
  strip225.clear();
  strip270.clear();
  strip315.clear();
  ring.clear();
  strip45.show();
  strip90.show();
  strip135.show();
  strip225.show();
  strip270.show();
  strip315.show();
  ring.show();
}

void allWipeClear() {
  for (int i = 150; i >= 0; i--) {
    strip45.setPixelColor(i, 0, 0, 0);
    strip45.show();
    strip225.setPixelColor(i, 0, 0, 0);
    strip225.show();
    strip90.setPixelColor(i, 0, 0, 0);
    strip90.show();
    strip270.setPixelColor(i, 0, 0, 0);
    strip270.show();
    strip135.setPixelColor(i, 0, 0, 0);
    strip135.show();
    strip315.setPixelColor(i, 0, 0, 0);
    strip315.show();
  }
  ring.clear();
  ring.show();
}

void load() {
  colorWipeRing(97, 0, 200);
  laserPulse(97, 0, 200, 135, 1);
  runButton();
}
void initialize() {
  colorWipeRing(255, 0, 0);
  laserPulse(255, 0, 0, 90, 1);
  runButton();
}
void cool() {
  colorWipeRing(0, 0, 150);
  simultaneousPulse(0, 0, 255, 135, 0, 0, 255, 315, 1);
  runButton();
}
void TQGate() {
  colorWipeRing(0, 255, 0);
  simultaneousPulse(0, 255, 0, 135, 0, 255, 0, 45, 1);
  runButton();
}
void SQGate() {
  colorWipeRing(0, 255, 0);
  laserPulse(0, 255, 0, 225, 1);
  runButton();
}
void measure() {
  colorWipeRing(255, 225, 0);
  laserPulse(255, 225, 0, 270, 1);
  runButton();
}
void runButton() {
  allWipeClear();
  delayVar = true;
}

void loadInterrupt() {
  buttonInterrupt();
  loadFlag = true;
}
void initializeInterrupt() {
  buttonInterrupt();
  initializeFlag = true;
}
void coolInterrupt() {
  buttonInterrupt();
  coolFlag = true;
}
void TQGateInterrupt() {
  buttonInterrupt();
  TQGateFlag = true;
}
void SQGateInterrupt() {
  buttonInterrupt();
  SQGateFlag = true;
}
void measureInterrupt() {
  buttonInterrupt();
  measureFlag = true;
}
void buttonInterrupt() {
  breakLoop = true;
  clear = true;
  delayVar = true;
}
