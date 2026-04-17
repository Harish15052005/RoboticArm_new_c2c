#include <WiFi.h>
#include <WebSocketsServer.h>
#include "servo_esp32.hpp"

// ---------------- Network Credentials ----------------
const char* ssid = "ROBOCON LDCE";         // <-- Enter your WiFi SSID
const char* password = "RBCN2025"; // <-- Enter your WiFi Password

// ---------------- Servo Pins ----------------
#define BASE_SERVO 23
#define SHOULDER_SERVO 22
#define ELBOW_SERVO 21
#define WRIST_PITCH_SERVO 19
#define WRIST_ROLL_SERVO 18
#define GRIPPER_SERVO 5

SERVO base, shoulder, elbow, wristPitch, wristRoll, gripper;

// ---------------- Websocket Server ----------------
// Listen on port 81 (standard for WebSockets)
WebSocketsServer webSocket = WebSocketsServer(81);

// ---------------- Target Joint Angles ----------------
// Initialized to match your setup() defaults
float baseAngle = 0;
float shoulderAngle = 45;
float elbowAngle = 120;
float wristPitchAngle = 90;
float wristRollAngle = 0;
float gripperAngle = 20;

// ---------------- WebSocket Event Handler ----------------
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Dashboard Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Dashboard Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
    case WStype_TEXT:
      // The browser sends data at 5Hz in this exact format: "S1,S2,S3,S4,S5,S6"
      // Example payload: "90,45,120,90,180,30"
      if (sscanf((const char *)payload, "%f,%f,%f,%f,%f,%f", 
                 &baseAngle, &shoulderAngle, &elbowAngle, 
                 &wristPitchAngle, &wristRollAngle, &gripperAngle) == 6) {
        // Values successfully unpacked into the global target variables
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // 1. Initialize WiFi
  Serial.print("\nConnecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP()); // <-- Enter this IP into your Web Dashboard!

  // 2. Initialize WebSockets
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket Server Started on Port 81.");

  // 3. Servo Attachments
  setupServo(base, BASE_SERVO, 0);
  setupServo(shoulder, SHOULDER_SERVO, 1);
  setupServo(elbow, ELBOW_SERVO, 2);
  setupServo(wristPitch, WRIST_PITCH_SERVO, 3);
  setupServo(wristRoll, WRIST_ROLL_SERVO, 4);
  setupServo(gripper, GRIPPER_SERVO, 5);

  // 4. Exact Servo Calibration (Preserved)
  base.min = 190;        base.minAngle = 0;        base.max = 920;        base.maxAngle = 180;
  shoulder.min = 300;    shoulder.minAngle = 0;    shoulder.max = 1100;   shoulder.maxAngle = 180;
  elbow.min = 290;       elbow.minAngle = 0;       elbow.max = 990;       elbow.maxAngle = 180;
  wristPitch.min = 350;  wristPitch.minAngle = 0;  wristPitch.max = 1100; wristPitch.maxAngle = 180;
  wristRoll.min = 340;   wristRoll.minAngle = 0;   wristRoll.max = 1050;  wristRoll.maxAngle = 180;
  gripper.min = 600;     gripper.minAngle = 0;     gripper.max = 850;     gripper.maxAngle = 180;

  // 5. Initial Safe Posing
  setServo(base, 70);
  setServo(shoulder, 125);
  setServo(elbow, 105);
  setServo(wristPitch, 90);
  setServo(wristRoll, 0);
}

// Timing variables
unsigned long lastUpdateTime = 0;
unsigned long prevPrintTime = 0;

void loop() {
  // Always keep the WebSocket server listening and processing incoming packets
  webSocket.loop();

  // Your preserved, high-frequency smoothing loop (12ms)
  if (millis() - lastUpdateTime > 12) {
    setServo(base, baseAngle);
    setServo(shoulder, shoulderAngle);
    setServo(elbow, elbowAngle);
    setServo(wristPitch, wristPitchAngle);
    setServo(wristRoll, wristRollAngle);
    setServo(gripper, gripperAngle);

    lastUpdateTime = millis();
  }

  // Print state to Serial Monitor for debugging (every 250ms)
  if (millis() - prevPrintTime > 100) {
    Serial.print("Current Targets -> Base: "); Serial.print(baseAngle);
    Serial.print(" | Shoulder: "); Serial.print(shoulderAngle);
    Serial.print(" | Elbow: "); Serial.print(elbowAngle);
    Serial.print(" | Pitch: "); Serial.print(wristPitchAngle);
    Serial.print(" | Roll: "); Serial.print(wristRollAngle);
    Serial.print(" | Gripper: "); Serial.println(gripperAngle);

    prevPrintTime = millis();
  }
}


// #include "servo_esp32.hpp"

// #define BASE_SERVO 23
// #define SHOULDER_SERVO 22
// #define ELBOW_SERVO 21
// #define WRIST_PITCH_SERVO 19
// #define WRIST_ROLL_SERVO 18
// #define GRIPPER_SERVO 5

// SERVO base, shoulder, elbow, wristPitch, wristRoll, gripper;
// void setup() {
//   Serial.begin(115200);
//   setupServo(base, BASE_SERVO, 0);
//   setupServo(shoulder, SHOULDER_SERVO, 1);
//   setupServo(elbow, ELBOW_SERVO, 2);
//   setupServo(wristPitch, WRIST_PITCH_SERVO, 3);
//   setupServo(wristRoll, WRIST_ROLL_SERVO, 4);
//   setupServo(gripper, GRIPPER_SERVO, 5);
// }

// SERVO* servos[] = {&base, &shoulder, &elbow, &wristPitch, &wristRoll, &gripper};

// int inp = 0, inp1 = 0;
// void loop() {
//   if(Serial.available()) {
//     inp = Serial.parseInt();
//     inp1 = Serial.parseInt();
//   }
//   ledcWrite(servos[inp]->channel, inp1);
// }
