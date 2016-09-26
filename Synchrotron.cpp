#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Synchrotron.h"

#define brightness 255

Synchrotron::Synchrotron(uint16_t n, uint8_t p, neoPixelType t)
{
  pxl = new Adafruit_NeoPixel(n, p, t);
  npixels = n;
  cur = 0;
  pxl->begin();
  pxl->show();
  standby();
}

Synchrotron::standby() {
  tickrate = 12;
  ticks = 0;
  r = brightness;
  g = 0;
  b = 0;
}

Synchrotron::charge() {
  tickrate = 2;
  ticks = 0;
  r = brightness;
  g = brightness / 8;
  b = 0;
}

Synchrotron::fire() {
}

Synchrotron::discharge() {
}

Synchrotron::tick(unsigned long jiffies) {
  float adj = (float)ticks / (float)tickrate;
  byte raa = r * adj;
  byte gaa = g * adj;
  byte baa = b * adj;
  byte ra = r - raa;
  byte ga = g - gaa;
  byte ba = b - baa;

  pxl->clear();
  pxl->setPixelColor((cur + 1) % npixels, pxl->Color(raa, gaa, baa));
  for (int i = 0; i < 4; i += 1) {
    int div = 1 << (2*i);
    pxl->setPixelColor((cur + npixels - i) % npixels, pxl->Color(ra/div, ga/div, ba/div));
  }

  pxl->show();

  ticks += 1;
  if (ticks == tickrate) {
    ticks = 0;
    cur = (cur + 1) % npixels;
  }
}
