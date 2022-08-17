#include <Arduino.h>

// ---------------------------- MOtor class ---------------------------------
#define STEP_MOTOR_DEFAULT_CYCLE 1600
#define STEP_MOTOR_DEFAULT_SPEED 500
#define STEP_MOTOR_MAX_SPEED 1000
#define STEP_MOTOR_MIN_SPEED 150
#define STEP_MOTOR_MAX_DELAY 1150

#define STEP_DISPLACER_DEFAULT_DISTANCE_COEF 1000       // Means 1 full cycle can move 1000 micrometres
#define STEP_PUMP_DEFAULT_VOLUME_COEF 200000            // Means 1 full cycle can pump 200000 micro_ccs

#define PUMPING_DISTANCE_VOLUME_RATIO 1.0               // Means in the output pipe, 1000 micrometres long is corresponding to 1000 micro_ccs of volume

#define SPEED_TO_DELAY(speed) (STEP_MOTOR_MAX_DELAY-speed)
#define XOR(a, b) (((a) && !(b)) || (!(a) && (b)))


// -----------------------------------------------------------------------------------------------------------------
typedef struct StepMotor {
  // ------------------------------------------------
  // Members
  uint8_t _s_pin;                         // Step Pin
  uint8_t _d_pin;                         // Direction pin

  bool _is_clockwise;                 // Is default direction clockwise or anti-clockwise
  int _speed;                         // The speed of the motor
  int _cycle;                         // Number of steps for a full cycle

  // ------------------------------------------------
  // Constructors
  StepMotor(uint8_t s_pin, uint8_t d_pin, int speed, int cycle): _s_pin(s_pin), _d_pin(d_pin),  _speed(speed), _cycle(cycle) {
    // Make sure speed is within range
    if (_speed > STEP_MOTOR_MAX_SPEED)
      _speed = STEP_MOTOR_MAX_SPEED;
    if (_speed < STEP_MOTOR_MIN_SPEED)
      _speed = STEP_MOTOR_MIN_SPEED;

    // Set default value for clockwise
    _is_clockwise = true;
  }
  StepMotor(uint8_t s_pin, uint8_t d_pin): StepMotor(s_pin, d_pin, STEP_MOTOR_DEFAULT_SPEED, STEP_MOTOR_DEFAULT_CYCLE) {}


  // -----------------------------------------------
  // Rotation functions
  // Rotate with steps, speed
  uint8_t set_direction(long steps) const {
    uint8_t direction;
    if (XOR(steps > 0, _is_clockwise))
      direction = LOW;
    else
      direction = HIGH;

    digitalWrite(_d_pin, direction);
    return direction;
  }
  void rotate(long steps, int speed) const {
    // Configure direction
    this->set_direction(steps);

    int delay = SPEED_TO_DELAY(speed);      // Convert speed to actual delay
    steps = abs(steps);                     // Get absolute value of the step
    for (long i=0;i<steps;++i) {
      digitalWrite(_s_pin, HIGH);
      delayMicroseconds(delay);
      digitalWrite(_s_pin, LOW);
      delayMicroseconds(delay);
    }
  }

  // Rotate with only steps & direction
  void rotate(long steps) const { this->rotate(steps, _speed); }

} StepMotor;


typedef struct StepDisplacer : public StepMotor {
  // -----------------------------------------------
  // Members
  int _distance_coef;                 // Distance coefficent


  // -----------------------------------------------
  // Constructor
  StepDisplacer(uint8_t s_pin, uint8_t d_pin, int distance_coef=STEP_DISPLACER_DEFAULT_DISTANCE_COEF): StepMotor(s_pin, d_pin), _distance_coef(distance_coef) {}


  // -----------------------------------------------
  // Move forward & backward functions
  // The distance is calculated based on the number of rounds the motor will rotate
  // One round = number of steps for 1 cycle = _cycle (DEFAULT = 1600).
  // Distance coefficient variable (_distance_coef) will decide how fast the displacer "moves" forward and backword
  // _distance_coef = 1000 MEANS 1 cycle (1600 steps) can move 1000 micrometres.
  void move(long micrometres, int speed) const {
    // Calculate the number of steps needed
    long steps = this->calculate_steps(micrometres);
    this->rotate(steps, speed);
  }
  void move(long micrometres) const { this->move(micrometres, _speed); }
  long calculate_steps(long micrometres) const { return long(micrometres * 1.0 / _distance_coef * _cycle); }

} StepDisplacer;


