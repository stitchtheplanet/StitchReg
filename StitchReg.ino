#include <MCP4131.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "mouse.h"
#include "state.h"
#include "button.h"
#include "encoder.h"

// Juki TL Pedal resistances
// Without pressing the pedal it sits at 140k ohms and the machine needs to see
// this resistance first before it will work. In the stop position we set the
// digital pot to 30k and put 110k of resistance in series, giving us the 40k.
// When running, we apply a voltage to the transistor which bypasses the 110k
// of resistors. Then we can set the pot according to our needs. We need to take
// care when turning the transistor off that we don't inadvertently land in the
// 30k - 40k zone or the thread cutter will trigger.
//
// Speeds adjust from ~20k (slow) down to 0 ohms (fastest)
//
// State      Transistor    Chip    R
// Off        LOW           89      ~140k
// Cut        HIGH          76      ~40k
// Idle       HIGH          89      ~30k
// Slow       HIGH          102     ~20k
// Fastest    HIGH          128     ~0

// The encoder must be on pins 2 and 3 because it uses interrupts
#define ENC_CLK 2
#define ENC_DT 3

#define MOUSE_CLK 7
#define MOUSE_DATA 8

#define BTN_RUN 5
#define BTN_CUT 6
#define BTN_ENC 4

#define POT_CS 10
#define IDLE_CTRL 9

#define WIPER_IDLE 89
#define THREAD_CUT 76
#define SENSOR_DPI 2032.0 // Empirically measured, data sheet says 1600
#define STITCHES_MAX 25.0 // 1500 stitches/min = 25 stitches/sec

#define DEBOUNCE 5

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MCP4131 Potentiometer(POT_CS);
mouse *m;
State *state;

Button *runButton = button_new(BTN_RUN);
Button *encButton = button_new(BTN_ENC);
Button *cutButton = button_new(BTN_CUT);

Encoder *encoder;

const int cw = SSD1306_WHITE;
const int cb = SSD1306_BLACK;

const int NUM_READINGS = 25;
double smoother[NUM_READINGS];
int readIndex = 0;


void setup() {  
  Serial.begin(9600);

  state = state_new();

  // initialize the display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(2);
  display.setTextColor(cw);
  setDisplay();

  // The transistor that engages the idle resistor
  pinMode(IDLE_CTRL, OUTPUT);
  digitalWrite(IDLE_CTRL, LOW);

  // Initialize the potentiometer to 40k
  Potentiometer.writeWiper(WIPER_IDLE);
 
  m = mouse_new(MOUSE_CLK, MOUSE_DATA);
  mouse_begin(m);

  // set up the encoder
  encoder = encoder_new(ENC_CLK, ENC_DT);
  attachInterrupt(digitalPinToInterrupt(encoder->clk_pin), updateEncoder, CHANGE);
  setDisplay();
}

void loop() {
  button_update(encButton);
  button_update(runButton);
  button_update(cutButton);
  
  if (state->running && runButton->pressed) {
    stop();
    setDisplay();
    return;
  }

  // manual and baste modes are constant speeds, no need to recalculate
  if (state->running && ((state->mode == MANUAL) || (state->mode == BASTE))) {
    return;
  }

  if (state->running) {
    // if (lastPoll > 10) {
      // Poll rate of 10ms
      mouse_update(m);
      int x = m->x;
      int y = m->y;
      double distance = (sqrt(sq(abs(x)) + sq(abs(y)))) / SENSOR_DPI; // Distance moved, in inches
      double speed = (distance / m->t) * 1000; // Speed in inches per seconds
      double stitches = speed * current_speed(state); // Stitches needed to hit target SPI
      double percent = stitches / 25;

      // Smooth out the last NUM_READINGS percentages
      smoother[readIndex] = percent;
      readIndex = (readIndex + 1) % NUM_READINGS;
      
      double percent_total = 0.0;
      for (int i = 0; i < NUM_READINGS; i++) {
        percent_total += smoother[i];
      }
      double percent_avg = percent_total / NUM_READINGS;
      if (percent_avg > 1.0)
        percent_avg = 1.0;

      if (distance > 0.0) {
        // 23 and 105 are magic numbers here, they can be tweaked
        double pot = (percent_avg * 23) + 105.0;
        // lastPoll = 0;

        if (state->mode == PRECISE) {
          digitalWrite(IDLE_CTRL, HIGH);
        }

        Potentiometer.writeWiper(pot);
      } else {
        Potentiometer.writeWiper((state->cruise_idle / 100.0) * 23.0 + 105);
        if (state->mode == PRECISE) {
          Potentiometer.writeWiper(WIPER_IDLE);
          digitalWrite(IDLE_CTRL, LOW);
        }
      }
    // }

    delay(10);

    return;
  }  

  if (runButton->pressed) {
    start(WIPER_IDLE);
    // Constant speeds only need to be turned on once so do that here 
    // rather than writing to the pot every loop.
    switch (state->mode) {
      case MANUAL:
        Potentiometer.writeWiper((state->manual_speed / 100.0) * 23.0 + 105);
        break;
      case BASTE:
        Potentiometer.writeWiper(state_baste_wiper(state));
        break;
      default:
        break;
    }
    setDisplay();
    return;
  }

  // Check for thread cutter
  if (cutButton->pressed) {
    start(THREAD_CUT);
    delay(5);
    stop();
  }

dbuttons:
  if (encButton->pressed) {
    state_toggle_selected(state);
    setDisplay();
  }

  if (state->dirty) {
    state->dirty = 0;
    setDisplay();
  }
}

