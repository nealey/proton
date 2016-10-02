// Proton Pack with NeoPixels
// Boy howdy do these make everything easy

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include "MusicPlayer.h"
#include "Synchrotron.h"

// Music Player
#define MUSIC_MCS 7
#define MUSIC_DCS 6
#define MUSIC_CCS 4
#define MUSIC_DRQ 0 // Cut trace on board and wire to 0, so you can use 3 for SPI
MusicPlayer *music;

// LED output scaling
#define brightness 255

// Synchrotron
#define SYNC1_NPIXELS 24
#define SYNC1_DATA 5
Synchrotron *sync1;

// Neutrona Wand
#define WAND1_NPIXELS 14
#define WAND1_DATA 9
Synchrotron *wand1;

// Displays
Adafruit_7segment disp1;
Adafruit_7segment disp2;

// Debug LED
#define DEBUG 13

// Inputs
#define TRIGGER 8
#define POT1 A0

// global time counter
unsigned long jiffies = 0;

// 6 seems to be about what my overly-critical brain needs to buffer out
// any delays caused by NMI sections of music player code so that they're unnoticeable
#define MILLIS_PER_JIFFY 6


void setup() {
  randomSeed(analogRead(12));

  // inputs
  pinMode(TRIGGER, INPUT_PULLUP);

  // outputs
  pinMode(DEBUG, OUTPUT);

  // music player, this sets up SPI for us
  music = new MusicPlayer(MUSIC_MCS, MUSIC_DCS, MUSIC_DRQ, MUSIC_CCS);

  // synchrotron
  sync1 = new Synchrotron(SYNC1_NPIXELS, SYNC1_DATA);

  // nuetrona wand
  wand1 = new Synchrotron(WAND1_NPIXELS, WAND1_DATA);

  // 7segment displays
  disp1 = Adafruit_7segment();
  disp1.begin(0x70);

  disp2 = Adafruit_7segment();
  disp2.begin(0x71);
}


void flashDebug() {
  uint8_t val;

  val = (jiffies % 100) < 50;
  digitalWrite(DEBUG, val);
}

void loop() {
  static int state = 0;
  static float disp2val = 40.83;
  unsigned long new_jiffies = millis() / MILLIS_PER_JIFFY;
  boolean trigger = ! digitalRead(TRIGGER);

  music->poll(jiffies);

  switch (state) {
  case 0: // move to steady state
    sync1->transition(400, 12, brightness, 0, 0);
    wand1->transition(400, 24, brightness, 0, 0);
    state = 10;
    break;
  case 10: // waiting for charge button
    if (trigger && sync1->transitioned() && music->startPlayingFile("track001.mp3")) {
      state = 20;
      sync1->transition(700, 2, brightness, brightness/8, 0);
      wand1->transition(700, 10, brightness, brightness/8, 0);
    }
    break;
  case 20: // charge button pressed
    if (sync1->transitioned()) {
      state = 30;
    }
    break;
  case 30: // waiting for fire button
    if (trigger && music->startPlayingFile("nutrona.mp3")) {
      state = 40;
      sync1->transition(40, 1, brightness/8, brightness/4, brightness/2);
      wand1->transition(40, 5, brightness/6, brightness/2, brightness/6);
    }
    break;
  case 40: // fire button pressed
    if (! trigger && music->startPlayingFile("track002.mp3")) {
      state = 0;
    }
    break;
  }

  if (new_jiffies > jiffies) {
    jiffies = new_jiffies;
    sync1->tick(jiffies);
    wand1->tick(jiffies);

    if (jiffies % 10 == 0) {
      // This is expensive
      disp1.printFloat(5198 * sync1->speed());
      disp1.writeDisplay();

      disp2val = analogRead(POT1) / 10.0;
      disp2.printFloat(disp2val, 1);
      disp2.writeDisplay();
    }
    flashDebug();
  }
}

