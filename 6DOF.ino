#include <WiFi.h>
#include <ESP32Servo.h>
#include <math.h>

// Servo setup
Servo servo1, servo2, servo3, servo4, servo5, servo6;
const int servoPin1 = 18, servoPin2 = 19, servoPin3 = 21, servoPin4 = 22, servoPin5 = 23, servoPin6 = 25;
int def1 = 90, def2 = 45, def3 = 135, def4 = 90, def5 = 60, def6 = 120;

WiFiServer server(80);
const char* ssid = "ESP32_Servo_AP";
const char* password = "12345678";

// Mode
String mode = "manual";

// Teach and repeat
std::vector<std::vector<int>> recordedAngles;
bool isRecording = false, isRepeating = false;
unsigned long lastStepTime = 0;
const unsigned long repeatInterval = 1000;
int repeatIndex = 0;

// IK link lengths and gripper length
const float L1 = 100; // shoulder to elbow
const float L2 = 100; // elbow to wrist
const float L3 = 100; // wrist to gripper tip
const float SHOULDER_OFFSET_DEG = 20;

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  server.begin();
  Serial.println(WiFi.softAPIP());

  servo1.attach(servoPin1); servo2.attach(servoPin2); servo3.attach(servoPin3);
  servo4.attach(servoPin4); servo5.attach(servoPin5); servo6.attach(servoPin6);

  resetServos();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (c == '\n') {
          if (request.indexOf("GET /?mode=manual") >= 0) {
            mode = "manual"; Serial.println("Switched to manual mode.");
          }
          if (request.indexOf("GET /?mode=auto") >= 0) {
            mode = "auto"; Serial.println("Switched to auto mode.");
          }

          if (mode == "manual" && request.indexOf("GET /?servo1=") >= 0) {
            int s1 = getValue(request, "servo1");
            int s2 = getValue(request, "servo2");
            int s3 = getValue(request, "servo3");
            int s4 = getValue(request, "servo4");
            int s5 = getValue(request, "servo5");
            int s6 = getValue(request, "servo6");

            servo1.write(s1); servo2.write(s2); servo3.write(s3);
            servo4.write(s4); servo5.write(s5); servo6.write(s6);

            if (isRecording) {
              recordedAngles.push_back({s1, s2, s3, s4, s5, s6});
            }
          }

          if (request.indexOf("GET /reset") >= 0) resetServos();
          if (request.indexOf("GET /recordStart") >= 0) {
            recordedAngles.clear(); isRecording = true; Serial.println("Recording started.");
          }
          if (request.indexOf("GET /recordStop") >= 0) {
            isRecording = false; Serial.println("Recording stopped.");
          }
          if (request.indexOf("GET /repeat") >= 0) {
            if (!recordedAngles.empty()) {
              isRepeating = true;
              repeatIndex = 0;
              lastStepTime = millis();
              Serial.println("Repeat started.");
            }
          }

          // Inverse Kinematics in auto mode
          if (mode == "auto" && request.indexOf("GET /?x=") >= 0) {
            float x = getValue(request, "x");
            float y = getValue(request, "y");
            float z = getValue(request, "z");

            computeIK(x, y, z);
          }

          // Send web page
          sendWebPage(client);
          break;
        }
      }
    }
    client.stop();
  }

  if (isRepeating && millis() - lastStepTime > repeatInterval) {
    if (repeatIndex < recordedAngles.size()) {
      auto a = recordedAngles[repeatIndex];
      servo1.write(a[0]); servo2.write(a[1]); servo3.write(a[2]);
      servo4.write(a[3]); servo5.write(a[4]); servo6.write(a[5]);
      repeatIndex++; lastStepTime = millis();
    } else {
      isRepeating = false;
    }
  }
}

void resetServos() {
  smoothMove(servo1, servo1.read(), def1);
  smoothMove(servo2, servo2.read(), def2);
  smoothMove(servo3, servo3.read(), def3);
  smoothMove(servo4, servo4.read(), def4);
  smoothMove(servo5, servo5.read(), def5);
  smoothMove(servo6, servo6.read(), def6);
  Serial.println("Reset to defaults.");
}

