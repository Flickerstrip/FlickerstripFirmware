// vim:ts=2 sw=2:
#include "LEDStrip.h"

LEDStrip::LEDStrip() {
  this->length = 1;
  this->ledBuffer = NULL;
  this->start = 0;
  this->end = 1;
  this->reverse = false;
}

void LEDStrip::begin(const uint8_t pin) {
  this->ledBuffer = new CRGB[this->length];

  this->controller = &FastLED.addLeds<WS2812B, LED_STRIP, GRB>(this->ledBuffer, this->length);
}

void LEDStrip::setLength(int length) {
  this->clear();
  this->length = length;
  if (this->ledBuffer != NULL) {
    delete [] this->ledBuffer;
  }
  this->clear();
}

void LEDStrip::setStart(int start) {
  this->clear();
  this->start = start;
}

void LEDStrip::setEnd(int end) {
  this->clear();
  this->end = end;
}

void LEDStrip::setReverse(bool reverse) {
  this->reverse = reverse;
}

int LEDStrip::getLength() {
  return this->length;
}

void LEDStrip::clear() {
  if (this->ledBuffer == NULL) return;
  for (int i=0; i<this->length; i++) {
    this->ledBuffer[i] = CRGB( 0,0,0 );
  }
}

void LEDStrip::setPixel(int i, byte r, byte g, byte b) {
  int index = i + this->start;
  if (index > this->end-1) return;

  if (this->reverse) {
    index = this->end - i - 1;
    if (index < this->start) return;
  }

  this->ledBuffer[index] = CRGB( r,g,b );
}

void LEDStrip::setBrightness(byte brightness) {
  this->brightness = brightness;
}

void LEDStrip::show() {
  this->controller->showLeds(this->brightness);
}
