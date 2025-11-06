#pragma once

// Some simple stuff to debounce the buttons

#include <stdlib.h>

typedef struct {
  int pin;
  int state;
  int lastState;
  int pressed;
  unsigned long lastT;
  unsigned long delay;
} Button;

Button *button_new(int pin);
void button_update(Button *b);