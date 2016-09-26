#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include "MusicPlayer.h"



MusicPlayer::MusicPlayer(int8_t cs, int8_t dcs, int8_t dreq, int8_t cardcs)
{
  musicPlayer = new Adafruit_VS1053_FilePlayer(cs, dcs, dreq, cardcs);
  musicPlayer->begin();
  musicPlayer->setVolume(20, 20); // lower = louder
  musicPlayer->sineTest(0x44, 500);
  SD.begin(cardcs);
}

void
MusicPlayer::setVolume(uint8_t left, uint8_t right)
{
  musicPlayer->setVolume(left, right);
}


boolean
MusicPlayer::startPlayingFile(const char *trackname)
{
  return musicPlayer->startPlayingFile(trackname);
}

boolean
MusicPlayer::isPlaying()
{
  return musicPlayer->playingMusic;
}

void
MusicPlayer::stopPlaying()
{
  musicPlayer->stopPlaying();
}

void
MusicPlayer::poll(unsigned long jiffies)
{
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
  
  if (musicPlayer->playingMusic && musicPlayer->readyForData()) {
    int bytesread = musicPlayer->currentTrack.read(musicPlayer->mp3buffer, VS1053_DATABUFFERLEN);
    if (bytesread == 0) {
      musicPlayer->playingMusic = false;
      musicPlayer->currentTrack.close();
    } else {
      musicPlayer->playData(musicPlayer->mp3buffer, bytesread);
    }
  }
}