int getValue(String req, String key) {
  int start = req.indexOf(key + "=");
  if (start == -1) return 0;
  start += key.length() + 1;
  int end = req.indexOf('&', start);
  if (end == -1) end = req.indexOf(' ', start);
  return req.substring(start, end).toInt();
}

void smoothMove(Servo &servo, int current, int target, int dTime = 10) {
  int step = (current < target) ? 1 : -1;
  for (int a = current; a != target; a += step) {
    servo.write(a); delay(dTime);
  }
  servo.write(target);
}

void computeIK(float x, float y, float z) {
  float baseAngle = atan2(y, x) * 180 / PI;
  float wristX = x - L3 * cos(baseAngle * PI / 180);
  float wristY = y - L3 * sin(baseAngle * PI / 180);
  float r = sqrt(wristX * wristX + wristY * wristY);
  float c = sqrt(r * r + z * z);

  float theta1 = acos((L1*L1 + c*c - L2*L2) / (2 * L1 * c)) * 180 / PI;
  float theta2 = acos((L1*L1 + L2*L2 - c*c) / (2 * L1 * L2)) * 180 / PI;

  float angleOffset = atan2(z, r) * 180 / PI;
  float shoulder = angleOffset + theta1 - SHOULDER_OFFSET_DEG;
  float elbow = 180 - theta2;

  servo1.write(baseAngle);
  servo2.write(shoulder);
  servo3.write(elbow);

  Serial.printf("Base: %.1f, Shoulder: %.1f, Elbow: %.1f\n", baseAngle, shoulder, elbow);
}

void sendWebPage(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html\n");

  client.println("<!DOCTYPE html><html><head><title>ESP32 Arm</title>");
  client.println("<style>body{text-align:center;}button,input{margin:5px;}</style>");
  client.println("</head><body><h2>ESP32 6-DOF Arm</h2>");

  client.println("<button onclick=\"location.href='/?mode=manual'\">Manual Mode</button>");
  client.println("<button onclick=\"location.href='/?mode=auto'\">Auto Mode</button><br>");

  if (mode == "manual") {
    client.println("<p><b>Manual Mode</b></p>");
    for (int i = 1; i <= 6; i++) {
      client.printf("<p>Servo %d: <span id='val%d'></span><br><input type='range' id='s%d' min='0' max='180' value='90' onchange='send()'></p>", i, i, i);
    }
    client.println("<button onclick='reset()'>Reset</button>");
    client.println("<button onclick='startRec()'>Start Record</button>");
    client.println("<button onclick='stopRec()'>Stop Record</button>");
    client.println("<button onclick='repeat()'>Repeat</button>");
  }

  if (mode == "auto") {
    client.println("<p><b>Auto Mode</b></p>");
    client.println("X: <input id='x' type='number' value='150'><br>");
    client.println("Y: <input id='y' type='number' value='0'><br>");
    client.println("Z: <input id='z' type='number' value='50'><br>");
    client.println("<button onclick='ik()'>Go</button>");
  }

  client.println("<script>");
  client.println("function send(){");
  client.println("let url = '/?servo1=' + s1.value + '&servo2=' + s2.value + '&servo3=' + s3.value + '&servo4=' + s4.value + '&servo5=' + s5.value + '&servo6=' + s6.value;");
  client.println("location.href = url; }");
  client.println("function reset(){ location.href='/reset'; }");
  client.println("function startRec(){ location.href='/recordStart'; }");
  client.println("function stopRec(){ location.href='/recordStop'; }");
  client.println("function repeat(){ location.href='/repeat'; }");
  client.println("function ik(){");
  client.println("let x = document.getElementById('x').value;");
  client.println("let y = document.getElementById('y').value;");
  client.println("let z = document.getElementById('z').value;");
  client.println("location.href = '/?x=' + x + '&y=' + y + '&z=' + z;");
  client.println("}</script>");

  client.println("</body></html>");
}