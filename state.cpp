#include "state.h"

enum Mode next_mode(enum Mode m);
enum Mode prev_mode(enum Mode m);
enum Baste next_baste(enum Baste b);
enum Baste prev_baste(enum Baste b);
int mode_rows(enum Mode m);

int clamp(int value, int inc, int min, int max);

State *state_new() {
  State *s = (State*)malloc(sizeof(State));
  s->mode = PRECISE;
  s->manual_speed = 50;
  s->cruise_speed = 11;
  s->cruise_idle = 5;
  s->precise_speed = 11;
  s->baste_speed = MEDIUM;
  s->running = 0;
  s->dsp_row = 0;
  s->dsp_selected = 0;
  s->dsp_rows = 1;
  s->dirty = 0;
  return s;
}

int current_speed(State *s) {
  switch (s->mode) {
    case CRUISE:
      return s->cruise_speed;
    case PRECISE:
      return s->precise_speed;
  }
}

void state_next_mode(State *s) {
  s->mode = next_mode(s->mode);
  s->dsp_rows = mode_rows(s->mode);
}

void state_prev_mode(State *s) {
  s->mode = prev_mode(s->mode);
  s->dsp_rows = mode_rows(s->mode);
}

void state_inc_speed(State *s) {
  switch (s->mode) {
    case MANUAL:
      s->manual_speed = clamp(s->manual_speed, 5, 5, 100);
      break;
    case CRUISE:
      s->cruise_speed = clamp(s->cruise_speed, 1, 6, 16);
      break;
    case PRECISE:
      s->precise_speed = clamp(s->precise_speed, 1, 6, 16);
      break;
    case BASTE:
      s->baste_speed = next_baste(s->baste_speed);
      break;
  }
}

void state_dec_speed(State *s) {
  switch (s->mode) {
    case MANUAL:
      s->manual_speed = clamp(s->manual_speed, -5, 5, 100);
      break;
    case CRUISE:
      s->cruise_speed = clamp(s->cruise_speed, -1, 6, 16);
      break;
    case PRECISE:
      s->precise_speed = clamp(s->precise_speed, -1, 6, 16);
      break;
    case BASTE:
      s->baste_speed = prev_baste(s->baste_speed);
      break;
  }
}

void state_inc_idle(State *s) {
  s->cruise_idle = clamp(s->cruise_idle, 5, 5, 55);
}

void state_dec_idle(State *s) {
  s->cruise_idle = clamp(s->cruise_idle, -5, 5, 55);
}

int state_baste_wiper(State *s) {
  switch (s->baste_speed) {
    case SMALL:
      return 107;
    case MEDIUM:
      return 106;
    case LARGE:
      return 105;
  }
}

enum Mode next_mode(enum Mode m) {
  switch (m) {
    case MANUAL:
      return CRUISE;
    case CRUISE:
      return PRECISE;
    case PRECISE:
      return BASTE;
    case BASTE:
      return MANUAL;
  }
}

enum Mode prev_mode(enum Mode m) {
  switch (m) {
    case BASTE:
      return PRECISE;
    case PRECISE:
      return CRUISE;
    case CRUISE:
      return MANUAL;
    case MANUAL:
      return BASTE;
  }
}

const char *state_mode(State *s) {
  switch (s->mode) {
    case MANUAL:
      return "Manual";
    case CRUISE:
      return "Cruise";
    case PRECISE:
      return "Precise";
    case BASTE:
      return "Baste";
  }
}

void state_toggle_selected(State *s) {
  s->dsp_selected = !s->dsp_selected;
}

void state_next_row(State *s) {
  s->dsp_row++;
  if (s->dsp_row > s->dsp_rows) {
    s->dsp_row = 0;
  }
}

void state_prev_row(State *s) {
  s->dsp_row--;
  if (s->dsp_row < 0) {
    s->dsp_row = s->dsp_rows;
  }
}

enum Baste next_baste(enum Baste b) {
  switch (b) {
    case SMALL:
      return MEDIUM;
    case MEDIUM:
      return LARGE;
    case LARGE:
      return SMALL;
  }
}

enum Baste prev_baste(enum Baste b) {
    switch(b) {
      case SMALL:
        return LARGE;
      case MEDIUM:
        return SMALL;
      case LARGE:
        return MEDIUM;
    }
}

const char *state_baste(State *s) {
  switch (s->baste_speed) {
    case SMALL:
      return "SMALL";
    case MEDIUM:
      return "MEDIUM";
    case LARGE:
      return "LARGE";
  }
}

int clamp(int value, int inc, int min, int max) {
  value = value + inc;
  if (value > max) {
    return max;
  }
  if (value < min) {
    return min;
  }
  return value;
}

int mode_rows(enum Mode m) {
  if (m == CRUISE) {
    return 2;
  }
  return 1;
}