typedef struct StepPump : public StepMotor {
  // -----------------------------------------------
  // Members
  long _volume_coef;                 // Volume coefficient


  // -----------------------------------------------
  // Constructors
  StepPump(uint8_t s_pin, uint8_t d_pin, long volume_coef=STEP_PUMP_DEFAULT_VOLUME_COEF): StepMotor(s_pin, d_pin), _volume_coef(volume_coef) {}


  // -----------------------------------------------
  // Pump an amount of of liquid function
  // The volume is calculated based on the number of rounds the motor will rotate
  // One round = number of steps for 1 cycle 
  // Volume coefficient varialbe (_volume_coef) will decide how much liquid can be pumped per motor step.
  // _volume_coef = 200 MEANS 1 cycle (1600 steps) can pump 200000 micro_ccs
  void pump(long micro_ccs, int speed) const {
    long steps = this->calculate_steps(micro_ccs);
    this->rotate(steps, speed);
  }
  void pump(long micro_ccs) const { this->pump(micro_ccs, _speed); }
  long calculate_steps(long micro_ccs) const { return long(micro_ccs * 1.0 / _volume_coef * _cycle); }

} StepPump;


// -----------------------------------------------------------------------------------------------------------------
typedef struct Vector {
  long x;      // Micrometres
  long y;
  long z;
  Vector(long p_x, long p_y, long p_z): x(p_x), y(p_y), z(p_z) {}
  Vector(Vector const &vec): x(vec.x), y(vec.y), z(vec.z) {}

  // Overloading overator
  Vector operator+(Vector const &vec) const { return Vector(x+vec.x, y+vec.y, z+vec.z); }
  Vector operator-(Vector const &vec) const { return Vector(x-vec.x, y-vec.y, z-vec.z); }
} Vector;


typedef struct Injector {
  // -----------------------------------------------
  // Members
  StepPump const *pump;
  Vector const *location;


  // -----------------------------------------------
  // Constructor
  Injector(StepPump const *pump, long x, long y, long z): pump(pump), location(new Vector(x, y, z)) {}
  ~Injector() {
    delete location;
  }

} Injector;


