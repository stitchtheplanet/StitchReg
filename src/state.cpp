#include "state.h"

enum Mode next_mode(enum Mode m);
enum Mode prev_mode(enum Mode m);
enum Baste next_baste(enum Baste b);
enum Baste prev_baste(enum Baste b);
int mode_rows(enum Mode m);

int clamp(int value, int inc, int min, int max);

State *state_new() {
  State *s = (State*)malloc(sizeof(State));
  s->mode = CRUISE;
  s->manual_speed = 50;
  s->cruise_speed = 11;
  s->cruise_idle = 5;
  s->precise_speed = 11;
  s->baste_speed = MEDIUM;
  s->running = 0;
  s->dsp_row = 0;
  s->dsp_selected = 0;
  s->dsp_rows = 2;
  s->dirty = 0;
  return s;
}

int speed_target(State *s) {
  int speed = 0;
  switch (s->mode) {
    case CRUISE:
      speed = s->cruise_speed;
      break;
    case PRECISE:
      speed = s->precise_speed;
      break;
    case MANUAL:
    case BASTE:
      break;
  }
  return speed;
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
  int wiper = 0;
  switch (s->baste_speed) {
    case SMALL:
      wiper = 107;
      break;
    case MEDIUM:
      wiper = 106;
      break;
    case LARGE:
      wiper = 105;
  }
  return wiper;
}

enum Mode next_mode(enum Mode m) {
  enum Mode mode;
  switch (m) {
    case MANUAL:
      mode = CRUISE;
      break;
    case CRUISE:
      mode = PRECISE;
      break;
    case PRECISE:
      mode = BASTE;
      break;
    case BASTE:
      mode = MANUAL;
      break;
  }
  return mode;
}

enum Mode prev_mode(enum Mode m) {
  enum Mode mode;
  switch (m) {
    case BASTE:
      mode = PRECISE;
      break;
    case PRECISE:
      mode = CRUISE;
      break;
    case CRUISE:
      mode = MANUAL;
      break;
    case MANUAL:
      mode = BASTE;
      break;
  }
  return mode;
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
  return "";
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
  enum Baste next;
  switch (b) {
    case SMALL:
      next = MEDIUM;
      break;
    case MEDIUM:
      next = LARGE;
      break;
    case LARGE:
      next = SMALL;
      break;
  }
  return next;
}

enum Baste prev_baste(enum Baste b) {
  enum Baste prev;
    switch(b) {
      case SMALL:
        prev = LARGE;
        break;
      case MEDIUM:
        prev = SMALL;
        break;
      case LARGE:
        prev = MEDIUM;
        break;
    }
    return prev;
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
  return "";
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


