#include <MCP4131.h>
#include "PS2MouseHandler.h"

// Desired features:
// Stitch length mode
// Speed mode
// Cut Thread button
// Start/Stop button
// Overspeed indicator
// Edge warning (might need a pi/storage for this kind of thing?)
// Idle speed (5% to 100%)
// Measurement (need a display)

#define RELAY 2
#define MOUSE_DATA 5
#define MOUSE_CLOCK 6
#define CHIP_SELECT 10
#define BUZZER 8
#define SWITCH 9

#define WIPER_IDLE 110
#define SENSOR_DPI 1600.0
#define STITCHES_MAX 25.0 // 1500 stitches/min = 25 stitches/sec


const int NUM_READINGS = 5;
float pot_settings[NUM_READINGS];
float dot_readings[NUM_READINGS];

int readIndex = 0;

bool running = false;

PS2MouseHandler mouse(MOUSE_CLOCK, MOUSE_DATA, PS2_MOUSE_REMOTE);
MCP4131 Potentiometer(CHIP_SELECT);

void setup() {
  Serial.begin(9600);

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  pinMode(SWITCH, INPUT);

  if (mouse.initialise() != 0) {
    Serial.println("mouse error");
  }

  mouse.enable_data_reporting();
  mouse.set_remote_mode();

  for (int i = 0; i < NUM_READINGS; i++) {
    pot_settings[i] = 0;
  }

  // initialize this to 40k
  Potentiometer.writeWiper(WIPER_IDLE);
}

void loop() {
  if (digitalRead(SWITCH) == 0) {
    digitalWrite(RELAY, LOW);
    delay(10);
    return;
  }
  
  digitalWrite(RELAY, HIGH);

  mouse.get_data();
  int x = mouse.x_movement();
  int y = mouse.y_movement();

  float dots = sqrt(sq(abs(x)) + sq(abs(y)));
  // float pot = calculate_pot(dots);
  float pot = calculate_log(dots);
  
  pot_settings[readIndex] = pot;
  dot_readings[readIndex] = dots;

  readIndex = (readIndex + 1) % NUM_READINGS;

  float dot_total = 0.0;
  for (int i = 0; i < NUM_READINGS; i++) {
    dot_total += dot_readings[i];
  }
  float dot_avg = dot_total / NUM_READINGS;

  float pot_total = 0.0;
  for (int i = 0; i < NUM_READINGS; i++) {
    pot_total += pot_settings[i];
  }
  float pot_avg = pot_total / NUM_READINGS;
  
  if (dot_avg > 0.0) {
    if (pot > 127.0) {
     tone(BUZZER, 523, 250);
      Serial.println("OVER SPEED");
    }
    // pot = constrain(pot, WIPER_IDLE, 127);
    pot = bucket(pot);
    Serial.println(pot);
    Potentiometer.writeWiper(pot);
  } else {
    Potentiometer.writeWiper(WIPER_IDLE);
  }

  // Faster polling means higher resolution in movement detection.
  // The mouse reports a number -127, 127 that it has moved since the last time it was polled. 
  // If it's polled too slowly it will start reporting 127s for even slow movements.
  delay(10);
}

float calculate_pot(float dots) {
  // Target stitch speed: 11 stitches/sec
  // Juki TL max speed: 1500 stitches/min
  // Sensor resolution: 1600 dpi
  // Polling rate: 10ms
  float distance = dots / SENSOR_DPI;
  float speed = distance * 100.0;  // How much we're moving per second with a polling rate of 10ms
  float stitches_needed = speed * 11.0;
  float percentage = stitches_needed / STITCHES_MAX;
  // pot range 100 - 127, which is really 0 - 27 + 100
  // TODO this is too linear, we need to ramp up quickly and ramp down slowly

  return (percentage * 27) + 100.0;
}

float calculate_log(float dots) {
  // Ramp up and down instead of linear, seems smoother.
  // 40 works when the machine is limited to ~50%, but I'd rather have the machine at 100%.
  return 100.0 + (27 * ((log(dots + 1) / log(40))));
}

float bucket(float speed) {
  // Put the speed into one of 4 buckets so it's not shifting all over the place
  // This seems to make it smoother, but is maybe less adjustable?
  float bucket_width = (127 - WIPER_IDLE) / 5;
  int multiplier;

  if (speed < (WIPER_IDLE + bucket_width)) {
    multiplier = 1;
  } else if (speed < (WIPER_IDLE + (bucket_width * 2))) {
    multiplier = 2;
  } else if (speed < (WIPER_IDLE + (bucket_width * 3))) {
    multiplier = 3;
  } else {
    multiplier = 4;
  }
  return WIPER_IDLE + (bucket_width * multiplier);
}