typedef struct BaseController {
  // -----------------------------------------------
  // Members
  StepDisplacer const *_x_displacer;
  StepDisplacer const *_y_displacer;
  StepDisplacer const *_z_displacer;

  Vector *_loc;


  // -----------------------------------------------
  // Constructors
  BaseController(StepDisplacer const *x_dis, StepDisplacer const *y_dis, StepDisplacer const *z_dis)
      : _x_displacer(x_dis), _y_displacer(y_dis), _z_displacer(z_dis)
      , _loc(new Vector(0, 0, 0)) {}
  ~BaseController() {
    delete _loc;
  }


  // -----------------------------------------------
  // Member functiosn
  // Move the base by a micrometre Vector(x, y, z)
  void move(Vector const &vec) { this->move(vec.x, vec.y, vec.z); }
  void move(Vector const *vec) { this->move(vec->x, vec->y, vec->z); }
  void move(long const x, long const y, long const z) {
    // Move along the x axis
    if (x != 0) {
      _x_displacer->move(x);
      _loc->x += x;
    }
    if (y != 0) {
      _y_displacer->move(y);
      _loc->y += y;
    }
    if (z != 0) {
      _z_displacer->move(z);
      _loc->z += z;
    }
  }

  // Move to a location
  void move_to(long const x, long const y, long const z) { this->move(x - _loc->x, y - _loc->y, z - _loc->z); }
  void move_to(Vector const *point) { this->move(point->x - _loc->x, point->y - _loc->y, point->z - _loc->z); }
  void move_to(Vector const &point) { this->move(point.x - _loc->x, point.y - _loc->y, point.z - _loc->z); }

  // Change the value of x, y, z without actually moving the displacers
  // Thsi is used to update the location of the base after pumping
  void shift(Vector const &vec) { this->shift(vec.x, vec.y, vec.z); }
  void shift(Vector const *vec) { this->shift(vec->x, vec->y, vec->z); }
  void shift(long const x, long const y, long const z) {
    _loc->x += x;
    _loc->y += y;
    _loc->z += z;
  }

  // Shift to a location
  void shift_to(long const x, long const y, long const z) { this->shift(x - _loc->x, y - _loc->y, z - _loc->z); }
  void shift_to(Vector const *point) { this->shift(point->x - _loc->x, point->y - _loc->y, point->z - _loc->z); }
  void shift_to(Vector const &point) { this->shift(point.x - _loc->x, point.y - _loc->y, point.z - _loc->z); }


  // Move the base to the (0, 0, 0)
  void reset() {
    this->move(-_loc->x, -_loc->y, -_loc->z);
  }

  Vector get_location() { return Vector(_loc->x, _loc->y, _loc->z); }

} BaseController;


typedef struct PumpingController {
  // -----------------------------------------------
  // Members
  StepDisplacer const *_displacer;
  StepPump const *_pump;


  // -----------------------------------------------
  // Constructor
  PumpingController(StepDisplacer const *displacer, StepPump const *pump): _pump(pump), _displacer(displacer) {}


  // -----------------------------------------------
  // Pump and move at the same time
  // Because no multithreading is supported, the basic idea is to make the StepDisplacer to rotate n steps while StepPump to rotate m steps. 
  // Both must start & finish at the same time.
  void pump(long micrometres, long micro_ccs, int speed=STEP_MOTOR_DEFAULT_SPEED) const {
    // Steps for each motors
    long pump_steps = _pump->calculate_steps(micro_ccs);
    long displacer_steps = _displacer->calculate_steps(micrometres);

    // Direction for each motors
    _pump->set_direction(pump_steps);
    _displacer->set_direction(displacer_steps);

    // Initialize running parameters
    int delay = SPEED_TO_DELAY(speed);        // Convert speed to actual delay
    pump_steps = abs(pump_steps);             // Steps should be always positive
    displacer_steps = abs(displacer_steps);

    // Algorithm
    // Rotate displacer by 1 step,
    // Rotate pump by n steps, n is calculate based on step_ratio
    // Repeat the above 2 lines until displacer already rotates full displacer_steps
    long current_pump_step = 0;
    float step_ratio = pump_steps * 1.0 / displacer_steps;
    for (long i=0;i<displacer_steps;++i) {
      bool rotate_pump = false;
      long total_pump_steps = long(i * step_ratio);
      total_pump_steps = total_pump_steps > pump_steps ? pump_steps : total_pump_steps;   // Make sure pump never rotates more than pump_steps

      // In case pump_steps > displacer_steps (step_ratio > 1)
      if (total_pump_steps - current_pump_step > 1) {
        while (current_pump_step < total_pump_steps - 1) {  // Minus 1 is to leave the 1 step-rotate for the code below
          digitalWrite(_pump->_s_pin, HIGH);
          delayMicroseconds(delay);
          digitalWrite(_pump->_s_pin, LOW);
          delayMicroseconds(delay);

          current_pump_step++;
        }

        rotate_pump = true;
        current_pump_step = total_pump_steps;               // Update current_pump_step for next round
      }
      // Otherwise, either rotating pump 1 step or not
      else if (total_pump_steps - current_pump_step > 0) {
        rotate_pump = true;
        current_pump_step = total_pump_steps;               // Update current_pump step for next round
      }

      // Rotate the motors
      digitalWrite(_displacer->_s_pin, HIGH);
      if (rotate_pump)
        digitalWrite(_pump->_s_pin, HIGH);
      delayMicroseconds(delay);
      digitalWrite(_displacer->_s_pin, LOW);
      if (rotate_pump)
        digitalWrite(_pump->_s_pin, LOW);
      delayMicroseconds(delay);
    }
  }
} PumpingController;



