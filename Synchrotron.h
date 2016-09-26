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
public:
  Synchrotron(uint16_t n, uint8_t p=6, neoPixelType t=NEO_GRB + NEO_KHZ800);
  standby();
  charge();
  fire();
  discharge();
  tick(unsigned long jiffies);	// Try to call this every jiffy
};
