#include "arduino-step-motors.cpp"
#include "arduino-system-configuration.cpp"
#include "arduino-utils.hh"


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

  delay(300);
}

// -------------------------------------------------------------------
// Run
void execute() {
  static char input_str[256];
  static uint8_t indices[INJECTOR_COUNT] = {1, 2, 3, 4};
  static long volumes[INJECTOR_COUNT] = {0, 0, 0, 0};

  static bool volume_prompt = true;
  static bool index_prompt = false;

  static bool volume_input = false;
  static bool index_input = false;
  static bool continue_input = false;

  static bool execute_flag = false;

  // Ask for input
  if (volume_prompt) {
    volume_prompt = false;
    Serial.println("Please enter list of volumes to be injected. Ex: \"1000 2000 3000 4000\"");
    volume_input = true;
  }
  if (index_prompt) {
    index_prompt = false;
    Serial.println("Please enter order of injectors. Ex: \"0 1 2 3\"");
    index_input = true;
  }

  // Get input values for volumes
  if (volume_input) {
    int size = read_from_console(input_str, 255);
    if (size > 0) {
      string_to_numbers(input_str, 255, volumes, INJECTOR_COUNT);
      volume_input = false;
      index_prompt = true;
    }
  }
  if (index_input) {
    int size = read_from_console(input_str, 255);
    if (size > 0) {
      long temp[INJECTOR_COUNT];                                      // I don't know why type casting (long -> uint8_t) doesn't work on uno
      string_to_numbers(input_str, 255, temp, INJECTOR_COUNT);
      for (int i=0;i<INJECTOR_COUNT;++i)
        indices[i] = temp[i];
      index_input = false;
      execute_flag = true;
    }
  }
  if (continue_input) {
    int size = read_from_console(input_str, 255);
    if (size > 0) {
      if (input_str[0] == 'c') {
        continue_input = false;
        execute_flag = true;
      }
      else if (input_str[0] == 'n') {
        continue_input = false;
        volume_prompt = true;
      }
    }
  }

  // Run when input are full
  if (execute_flag) {
    Serial.println("System will start running with the following inputs of volumes & injector's order: ");
    Serial.print("(Volume,Index: ");
    for (int i=0;i<INJECTOR_COUNT;++i) {
      Serial.print(volumes[i]);
      Serial.print(",");
      Serial.print(indices[i]);
      Serial.print(" ");
    }
    Serial.println("");

    // Actual execution
    system_controller.inject(volumes, indices, INJECTOR_COUNT);
    //delay(1000);

    Serial.println("Execution done, please enter 'c' for continue with previous values, 'n' for entering new value");
    execute_flag = false;
    continue_input = true;
  }
}

void loop() {
//  test_1();

  execute();
}