// -----------------------------------------------------------------------------------------------------------------
// For contraolling all the system & execute the system based on the input
typedef struct SystemController {
  // -----------------------------------------------
  // Members
  BaseController *base;                 // Pointer to the base controller
  Injector **injectors;                 // Pointer to a list of injector
  
  uint8_t injector_count;                   // Number of injector

  long const c_injecting_starting_point_z;        // The height of the base at which the liquid should be injected into the tube first time.
  long injecting_current_point_z;                 // The height of the current base at which the liquid can be injected into the tube

  // -----------------------------------------------
  // Constructors
  SystemController(long injecting_starting_point_z): c_injecting_starting_point_z(injecting_starting_point_z) {
    base = NULL;
    injectors = NULL;

    injector_count = 0;
    injecting_current_point_z = c_injecting_starting_point_z;
  }
  ~SystemController() {
//    if (base != NULL)
//      delete base;

    if (injectors != NULL) {
      delete[] injectors;
    }
  }


  // -----------------------------------------------
  // Initializing function
//  void init_base(StepDisplacer const *x_dis, StepDisplacer const *y_dis, StepDisplacer const *z_dis, Vector const *vec=NULL) {
//    base = new BaseController(x_dis, y_dis, z_dis);
//    if (vec)
//      base->move_to(vec);
//  }
//  void init_injectors(StepPump const * const *pumps, Vector const * const *locations, int count) {
//    injector_count = count;
//    injectors = new Injector*[count];
//    for (int i=0;i<count;++i)
//      injectors[i] = new Injector(pumps[i], locations[i]->x, locations[i]->y, locations[i]->z);
//  }
  void init_base(BaseController *p_base) { base = p_base; }
  void init_injectors(Injector * const * const p_injectors, uint8_t count) {
    if (injectors)
      delete[] injectors;
    injectors = new Injector*[count];
    for (uint8_t i=0;i<count;++i)
      injectors[i] = p_injectors[i];
  }

  // Configure the whole system
  void configure() {
  }
  

  // -----------------------------------------------
  // Control functions

  // Move the base
  void set_base_loc(Vector const &vec) { base->move(vec); }
  void reset() {
    base->reset();
    injecting_current_point_z = c_injecting_starting_point_z;
  }

  // Inject an amount of liquid using the nth injector
  void inject(long micro_ccs, uint8_t injector_index) {
    if (injector_index >= injector_count)
      return;

    // Calculate the distance based onthe volume
    long micrometres = long(micro_ccs * PUMPING_DISTANCE_VOLUME_RATIO);

    // Move the base to the right location
    base->move_to(injectors[injector_index]->location);                 // Right under injector head, y IS STILL = 0
    base->move(0, injecting_current_point_z, 0);                        // Lift the base to the point where it can be injected

    // Start injecting the liquid
    PumpingController *controller = new PumpingController(base->_y_displacer, injectors[injector_index]->pump);
    controller->pump(-micrometres, micro_ccs);
    delete controller;

    // Move the base off to the y=0 location
    base->shift(0, -micrometres, 0);                                    
    injecting_current_point_z -= micrometres;                           // Recalculate the current pumping location
    base->move(0, -injecting_current_point_z, 0);
  }

  // Inject a list of (liquid, injector_index)
  void inject(long const *volumes, long const *indices, uint8_t count) {
    for (int i=0;i<count;++i)
      this->inject(volumes[i], indices[i]);
  }


} SystemController;



