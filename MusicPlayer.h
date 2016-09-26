#pragma once
#include <Arduino.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

class MusicPlayer {
  Adafruit_VS1053_FilePlayer *musicPlayer;
public:
  MusicPlayer(int8_t cs, int8_t dcs, int8_t dreq, int8_t cardcs);
  boolean startPlayingFile(const char *trackname);
  void setVolume(uint8_t left, uint8_t right);
  void stopPlaying();
  void poll(unsigned long jiffies);			// Call this once per loop()
};
