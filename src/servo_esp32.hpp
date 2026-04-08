#include "esp32-hal-ledc.h"
#ifndef SERVO_H
#define SERVO_H
#include <arduino.h>
#define PWM_FREQ 50
#define PWM_RES 13
#define PWM_MAX 8191

struct SERVO {
  int min = 125, max = 1105, minAngle = 0, maxAngle = 225, pin, channel;
  float alpha = 0.98, filterdDuty = 0;
};

void setServo(SERVO& s, int angle) {
  if (angle < s.minAngle) angle = s.minAngle;
  if (angle > s.maxAngle) angle = s.maxAngle;
  uint32_t duty = map(angle, s.minAngle, s.maxAngle, s.min, s.max);

  s.filterdDuty = s.filterdDuty * s.alpha + (1 - s.alpha) * duty;

  if (s.filterdDuty < s.min) s.filterdDuty = s.min;
  if (s.filterdDuty > s.max) s.filterdDuty = s.max;
  ledcWrite(s.channel, s.filterdDuty);
}

void setupServo(SERVO& s, int servoPin, int channel) {
  s.pin = servoPin;
  s.channel = channel;
  ledcSetup(s.channel, PWM_FREQ, PWM_RES);
  ledcAttachPin(s.pin, s.channel);
  //     ledcSetup(0, PWM_FREQ, PWM_RES);
  //   ledcAttachPin(tstServoPin, 0);
}


#endif