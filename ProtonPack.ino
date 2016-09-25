#include <SPI.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#define LTCH 8
#define RED 9
#define GREEN 10
#define BLUE 11
#define DEBUG 13
#define TRIGGER 4

const byte powerColor[3] = {0xff, 0, 0};
const byte dispBright = 10;
unsigned long jiffies = 0;
Adafruit_7segment disp1;

void rgbPWM(byte r, byte g, byte b) {
  analogWrite(RED, 0xff - r);
  analogWrite(GREEN, 0xff - g);
  analogWrite(BLUE, 0xff - b);
}

void rgb(byte r, byte g, byte b) {
  SPI.transfer(b);
  SPI.transfer(g);
  SPI.transfer(r);
  digitalWrite(LTCH, HIGH);
  digitalWrite(LTCH, LOW);
}

void setup() {
  randomSeed(analogRead(12));
  
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(LSBFIRST);

  disp1 = Adafruit_7segment();
  disp1.begin(0x70);
  
  pinMode(LTCH, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(DEBUG, OUTPUT);
  pinMode(TRIGGER, INPUT_PULLUP);
}

// Cycle through colors, one spoke at a time.
// Since we can only control brightness by color component for all spokes,
// we can't do a fancier trick per-spoke.
// But this one isn't that bad, really.
bool doStartup() {
  static int count = 0;
  static byte cur[3] = {0, 0, 0};
  
  // Run this every 12 jiffies
  if (jiffies % 6 != 0) {
    return false;
  }
  
  int weight = 0;
  int pos = count % 8;
  int color = 6 - (count / 8);

  count += 1;

  for (int i = 0; i < 3; i += 1) {
    int bit = (color & (1 << i))?1:0;
    weight += bit;
    // Shift the current color in from the LSB to the MSB
    cur[i] = (cur[i] << 1) | bit;
  }
  rgb(cur[0], cur[1], cur[2]);
  rgbPWM(32 * weight, 32 * weight, 32 * weight);
  
  for (int i = 0; i < 5; i += 1) {
    disp1.writeDigitRaw(i, random(256));
  }
  disp1.setBrightness(random(16));
  disp1.writeDisplay();
  
  if ((color == 1) && (pos == 7)) {
    rgb(powerColor[0], powerColor[1], powerColor[2]);
    disp1.clear();
    disp1.printNumber(0xb00, HEX);
    disp1.setBrightness(dispBright);
    disp1.writeDisplay();
    return true;
  }
  
  return false;
}

// Pulse to an extreme, then back
bool pulse(byte initial, int pct) {
  static int prev = 0;
  static int state = 0;
  static int val = 0;
  int cur = (pct << 8) | initial;
  int newval = initial;
  
  // Reset if called with new values
  if (prev != cur) {
    state = 0;
    prev = cur;
  }

  switch (state) {
  case 0:
    state = 1;
    val = initial;
    break;
  case 1:
    val = (val * pct) / 100;
    if ((val <= 1) || (val >= 255)) {
      state = 2;
    }
    break;
  case 2:
    // discrete exponentiation, woo woo
    while ((newval * pct) / 100 != val) {
      newval = (newval * pct) / 100;
    }
    val = newval;
    if (val == initial) {
      state = 3;
    }
    break;
  case 3:
    state = 0;
    val = 0;
    return true;
  }
  
  newval = min(val, 255);
  rgbPWM(newval, newval, newval);
  return false;
}

bool glitch(int r, int g, int b) {
  static int state = 0;
  int i;
  
  if (jiffies % 5 != 0) {
    return false;
  }
  
  switch (state) {
  case 0:
    // pick a random bit and clear it
    i = random(8);
    r &= ~(1 << i);
    g &= ~(1 << i);
    b &= ~(1 << i);
    rgb(r, g, b);
    state = 1;
    break;
  case 1:
    rgb(r, g, b);
    state = 0;
    return true;
    break;
  }  
  
  return false;
}

void fire() {
  rgb(0, 0xff, 0xff);
  pulse(32, 160);
}

void fireDone() {
  rgb(powerColor[0], powerColor[1], powerColor[2]);
  rgbPWM(64, 64, 64);
}


int doPowered() {
  static int doing = 0;
  static float val1 = 584.2;
  static bool firing = false;
  bool trigger;
 
  trigger = (digitalRead(TRIGGER) == LOW);

  if (trigger) {
    firing = true;
    doing = 100;
  }  
  
  switch (doing) {
  case 0: // doing nothing
    if (jiffies % 200 == 0) {
      doing = 1; // pulse
    } else if (random(350) == 0) {
      doing = 2; // surge
    } else if (random(200) == 0) {
      doing = 3; // glitch
    }
    break;
  case 1:
    if (pulse(64, 80)) {
      doing = 0;
    }
    break;
  case 2:
    if (pulse(64, 120)) {
      doing = 0;
    }
    break;
  case 3:
    if (glitch(powerColor[0], powerColor[1], powerColor[2])) {
      doing = 0;
    }
    break;
  case 100:
    fire();
    if (! trigger) {
      doing = 101;
    }
    break;
  case 101:
    fireDone();
    doing = 0;
    break;
  default:
    doing = 0;
  }
  
  // screw around with the displays
  if (random(20) == 0) {
    val1 += (random(3) - 1) / 10.0;
    disp1.print(val1);
    disp1.setBrightness(dispBright);
    disp1.writeDisplay();
  } else if (random(150) == 0) {
    disp1.setBrightness(random(16));
    disp1.writeDisplay();
  } else if (random(150) == 0) {
    disp1.clear();
    disp1.writeDisplay();
  } else if (random(400) == 0) {
    int someNumber = random(9999);
    disp1.print(someNumber);
    disp1.writeDisplay();
  }
  
  
  return 1;
}

void flashDebug() {
  if (jiffies % 50 == 0) {
    int val = digitalRead(DEBUG);
    digitalWrite(DEBUG, (val==HIGH)?LOW:HIGH);
  }
}

void loop() {
  static int state = 0;

  // state machine
  // The delay is *outside* the state machine, you'll notice.
  // So don't call sleep in your state function.
  switch (state) {
  case 0:
    if (doStartup()) {
      state = 1;
    }
    break;
  case 1:
    state = doPowered();
    break;
  }
  
  flashDebug();

  delay(12);
  jiffies += 1;
}

