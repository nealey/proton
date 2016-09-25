// Proton Pack with NeoPixels
// Boy howdy do these make everything easy

#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_VS1053.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>

#define DEBUG 12

// Music Player object
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)
#define CARDCS 4 // Card chip select
#define DREQ 1 // VS1053 Data request (an interrupt pin)
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

// NeoPixel: so cool
#define SYNCHROTRON_PIN 5
#define SYNCHROTRON_PIXELS 24 // I'm using the middle-sized NeoPixel ring
Adafruit_NeoPixel synchrotron = Adafruit_NeoPixel(SYNCHROTRON_PIXELS, SYNCHROTRON_PIN, NEO_GRB | NEO_KHZ800);

// 7-segment displays
Adafruit_7segment disp1 = Adafruit_7segment();

// Inputs
#define TRIGGER 8

// Nominal brightness
#define brightness 64

const byte dispBright = 10;
unsigned long jiffies = 0;

void rgbPWM(byte r, byte g, byte b) {
  // XXX: do this
}

void rgb(byte r, byte g, byte b) {
  for (int i = 0; i < SYNCHROTRON_PIXELS; i += 1) {
    synchrotron.setPixelColor(i, synchrotron.Color(r, g, b));
  }
  synchrotron.show();
}

void setup() {
  randomSeed(analogRead(12));

  // synchrotron
  synchrotron.begin();
  synchrotron.show(); // Turn everything off

  // inputs
  pinMode(TRIGGER, INPUT_PULLUP);

  // music player, this sets up SPI for us
  SD.begin(CARDCS);
  musicPlayer.begin();
  musicPlayer.setVolume(20, 20); // lower = louder
  // We don't set useInterrupt, since we do our own polling for smoother operations

  // 7-segment displays.
  // These also use SPI, in i2c mode.
  // Since the music player has a CS line,
  // and we're unlikely to send the right i2c command to the 7-segment to wake it up,
  // it's okay to use the same SPI bus for both.
  disp1.begin(0x70);
}

// Synchrotron needs to "spin up"
// We start slow, with red, then work our way through the rainbow to blue
bool charge() {
  static uint32_t count = 0;
  static int every = 9;
  static int reps = 0;
  uint32_t color_count;
  byte r, g, b;
  static int whichout = 0;

  // Play startup sound at the start
  if (count == 0) {
    musicPlayer.startPlayingFile("track001.mp3");
  }

  // Make the animation play out a little more slowly,
  // while still allowing a nice fast rotation
  color_count = count / 4;

  // Give the illusion of something spinning up
  if (every == 1) {
    whichout = (whichout + 1) % SYNCHROTRON_PIXELS;
  } else if (count % every == 0) {
    whichout = (whichout + 1) % SYNCHROTRON_PIXELS;
    reps += 1;
    if (reps == 20 - every) {
      every -= 1;
      reps = 0;
    }
  }

  // Start at blue, go through hue to red
  switch (color_count / brightness) {
  case 0:
    r = color_count % brightness;
    g = 0;
    b = 0;
    break;
  case 1:
    r = brightness - (color_count % brightness) - 1;
    g = color_count % brightness;
    b = 0;
    break;
  case 2:
    r = 0;
    g = brightness - (color_count % brightness) - 1;
    b = color_count % brightness;
    break;
  default:
    rgb(brightness, 0, 0);
    return true;
  }

  // Set 'em up pixels
  for (int i = 0; i < SYNCHROTRON_PIXELS; i += 1) {
    if (whichout == i) {
      synchrotron.setPixelColor(i, 0);
    } else if ((whichout == (i+1) % SYNCHROTRON_PIXELS) || ((whichout+1) % SYNCHROTRON_PIXELS == i)) {
      synchrotron.setPixelColor(i, synchrotron.Color(r/4, g/4, b/4));
    } else {
      synchrotron.setPixelColor(i, synchrotron.Color(r, g, b));
    }
  }
  synchrotron.show();

  disp1.clear();
  disp1.printNumber(0xb00, HEX);
  disp1.setBrightness(dispBright);
  disp1.writeDisplay();

  count += 1;

  return false;
}

