#include "encoder.h"

#include <Arduino.h>

// Adapted from https://github.com/mo-thunderz/RotaryEncoder/tree/main

#define ENC_A 3
#define ENC_B 2

void read_encoder();

volatile int enc_counter = 0;

void encoder_begin() {
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), read_encoder, CHANGE);
}

int encoder_count() {
  int c = enc_counter;
  enc_counter = 0;
  return c;
}

void read_encoder() {
  static uint8_t old_AB = 3;
  static int8_t encval = 0;
  static const int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};

  old_AB <<=2;

  if (digitalRead(ENC_A)) old_AB |= 0x02;
  if (digitalRead(ENC_B)) old_AB |= 0x01;

  encval += enc_states[(old_AB & 0x0f)];

  if (encval > 3) {
    enc_counter++;
    encval = 0;
  } else if (encval < -3) {
    enc_counter--;
    encval = 0;
  }
}