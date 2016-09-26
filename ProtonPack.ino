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
#define MUSIC_CS 7
#define MUSIC_DATA 6
#define MUSIC_CARDCS 4
#define MUSIC_REQ 3
MusicPlayer *music;

// Synchrotron
#define SYNC1_NPIXELS 24
#define SYNC1_DATA 5
Synchrotron *sync1;

// Debug LED
#define DEBUG 13

// Inputs
#define TRIGGER 9

// global time counter
unsigned long jiffies = 0;

void setup() {
  randomSeed(analogRead(12));

  // inputs
  pinMode(TRIGGER, INPUT_PULLUP);

  // outputs
  pinMode(DEBUG, OUTPUT);

  // music player, this sets up SPI for us
  music = new MusicPlayer(MUSIC_CS, MUSIC_DATA, MUSIC_REQ, MUSIC_CARDCS);

  // synchrotron
  sync1 = new Synchrotron(SYNC1_NPIXELS, SYNC1_DATA);
}


void flashDebug() {
  uint8_t val;

  val = (jiffies % 100) < 50;
  digitalWrite(DEBUG, val);
}

void loop() {
  static int state = 0;
  // 6 seems to be about what my overly-critical brain needs to buffer out
  // any music player delays so that they're unnoticeable
  unsigned long new_jiffies = millis() / 6;
  boolean trigger = ! digitalRead(TRIGGER);

  music->poll(jiffies);

  if (state == 0) {
    if (new_jiffies > jiffies) {
      if (trigger) {
	state = 1;
	music->startPlayingFile("track001.mp3");
	sync1->charge();
      }

      jiffies = new_jiffies;
      sync1->tick(jiffies);
      flashDebug();
    }
  }
}