// Do a sort of mirrored KITT effect
bool kitt() {
  static int count = 0;
  int out = count % (SYNCHROTRON_PIXELS/2);

  if (jiffies % 12 != 0) {
    return false;
  }

  for (int i = 0; i < SYNCHROTRON_PIXELS; i += 1) {
    int pixnum = (SYNCHROTRON_PIXELS/2) - abs(i - (SYNCHROTRON_PIXELS / 2));
    int intensity;

    if (count < SYNCHROTRON_PIXELS/2) {
      intensity = 100;
      if (pixnum == out) {
       intensity = 50;
      } else if (pixnum < out) {
       intensity = 10;
      }
    } else {
      intensity = 10;
      if (pixnum == out) {
        intensity = 50;
      } else if (pixnum < out) {
        intensity = 100;
      }
    }
    synchrotron.setPixelColor(i, synchrotron.Color(brightness * intensity / 100, 0, 0));
  }
  synchrotron.show();

  count += 1;
  if (count > SYNCHROTRON_PIXELS) {
    rgb(brightness, 0, 0);
    count = 0;
    return true;
  }
  return false;
}

bool glitch(int r, int g, int b) {
  static int state = 0;
  int i;
  
  if (jiffies % 10 != 0) {
    return false;
  }
  
  switch (state) {
  case 0:
    // glitch to a random color
    r = random(brightness / 6);
    g = random(brightness / 6);
    b = random(brightness / 6);
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
  rgb(0, brightness, brightness);
}

void fireDone() {
  rgb(brightness, 0, 0);
}

void flashDebug() {
  if (jiffies % 50 == 0) {
    int val = digitalRead(DEBUG);
    digitalWrite(DEBUG, (val==HIGH)?LOW:HIGH);
  }
}

void tick() {
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
    if (jiffies % 300 == 0) {
      doing = 1; // KITT
    } else if (random(350) == 0) {
      doing = 2; // surge
    } else if (random(400) == 0) {
      doing = 3; // glitch
    }
    break;
  case 1:
    if (kitt()) {
      doing = 0;
    }
    break;
  case 2:
    doing = 0;
    break;
  case 3:
    if (glitch(brightness, 0, 0)) {
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
    
  flashDebug();
}

void loop() {
  // 6 seems to be about what my overly-critical brain needs to buffer out
  // any music player delays so that they're unnoticeable
  unsigned long new_jiffies = millis() / 6;

  if (new_jiffies > jiffies) {
    jiffies = new_jiffies;
    tick();
  }

  /* Cleverness ensues
   * 
   * The Adafruit library is written to let you go off and do whatever you need,
   * hooking into an interrupt to sort of act like a multitasking operating system,
   * interrupting your program periodically.
   * 
   * That's smart, since it makes it easy to use,
   * but we want this to be responsive, and can't handle something barging in and taking up lots of time:
   * it makes things look really uneven as our display code pauses to fill the buffer.
   * Fortunately, we don't have to fill the entire buffer at once, we can trickle data in.
   * That's what this does. 
   * 
   * Since the entire program is polling, without ever calling delay,
   * and hopefully doing what needs to be done quickly,
   * we check to see if the music chip wants more data.
   * If it does, we give it one chunk, and only one chunk,
   * rather than filling its buffer back up completely.
   * 
   * There is still some weirdness with this loop,
   * possibly because the SPI routines are masking interrupts used to increment millis.
   * But it's remarkably more fluid than the other way.
   */
  
  if (musicPlayer.playingMusic && musicPlayer.readyForData()) {
    int bytesread = musicPlayer.currentTrack.read(musicPlayer.mp3buffer, VS1053_DATABUFFERLEN);
    if (bytesread == 0) {
      musicPlayer.playingMusic = false;
      musicPlayer.currentTrack.close();
    } else {
      musicPlayer.playData(musicPlayer.mp3buffer, bytesread);
    }
  }
}

