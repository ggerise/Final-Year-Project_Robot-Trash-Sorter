#include <NewPing.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>

const char* ssid = "darrenhh-TIME";
const char* password = "HH881026";

#define BOT_TOKEN "7312496633:AAE3ACYP8u7n-7Ps80NgCHwyQrXEEcHElhk"
#define CHAT_ID "1362610776"

// Waste bin level sensors
#define trigL 33 //ultrasonic sensor (left) non metal
#define echoL 26
#define trigR 32 //ultrasonic sensor (right) metal
#define echoR 34
#define MAX_DISTANCE 27 // Maximum distance for waste level (in centimeters)

// Waste separation sensors
#define trig 13
#define echo 12
#define max_separate 10

const int metal = 27; // proximity sensor
const int servoPin = 23; // servo sensor

const int motorA1 = 21;
const int motorA2 = 19;
const int motorB1 = 18;
const int motorB2 = 5;

// Enable pins for speed control
const int enableA = 2;
const int enableB = 4;

int speed = 255; // Default speed (0-255)


NewPing sonar1(trigL, echoL, MAX_DISTANCE);
NewPing sonar2(trigR, echoR, MAX_DISTANCE);
NewPing sonarSeparate(trig, echo, max_separate);

Servo servo1;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

unsigned long lastNotificationTime = 0;
const unsigned long notificationInterval = 3600000; // 1 hour in milliseconds

void setup() {
  Serial.begin(115200);
  delay(1000);
  // Set up motor pins
  pinMode(motorA1, OUTPUT);
  pinMode(motorA2, OUTPUT);
  pinMode(motorB1, OUTPUT);
  pinMode(motorB2, OUTPUT);
  pinMode(enableA, OUTPUT);
  pinMode(enableB, OUTPUT);
    
  digitalWrite(enableA, HIGH);
  digitalWrite(enableB, HIGH);

  setSpeed(speed);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println(WiFi.localIP());

  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  // Set up WebSocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Set up web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getHTML());
  });

  server.begin();

  // Set up waste separation components
  pinMode(metal, INPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  servo1.attach(servoPin);
  servo1.write(90);
}

void loop() {
  ws.cleanupClients();
  // Waste level monitoring
  unsigned int distanceL = sonar1.ping();
  unsigned int distanceR = sonar2.ping();
  int distanceL_cm = distanceL / US_ROUNDTRIP_CM;
  int distanceR_cm = distanceR / US_ROUNDTRIP_CM;

  unsigned int levelL = 100 - ((distanceL_cm * 100) / MAX_DISTANCE);
  unsigned int levelR = 100 - ((distanceR_cm * 100) / MAX_DISTANCE);

  String message = String(levelL) + "," + String(levelR);
  ws.textAll(message);

  Serial.print("L:");
  Serial.print(levelL);
  Serial.println("%");
  Serial.print("R:");
  Serial.print(levelR);
  Serial.println("%");

  // Check if it's time to send a notification
  if (millis() - lastNotificationTime > notificationInterval) {
    checkAndNotify(levelL, levelR);
    lastNotificationTime = millis();
  }

  // Waste separation
  int sensorValue = digitalRead(metal);
  unsigned int distance_s = sonarSeparate.ping();
  int distanceCm = distance_s / US_ROUNDTRIP_CM;

  if (sensorValue == HIGH && distanceCm <= 7) { // trash exists (metal)
    Serial.println("No metal detected");
    Serial.println(distanceCm);
    servo1.write(135); // Pan left
    delay(1000);
    servo1.write(90); //
  } else if (sensorValue == LOW && distanceCm <= 7) { // trash exists (non-metal)
    Serial.println("Metal detected");
    Serial.println(distanceCm);
    servo1.write(45); // Pan right
    delay(1000);
    servo1.write(90); //
  } else if (distanceCm > 8) { // No trash within 7 cm
    servo1.write(0); // Keep servo centered
    Serial.println("No trash");
    Serial.println(distanceCm);
  }

  delay(1000); // Adjust delay as needed
}

