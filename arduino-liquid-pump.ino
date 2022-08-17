#include "arduino-step-motors.cpp"
#include "arduino-system-configuration.cpp"


// -------------------------------------------------------------------
// Initialization

// Displacer & Base
StepDisplacer displacer_x(DISPLACER_X_STEP_PIN, DISPLACER_X_DIRECTION_PIN);
StepDisplacer displacer_y(DISPLACER_Y_STEP_PIN, DISPLACER_Y_DIRECTION_PIN);
StepDisplacer displacer_z(DISPLACER_Z_STEP_PIN, DISPLACER_Z_DIRECTION_PIN);

BaseController base_controller(&displacer_x, &displacer_y, &displacer_z);

// Pumps & Injectors
StepPump pump_1(INJECTOR_1_PUMP_STEP_PIN, INJECTOR_1_PUMP_DIRECTION_PIN);
StepPump pump_2(INJECTOR_2_PUMP_STEP_PIN, INJECTOR_2_PUMP_DIRECTION_PIN);
StepPump pump_3(INJECTOR_3_PUMP_STEP_PIN, INJECTOR_3_PUMP_DIRECTION_PIN);
StepPump pump_4(INJECTOR_4_PUMP_STEP_PIN, INJECTOR_4_PUMP_DIRECTION_PIN);

Injector injector_1(&pump_1, INJECTOR_1_LOCATION_X, INJECTOR_1_LOCATION_Y, INJECTOR_1_LOCATION_Z);
Injector injector_2(&pump_2, INJECTOR_2_LOCATION_X, INJECTOR_2_LOCATION_Y, INJECTOR_2_LOCATION_Z);
Injector injector_3(&pump_3, INJECTOR_3_LOCATION_X, INJECTOR_3_LOCATION_Y, INJECTOR_3_LOCATION_Z);
Injector injector_4(&pump_4, INJECTOR_4_LOCATION_X, INJECTOR_4_LOCATION_Y, INJECTOR_4_LOCATION_Z);

// System Controller
SystemController system_controller(INJECTING_STARTING_POINT_Z);


// -------------------------------------------------------------------
// Setup functions
void config_pins() {
  pinMode(DISPLACER_X_DIRECTION_PIN, OUTPUT);
  pinMode(DISPLACER_Y_DIRECTION_PIN, OUTPUT);
  pinMode(DISPLACER_Z_DIRECTION_PIN, OUTPUT);

  pinMode(DISPLACER_X_STEP_PIN, OUTPUT);
  pinMode(DISPLACER_Y_STEP_PIN, OUTPUT);
  pinMode(DISPLACER_Z_STEP_PIN, OUTPUT);

  pinMode(INJECTOR_1_PUMP_DIRECTION_PIN, OUTPUT);
  pinMode(INJECTOR_2_PUMP_DIRECTION_PIN, OUTPUT);
  pinMode(INJECTOR_3_PUMP_DIRECTION_PIN, OUTPUT);
  pinMode(INJECTOR_4_PUMP_DIRECTION_PIN, OUTPUT);

  pinMode(INJECTOR_1_PUMP_STEP_PIN, OUTPUT);
  pinMode(INJECTOR_2_PUMP_STEP_PIN, OUTPUT);
  pinMode(INJECTOR_3_PUMP_STEP_PIN, OUTPUT);
  pinMode(INJECTOR_4_PUMP_STEP_PIN, OUTPUT);
}

void config_system() {
  Injector *injectors[4] = { &injector_1, &injector_2, &injector_3, &injector_4 };

  system_controller.init_base(&base_controller);
  system_controller.init_injectors(injectors, 4);
}

// -------------------------------------------------------------------
// Setup
void setup() {
  config_pins();
  config_system();

  Serial.begin(9600);
}


// -------------------------------------------------------------------
// Test function
void test_1() {
  PumpingController controller(&displacer_x, &pump_1);
  controller.pump(1000, 50000);

  delay(1000);
}

// -------------------------------------------------------------------
// Run
void loop() {
  test_1();

  Serial.println("Hello Wordl");
}