void setDisplay() {
  // Row 0, mode row
  display.clearDisplay();

  display.drawCircle(116, 11, 4, cw);
  if (state->running)
    display.fillCircle(116, 11, 4, cw);

  display.setCursor(3, 3);
  display.print(state_mode(state));
  if (state->dsp_row == 0) {
    if (state->dsp_selected) {
      display.drawRect(0, 0, mode_box_width(state->mode), 20, cw);
    } 

    display.drawFastVLine(0, 0, 20, cw);
  } 

  // Row 1
  //   Manual mode - manual speed, 1 row
  //   Cruise mode - cruise speed, cruise idle, 2 rows
  //   Precise mode - precise speed, 1 row
  //   Baste mode - baste speed, 1 row
  int rw = 60;
  display.setCursor(3, 25);
  switch (state->mode) {
    case MANUAL:
      display.print(state->manual_speed);
      display.print("%");
      if (state->manual_speed < 10)
        rw = 28;
      else if (state->manual_speed == 100)
        rw = 52;
      else
        rw = 40;
      break;
    case CRUISE:
      display.print(state->cruise_speed);
      display.print(" SPI");
      if (state->cruise_speed < 10)
        rw = 64;
      else
        rw = 76;
      break;
    case PRECISE:
      display.print(state->precise_speed);
      display.print(" SPI");
      if (state->precise_speed < 10)
        rw = 64;
      else
        rw = 76;
      break;
    case BASTE:
      display.print(state_baste(state));
      if (state->baste_speed == MEDIUM)
        rw = 76;
      else
        rw = 65;
      break;
  }
  if (state->dsp_row == 1) {
    if (state->dsp_selected) {
      display.drawRect(0, 22, rw, 20, cw);
    }
    display.drawFastVLine(0, 22, 20, cw);
  }

  // Row 2
  if (state->mode == CRUISE) {
    display.setCursor(3, 47);
    display.print(state->cruise_idle);
    display.print("% IDLE");
    if (state->cruise_idle < 10)
      rw = 88;
    else
      rw = 100;
    if (state->dsp_row == 2) {
      if (state->dsp_selected) {
        display.drawRect(0, 44, rw, 20, cw);
      }
      display.drawFastVLine(0, 44, 20, cw);
    }
  }

  display.display();
}

void updateEncoder() {
  EncDirection d = encoder_update(encoder);
  if (d == None) {
    return;
  }

  state->dirty = 1;

  if (d == CW) {
    if (state->dsp_selected) {
      switch (state->dsp_row) {
        case 0: // Mode
          state_next_mode(state);
          break;
        case 1:
          state_inc_speed(state);
          break;
        case 2: // Only happens in cruise
          state_inc_idle(state);
          break;
      }
    } else {
      state_next_row(state);
    }
    return;
  }
  
  if (state->dsp_selected) {
    switch (state->dsp_row) {
      case 0: // Mode
        state_prev_mode(state);
        break;
      case 1:
        state_dec_speed(state);
        break;
      case 2: // Only happens in cruise
        state_dec_idle(state);
        break;
    }
  } else {
    state_prev_row(state);
  }
}

void stop() {
  Potentiometer.writeWiper(WIPER_IDLE);
  digitalWrite(IDLE_CTRL, LOW);
  state->running = false;
}

void start(int speed) {
  Potentiometer.writeWiper(speed);
  digitalWrite(IDLE_CTRL, HIGH);
  state->running = true;
  xd = 0;
}

int mode_box_width(enum Mode m) {
  switch (m) {
    case MANUAL:
      return 74;
    case CRUISE:
      return 76;
    case PRECISE:
      return 88;
    case BASTE:
      return 65;
  }
  return 0;
}
