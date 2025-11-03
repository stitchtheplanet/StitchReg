#pragma once

enum EncDirection {
  CW,
  CCW,
  None
};

typedef struct {
  int clk_pin;
  int data_pin;
  int current_clk;
  int last_clk;
} Encoder;

Encoder *encoder_new(int clk, int data);
enum EncDirection encoder_update(Encoder *e);