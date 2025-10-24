#ifndef STATE_H
#define STATE_H

#include <stdlib.h>

enum Mode {
  MANUAL,
  CRUISE,
  PRECISE,
  BASTE
};

enum Baste {
  SMALL,
  MEDIUM,
  LARGE
};

typedef struct {
  enum Mode mode;
  int manual_speed;
  int cruise_speed;
  int cruise_idle;
  int precise_speed;
  enum Baste baste_speed;
  int running;
  int dsp_row;
  int dsp_selected;
  int dsp_rows;
  int dirty;
} State;

State *state_new();

int current_speed(State *s);
void state_next_mode(State *s);
void state_prev_mode(State *s);
void state_inc_speed(State *s);
void state_dec_speed(State *s);
void state_inc_idle(State *s);
void state_dec_idle(State *s);

int state_baste_wiper(State *s);

const char *state_mode(State *s);
const char *state_baste(State *s);

void state_toggle_selected(State *s);
void state_next_row(State *s);
void state_prev_row(State *s);


#endif