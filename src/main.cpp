#include "servo_esp32.hpp"
#include <math.h>

// ---------------- Servo Pins ----------------
#define BASE_SERVO        19
#define SHOULDER_SERVO    18
#define ELBOW_SERVO        5
#define WRIST_PITCH_SERVO 17
#define WRIST_ROLL_SERVO  16
#define GRIPPER_SERVO      4

SERVO base, shoulder, elbow, wristPitch, wristRoll, gripper;

// ---------------- Arm Geometry ----------------
// Fill these later after measuring your arm

float BASE_HEIGHT = 5.88;  // base axis -> shoulder joint height (cm)
float L1 = 10.1;           // shoulder -> elbow link length (cm)
float L2 = 13.2;           // elbow -> wrist pitch joint length (cm)
float L3 = 16.3;           // wrist pitch joint -> gripper tip (cm)

// ---------------- Target Cartesian Pose ----------------

float targetX = 15;
float targetY = 0;
float targetZ = 10;

float targetPhi = 0;     // wrist pitch
float targetRoll = 90;
float targetGrip = 20;

// ---------------- Joint Angles ----------------

float baseAngle = 0;
float shoulderAngle = 45;
float elbowAngle = 120;
float wristPitchAngle = 90;
float wristRollAngle = 90;
float gripperAngle = 20;

static const float DEG2RAD = 0.0174532925;
static const float RAD2DEG = 57.2957795;


// ---------------- Fast Clamp ----------------
inline float clampf(float v, float lo, float hi)
{
  if(v < lo) return lo;
  if(v > hi) return hi;
  return v;
}


// ======================================================
// INVERSE KINEMATICS SOLVER
// ======================================================

void solveIK(float x, float y, float zDeg, float phiDeg)
{
  float phi = phiDeg * DEG2RAD;

  baseAngle = zDeg;

  float wx = x - L3 * cosf(phi);
  float wy = (y - BASE_HEIGHT) - L3 * sinf(phi);

  float D = (wx*wx + wy*wy - L1*L1 - L2*L2) / (2 * L1 * L2);
  D = clampf(D, -1, 1);

  float elbowRad = atan2f(-sqrtf(1 - D*D), D);

  float shoulderRad =
      atan2f(wy, wx) -
      atan2f(L2 * sinf(elbowRad), L1 + L2 * cosf(elbowRad));

  float wristRad = phi - shoulderRad - elbowRad;

  shoulderAngle = shoulderRad * RAD2DEG;
  elbowAngle = elbowRad * RAD2DEG;

  // servo frame correction
  wristPitchAngle = wristRad * RAD2DEG + 90;
}

// ======================================================
// SERVO UPDATE (must run continuously)
// ======================================================

void setup()
{
  Serial.begin(115200);

  setupServo(base, BASE_SERVO, 0);
  setupServo(shoulder, SHOULDER_SERVO, 1);
  setupServo(elbow, ELBOW_SERVO, 2);
  setupServo(wristPitch, WRIST_PITCH_SERVO, 3);
  setupServo(wristRoll, WRIST_ROLL_SERVO, 4);
  setupServo(gripper, GRIPPER_SERVO, 5);

  // servo calibration (your previous values)

  base.min = 190; base.max = 920;
  shoulder.min = 300; shoulder.max = 1100;
  elbow.min = 290; elbow.max = 990;
  wristPitch.min = 430; wristPitch.max = 1190;
  wristRoll.min = 340; wristRoll.max = 1050;
  gripper.min = 650; gripper.max = 750;
}

unsigned long lastUpdateTime = 0;
unsigned long prevPrintTime = 0;

void loop()
{
  if(Serial.available())
  {
    targetX = Serial.parseFloat();
    targetY = Serial.parseFloat();
    targetZ = Serial.parseFloat();

    targetPhi = Serial.parseFloat();
    targetRoll = Serial.parseFloat();
    targetGrip = Serial.parseFloat();

    solveIK(targetX, targetY, targetZ, targetPhi);

    wristRollAngle = targetRoll;
    gripperAngle = targetGrip;
  }

  if(millis() - lastUpdateTime > 12) // update every 20 ms
  {
    setServo(base, baseAngle);
    setServo(shoulder, shoulderAngle);
    setServo(elbow, elbowAngle);
    setServo(wristPitch, wristPitchAngle);
    setServo(wristRoll, wristRollAngle);
    setServo(gripper, gripperAngle);

    lastUpdateTime = millis();
  }
    if (millis() - prevPrintTime < 75) return;

  Serial.print("Target XYZ: ");
  Serial.print(targetX); Serial.print(" , ");
  Serial.print(targetY); Serial.print(" , ");
  Serial.print(targetZ); Serial.print(" | ");

  Serial.print("Pitch: "); Serial.print(targetPhi); Serial.print(" , ");
  Serial.print("Roll: "); Serial.print(targetRoll); Serial.print(" , ");
  Serial.print("Grip: "); Serial.print(targetGrip);

  Serial.print(" Base: ");
  Serial.print(baseAngle); Serial.print(" | ");

  Serial.print("Shoulder: ");
  Serial.print(shoulderAngle); Serial.print(" | ");

  Serial.print("Elbow: ");
  Serial.print(elbowAngle); Serial.print(" | ");

  Serial.print("WristPitch: ");
  Serial.print(wristPitchAngle); Serial.print(" | ");

  Serial.print("WristRoll: ");
  Serial.print(wristRollAngle); Serial.print(" | ");

  Serial.print("Gripper: ");
  Serial.print(gripperAngle);

  Serial.println();

  prevPrintTime = millis();
}