void checkAndNotify(int levelL, int levelR) {
  String message = "";
  if (levelL >= 80) {
    message += "Left bin is " + String(levelL) + "% full. Please empty soon.\n";
  }
  if (levelR >= 80) {
    message += "Right bin is " + String(levelR) + "% full. Please empty soon.\n";
  }
  
  if (message != "") {
    bot.sendMessage(CHAT_ID, message, "");
    Serial.println("Notification sent to Telegram");
  }
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        String command = String((char*)data);
        
        Serial.print("Received command: ");
        Serial.println(command);

        if (command == "forward") {
            moveForward();
        } else if (command == "backward") {
            moveBackward();
        } else if (command == "left") {
            turnLeft();
        } else if (command == "right") {
            turnRight();
        } else if (command == "stop") {
            stopMotors();
        } else if (command.startsWith("speed")) {
            speed = command.substring(5).toInt();
            setSpeed(speed);
        }
    }
}

void moveForward() {
    Serial.println("Moving forward");
    digitalWrite(motorA1, HIGH);
    digitalWrite(motorA2, LOW);
    digitalWrite(motorB1, HIGH);
    digitalWrite(motorB2, LOW);
}

void moveBackward() {
    Serial.println("Moving backward");
    digitalWrite(motorA1, LOW);
    digitalWrite(motorA2, HIGH);
    digitalWrite(motorB1, LOW);
    digitalWrite(motorB2, HIGH);
}

void turnLeft() {
    Serial.println("Turning left");
    digitalWrite(motorA1, HIGH);
    digitalWrite(motorA2, LOW);
    digitalWrite(motorB1, LOW);
    digitalWrite(motorB2, HIGH);
}

void turnRight() {
    Serial.println("Turning right");
    digitalWrite(motorA1, LOW);
    digitalWrite(motorA2, HIGH);
    digitalWrite(motorB1, HIGH);
    digitalWrite(motorB2, LOW);
}

void stopMotors() {
    digitalWrite(motorA1, LOW);
    digitalWrite(motorA2, LOW);
    digitalWrite(motorB1, LOW);
    digitalWrite(motorB2, LOW);
}

void setSpeed(int speed) {
    analogWrite(enableA, speed);
    analogWrite(enableB, speed);
}

String getHTML() {
    String html = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Waste Bin and Car Control</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; }
        button { font-size: 18px; margin: 10px; padding: 10px 20px; }
        #speed { width: 200px; }
        .waste-bins { display: flex; justify-content: space-around; margin-bottom: 50px; }
        .bin { width: 100px; height: 200px; border: 2px solid black; position: relative; }
        .level { width: 100%; position: absolute; bottom: 0; background-color: #4CAF50; transition: height 0.5s; }
        .label { margin-top: 10px; font-weight: bold; }
        .controls { margin-top: 50px; }
    </style>
</head>
<body>
    <h1>ESP32 Waste Bin and Car Control</h1>
    <div class="waste-bins">
        <div>
            <div class="bin"><div id="levelL" class="level"></div></div>
            <div id="levelLLabel" class="label">0%</div>
            <p>Left Bin</p>
        </div>
        <div>
            <div class="bin"><div id="levelR" class="level"></div></div>
            <div id="levelRLabel" class="label">0%</div>
            <p>Right Bin</p>
        </div>
    </div>
    <div class="controls">
        <h2>Car Control</h2>
        <div>
            <button onclick="sendCommand('forward')">Forward</button><br>
            <button onclick="sendCommand('left')">Left</button>
            <button onclick="sendCommand('stop')">Stop</button>
            <button onclick="sendCommand('right')">Right</button><br>
            <button onclick="sendCommand('backward')">Backward</button>
        </div>
        <div>
            <input type="range" id="speed" min="0" max="255" value="255" oninput="updateSpeed()">
            <p>Speed: <span id="speedValue">255</span></p>
        </div>
    </div>

    <script>
        const socket = new WebSocket('ws://' + window.location.hostname + '/ws');
        
        socket.onopen = function(event) {
            console.log('WebSocket connection established');
        };

        socket.onmessage = function(event) {
            const data = event.data.split(',');
            updateBin('levelL', data[0]);
            updateBin('levelR', data[1]);
        };

        function updateBin(id, level) {
            const binLevel = document.getElementById(id);
            const binLabel = document.getElementById(id + 'Label');
            binLevel.style.height = level + '%';
            binLabel.innerText = level + '%';
        }

        function sendCommand(command) {
            socket.send(command);
        }

        function updateSpeed() {
            const speed = document.getElementById('speed').value;
            document.getElementById('speedValue').innerText = speed;
            socket.send('speed' + speed);
        }
    </script>
</body>
</html>
)rawliteral";
  return html;
}