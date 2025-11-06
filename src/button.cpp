#include "button.h"
#include <Arduino.h>

Button *button_new(int pin) {
  Button *b = (Button*)malloc(sizeof(Button));
  b->pin = pin;
  b->state = 0;
  b->lastState = HIGH;
  b->pressed = 0;
  b->lastT = millis();
  b->delay = 50;

  pinMode(pin, INPUT_PULLUP);
  
  return b;
}

void button_update(Button *b) {
  b->pressed = 0;
  int reading = digitalRead(b->pin);
  if (reading != b->lastState) {
    b->lastT = millis();
  }

  if ((millis() - b->lastT) > b->delay) {
    if (reading != b->state) {
      b->state = reading;

      if (b->state == LOW) {
        b->pressed = 1;
      }
    }
  }
  b->lastState = reading;
}