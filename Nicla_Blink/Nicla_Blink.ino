#include "Nicla_System.h"

static bool enableCharge(uint8_t mA = 20, bool disable_ntc = true);

void setup() {
  nicla::begin();
  nicla::leds.begin();
  nicla::enableCharge(100);
}

void loop() {
  nicla::leds.setColor(red);
  delay(1000);
  nicla::leds.setColor(off);
  delay(1000);
  nicla::leds.setColor(yellow);
  delay(1000);
  nicla::leds.setColor(off);
  delay(1000);
  nicla::leds.setColor(green);
  delay(1000);
  nicla::leds.setColor(off);
  delay(1000);
  nicla::leds.setColor(cyan);
  delay(1000);
  nicla::leds.setColor(off);
  delay(1000);
  nicla::leds.setColor(blue);
  delay(1000);
  nicla::leds.setColor(off);
  delay(1000);
  nicla::leds.setColor(magenta);
  delay(1000);
  nicla::leds.setColor(off);
  delay(1000);
}
