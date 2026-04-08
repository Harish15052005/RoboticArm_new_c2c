#include "servo_esp32.hpp"

#define BASE_SERVO 19
#define SHOULDER_SERVO 18
#define ELBOW_SERVO 5
#define WRIST_PITCH_SERVO 17
#define WRIST_ROLL_SERVO 16
#define GRIPPER_SERVO 4

SERVO base, shoulder, elbow, wristPitch, wristRoll, gripper;
void setup() {
  Serial.begin(115200);
  setupServo(base, BASE_SERVO, 0);
  setupServo(shoulder, SHOULDER_SERVO, 1);
  setupServo(elbow, ELBOW_SERVO, 2);
  setupServo(wristPitch, WRIST_PITCH_SERVO, 3);
  setupServo(wristRoll, WRIST_ROLL_SERVO, 4);
  setupServo(gripper, GRIPPER_SERVO, 5);
  base.min = 190;base.minAngle = 0;base.max = 920;base.maxAngle = 180;
  shoulder.min = 300;shoulder.minAngle = 180;shoulder.max = 1100;shoulder.maxAngle = 0;
  elbow.min = 290;elbow.minAngle = 0;elbow.max = 990;elbow.maxAngle = 180;
  wristPitch.min = 430;wristPitch.minAngle = 0;wristPitch.max = 1190;wristPitch.maxAngle = 180;
  wristRoll.min = 340;wristRoll.minAngle = 0;wristRoll.max = 1050;wristRoll.maxAngle = 180;
  gripper.min = 650;gripper.minAngle = 0;gripper.max = 750;gripper.maxAngle = 180;
  setServo(base, 0);
  setServo(shoulder, 45);
  setServo(elbow, 120);
  setServo(wristPitch, 90);
  setServo(wristRoll, 0);
}

SERVO* servos[] = {&base, &shoulder, &elbow, &wristPitch, &wristRoll, &gripper};

int inp = 0, inp1 = 90, inp2 = 10;
unsigned long prevPrintTime = 0;
unsigned long prevServoUpdateTime = 0;

void loop() {
  if(Serial.available()) {
    inp = Serial.parseInt();
    inp1 = Serial.parseInt();
    inp2 = Serial.parseInt();
  }
  // ledcWrite(servos[inp]->channel, inp1);
  if(millis() - prevPrintTime > 75) {
    Serial.print("Base: "); Serial.print(map(base.filterdDuty, base.min, base.max, base.minAngle, base.maxAngle)); Serial.print(" , ");Serial.print(base.filterdDuty);Serial.print(" | ");
    Serial.print("Shoulder: "); Serial.print(map(shoulder.filterdDuty, shoulder.min, shoulder.max, shoulder.minAngle, shoulder.maxAngle));Serial.print(" , ");Serial.print(shoulder.filterdDuty); Serial.print(" | ");
    Serial.print("Elbow: "); Serial.print(map(elbow.filterdDuty, elbow.min, elbow.max, elbow.minAngle, elbow.maxAngle));Serial.print(" , ");Serial.print(elbow.filterdDuty); Serial.print(" | ");
    Serial.print("Wrist Pitch: "); Serial.print(map(wristPitch.filterdDuty, wristPitch.min, wristPitch.max, wristPitch.minAngle, wristPitch.maxAngle));Serial.print(" , ");Serial.print(wristPitch.filterdDuty); Serial.print(" | ");
    Serial.print("Wrist Roll: "); Serial.print(map(wristRoll.filterdDuty, wristRoll.min, wristRoll.max, wristRoll.minAngle, wristRoll.maxAngle)); Serial.print(" , ");Serial.print(wristRoll.filterdDuty); Serial.print(" | ");
    Serial.print("Gripper: "); Serial.print(map(gripper.filterdDuty, gripper.min, gripper.max, gripper.minAngle, gripper.maxAngle));
    Serial.print(" | "); Serial.print(inp2);
    Serial.println();
    prevPrintTime = millis();
  }

  if(millis() - prevServoUpdateTime > 10)
  {
    setServo(*servos[inp], inp1);
    prevServoUpdateTime = millis();
  }
}
/*
base : 0 = 190 | 180 = 920 
shoulder : 0 = 300 | 90 = 700 -> 180= 1100
elbow : 0 = 290 | 90 = 640 -> 180 = 990
wristPitch : 0 = 430 | 90 = 810 -> 180 = 1190
wristRoll : 0 = 340 | 180 = 1050
gripper : close: 750 | 650
*/