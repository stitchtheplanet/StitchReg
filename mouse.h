#pragma once

typedef struct {
  int clk;
  int data;
  int x;
  int y;
  unsigned char resolution;
  unsigned char sample;
} mouse;

mouse *mouse_new(int clk, int data);
void mouse_begin(mouse *m);
void mouse_update(mouse *m);