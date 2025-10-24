#include "encoder.h"

#include <Arduino.h>

Encoder *encoder_new(int clk, int data) {
  pinMode(clk, INPUT_PULLUP);
  pinMode(data, INPUT_PULLUP);

  Encoder *e = (Encoder*)malloc(sizeof(Encoder));
  e->last_clk = digitalRead(clk);
  e->clk_pin = clk;
  e->data_pin = data;
  return e;
}

enum EncDirection encoder_update(Encoder *e) {
  enum EncDirection d = None;

  // Read the current state of CLK
  e->current_clk = digitalRead(e->clk_pin);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (e->current_clk != e->last_clk && e->current_clk == 1) {
    // If the DT state is different than the CLK state then
    if (digitalRead(e->data_pin) != e->current_clk) {
      // Rotating CCW
      d = CCW;
    } else {
      // Rotating CW
      d = CW;
    }
  }

  // Remember last CLK state
  e->last_clk = e->current_clk;
  return d;
}