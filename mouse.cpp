#include "mouse.h"
#include <Arduino.h>
#include <stdlib.h>

int send(mouse *m, unsigned char data);
unsigned char read(mouse *m);
void pull_low(int pin);
void pull_high(int pin);
int wait_for(int pin, int state);

mouse *mouse_new(int clk, int data) {
  mouse *m = (mouse*)malloc(sizeof(mouse));
  m->clk = clk;
  m->data = data;
  return m;
}

void mouse_begin(mouse *m) {
  unsigned char b;
  // idle
  pull_high(m->clk);
  pull_high(m->data);
 
  send(m, 0xFF); // reset
  // The self test takes longer than our normal time out, so wait for that.
  while (digitalRead(m->clk)) {}
  b = read(m); // reset ACK
  b = read(m); // self test results
  b = read(m); // device id

  // Disable data reporting
  send(m, 0xF4);
  b = read(m); // ACK

  // We want remote mode
  send(m, 0xF0);
  b = read(m); // ACK

  // Set resolution to 8 counts/mm
  send(m, 0xE8);
  read(m); // ACK
  send(m, 0x03);
  read(m); // ACK

  // Set sample rate to 200/s
  send(m, 0xF3);
  read(m); // ACK
  send(m, 0xC8);
  read(m); // ACK

  // Get Status
  send(m, 0xE9);
  read(m); // ACK
  read(m); // statuses
  m->resolution = read(m);
  m->sample = read(m);
}

void mouse_update(mouse *m) {
  unsigned long start = millis();
  unsigned char b = 0;
  unsigned char ack = 0;

  send(m, 0xEB); // read data
  ack = read(m); // ACK

  b = read(m);
  m->x = (0x10 & b) ? 0xFF00 | read(m) : read(m);
  m->y = (0x20 & b) ? 0xFF00 | read(m) : read(m);
  m->t = start - m->last;
  m->last = start;
}

int send(mouse *m, unsigned char data) {
  unsigned char parity = 1;

  pull_high(m->data);
  pull_high(m->clk);
  pull_low(m->clk);
  delayMicroseconds(100);
  pull_low(m->data);
  delayMicroseconds(10);
  pull_high(m->clk);

  for (int i = 0; i < 8; i++) {
    wait_for(m->clk, LOW);
    if (data & 0x01) {
      pull_high(m->data);
    } else {
      pull_low(m->data);
    }
    parity += data & 0x01;
    data = data >> 1;
    wait_for(m->clk, HIGH);
  }

  wait_for(m->clk, LOW);

  if (parity & 0x01) {
    pull_high(m->data);
  } else {
    pull_low(m->data);
  }

  wait_for(m->clk, HIGH);
  wait_for(m->clk, LOW);
  pull_high(m->data);
  wait_for(m->clk, HIGH);

  pull_high(m->data);
  wait_for(m->data, LOW);
  wait_for(m->clk, LOW);
  wait_for(m->clk, HIGH);
  wait_for(m->data, HIGH);

  return 0;
}

unsigned char read(mouse *m) {
  unsigned char data = 0;
  unsigned char parity;

  wait_for(m->clk, LOW);
  wait_for(m->clk, HIGH);

  // 8 bits of data
  for (int i = 0; i < 8; i++) {
    digitalWrite(10, HIGH);
    wait_for(m->clk, LOW);
    digitalWrite(10, LOW);
  
    int d = digitalRead(m->data);
    data = data >> 1;
    data += d * 0x80;
    // parity += data & 0x1;
    wait_for(m->clk, HIGH);
  }
  // validate parity? nah, wgaf
  wait_for(m->clk, LOW);
  wait_for(m->clk, HIGH);

  // stop bit, skipped
  wait_for(m->clk, LOW);
  wait_for(m->clk, HIGH);

  return data;
}

void pull_low(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void pull_high(int pin) {
  pinMode(pin, INPUT_PULLUP);
}

int wait_for(int pin, int state) {
  unsigned long start = millis();
  while (digitalRead(pin) != state) {
    if (millis() - start > 100) {
      return -1;
    }
  }
  return 0;
}

