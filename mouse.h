#pragma once
#include <stdint.h>

typedef struct {
  int clk;
  int data;
  int16_t x;
  int16_t y;
  unsigned char resolution;
  unsigned char sample;
  unsigned long t;
  unsigned long last;
} Mouse;

Mouse *mouse_new(int clk, int data);
void mouse_begin(Mouse *m);
void mouse_update(Mouse *m);