#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Synchrotron.h"

#define width 6
#define brightness 255

Synchrotron::Synchrotron(uint16_t n, uint8_t p, neoPixelType t)
{
  pxl = new Adafruit_NeoPixel(n, p, t);
  npixels = n;
  cur = 0;
  pxl->begin();
  pxl->show();

  // Inital values of 0 will make it look like it's sort of warming up
  ticks = 0;
  tickrate = 200;
  r = 0;
  g = 0;
  b = 0;

  standby();
}

Synchrotron::transition(int duration, int final_tickrate, byte final_r, byte final_g, byte final_b)
{
  transition_length = duration;
  transition_elapsed = 0;

  dtickrate = (final_tickrate - tickrate) / (float)duration;
  dr = (final_r - r) / (float)duration;
  dg = (final_g - g) / (float)duration;
  db = (final_b - b) / (float)duration;

  initial_tickrate = tickrate;
  initial_r = r;
  initial_g = g;
  initial_b = b;
}

Synchrotron::standby() {
  transition(400, 12, brightness, 0, 0);
}

Synchrotron::charge() {
  transition(400, 2, brightness, brightness/8, 0);
}

Synchrotron::fire() {
  transition(40, 1, brightness/8, brightness/4, brightness/2);
}

Synchrotron::discharge() {
  standby();
}

Synchrotron::tick(unsigned long jiffies) {
  pxl->clear();
  for (int i = 0; i < width; i += 1) {
    pxl->setPixelColor((cur + i) % npixels, pxl->Color(r, g, b));
  }
  for (int i = 0; i < 4; i += 1) {
    int div = 1 << (2*i);
    pxl->setPixelColor((cur + npixels - i) % npixels, pxl->Color(r/div, g/div, b/div));
    pxl->setPixelColor((cur + npixels + width + i) % npixels, pxl->Color(r/div, g/div, b/div));
  }

  pxl->show();

  ticks += 1;
  if (ticks >= tickrate) {
    cur = (cur + 1) % npixels;
    ticks = 0;
  }

  if (transition_length > transition_elapsed) {
    tickrate = initial_tickrate + (dtickrate * transition_elapsed);
    r = initial_r + (dr * transition_elapsed);
    g = initial_g + (dg * transition_elapsed);
    b = initial_b + (db * transition_elapsed);
    transition_elapsed += 1;
  }
}
