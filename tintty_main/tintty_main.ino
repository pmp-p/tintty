/**
 * TinTTY main sketch
 * by Nick Matantsev 2017
 *
 * Original reference: VT100 emulation code written by Martin K. Schroeder
 * and modified by Peter Scargill.
 */

#include <SPI.h>
#include <SoftwareSerial.h>

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

#include "tintty.h"

// using stock MCUFRIEND 2.4inch shield
MCUFRIEND_kbv tft;

#define ILI9341_WIDTH 240
#define ILI9341_HEIGHT 320

struct tintty_display ili9341_display = {
  ILI9341_WIDTH,
  ILI9341_HEIGHT,
  ILI9341_WIDTH / TINTTY_CHAR_WIDTH,
  ILI9341_HEIGHT / TINTTY_CHAR_HEIGHT,

  [=](int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
    tft.fillRect(x, y, w, h, color);
  },

  [=](int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pixels){
    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    tft.pushColors(pixels, w * h, 1);
  },

  [=](int16_t offset){
    tft.vertScroll(0, 320, offset);
  }
};

// input serial forwarder RX, TX (TX should not be used anyway)
// @todo separate out to be configuration-specific and replace with touchscreen
// SoftwareSerial inputSerial(9, 10);

// buffer to test various input sequences
char *test_buffer = "-- \e[1mTinTTY\e[m --\r\n";
uint8_t test_buffer_cursor = 0;

void setup() {
  Serial.begin(9600); // normal baud-rate
  // inputSerial.begin(9600); // normal baud-rate

  uint16_t tftID = tft.readID();
  tft.begin(tftID);

  tintty_run(
    [=](){
      // first peek from the test buffer
      char test_char = test_buffer[test_buffer_cursor];

      if (test_char) {
        tintty_idle(&ili9341_display);
        return test_char;
      }

      // fall back to normal blocking serial input
      while (Serial.available() < 1) {
        tintty_idle(&ili9341_display);
      }

      return (char)Serial.peek();
    },
    [=](){
      // process at least one idle loop to allow input to happen
      tintty_idle(&ili9341_display);
      input_idle();

      // first read from the test buffer
      char test_char = test_buffer[test_buffer_cursor];

      if (test_char) {
        tintty_idle(&ili9341_display);
        input_idle();

        test_buffer_cursor += 1;
        return test_char;
      }

      // fall back to normal blocking serial input
      while (Serial.available() < 1) {
        tintty_idle(&ili9341_display);
        input_idle();
      }

      return (char)Serial.read();
    },
    [=](char ch){ Serial.print(ch); },
    &ili9341_display
  );
}

void loop() {
}

void input_idle() {
  // if (inputSerial.available()) {
  //   Serial.write(inputSerial.read());
  // }
}
