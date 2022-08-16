#include "arduino-step-motors.cpp"

int smDirectionPin = 2; //Direction pin
int smStepPin = 3; //Stepper pin

int smDirectionPin2 = 6;
int smStepPin2 = 7;

int smDirectionPin3 = 9;
int smStepPin3 = 10;

void setup() {
  // put your setup code here, to run once:
  /*Sets all pin to output; the microcontroller will send them(the pins) bits, it will not expect to receive any bits from thiese pins.*/
  pinMode(smDirectionPin, OUTPUT);
  pinMode(smStepPin, OUTPUT);
  pinMode(smDirectionPin2, OUTPUT);
  pinMode(smStepPin2, OUTPUT);
  pinMode(smDirectionPin3, OUTPUT);
  pinMode(smStepPin3, OUTPUT);

  Serial.begin(9600);
}

// Configure the motors
StepMotor motor1(smStepPin, smDirectionPin);
StepDisplacer displacer1(smStepPin, smDirectionPin);
StepDisplacer displacer2(smStepPin2, smDirectionPin2);
StepDisplacer displacer3(smStepPin3, smDirectionPin3);
StepPump pump1(smStepPin, smDirectionPin);
StepPump pump2(smStepPin2, smDirectionPin2);


void loop() {
//  // put your main code here, to run repeatedly:
//  digitalWrite(smDirectionPin, HIGH); //Writes the direction to the EasyDriver DIR pin. (HIGH is clockwise).
//  digitalWrite(smDirectionPin2, HIGH);
//  /*Slowly turns the motor 1600 steps*/
//  for (int i = 0; i < 1600; i++){
//    digitalWrite(smStepPin, HIGH);
//    digitalWrite(smStepPin2, HIGH);
//    delayMicroseconds(700);
//    digitalWrite(smStepPin, LOW);
//    digitalWrite(smStepPin2, LOW);
//    delayMicroseconds(700);
//  }
//
//  delay(1000); //Pauses for a second (the motor does not need to pause between switching direction, so you can safely remove this)
//
//  digitalWrite(smDirectionPin, LOW); //Writes the direction to the EasyDriver DIR pin. (LOW is counter clockwise)
//  digitalWrite(smDirectionPin2, LOW);
//  /*Turns the motor fast 1600 steps*/
//  for (int i = 0; i < 1600; i++){
//    digitalWrite(smStepPin, HIGH);
//    digitalWrite(smStepPin2, HIGH);
//    delayMicroseconds(150);
//    digitalWrite(smStepPin, LOW);
//    digitalWrite(smStepPin2, LOW);
//    delayMicroseconds(150);
//  }
  PumpingController controller(&displacer1, &pump2);
  controller.pump(1000, 50000);

  delay(1000);

  Serial.println("Hello Wordl");
}
