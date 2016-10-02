#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>


class Synchrotron {
  Adafruit_NeoPixel *pxl;
  uint16_t npixels;  // How many pixels there are
  int cur;	     // Which pixel the synchrotron is on, currently
  int tickrate;	     // How many millis between pixel position changes
  int ticks; // How many ticks have elapsed since last position change
  byte r, g, b;			// Current color

  int transition_length, transition_elapsed;
  int initial_tickrate, initial_r, initial_g, initial_b;
  float dtickrate, dr, dg, db;
  float ftickrate;
public:
  Synchrotron(uint16_t n, uint8_t p=6, neoPixelType t=NEO_GRB + NEO_KHZ800);
  transition(int duration, int final_tickrate, byte final_r, byte final_g, byte final_b);
  bool transitioned();
  float speed();
  standby();
  charge();
  fire();
  discharge();
  tick(unsigned long jiffies);	// Try to call this every jiffy
};
