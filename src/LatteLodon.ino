#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESP32Servo.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

WebServer server(80);
Servo myservo;
const int SERVO_PIN = 18;
const int BUTTON_PIN = 4;

bool feeding = false;
String lastStatus = "Idle";

void feed() {

  feeding = true;
  lastStatus = "Feeding";

  Serial.println("Feeding...");
  myservo.write(90);

  delay(1000);

  myservo.write(0);
  delay(500);

  Serial.println("Feed Done");
  feeding = false;
  lastStatus = "Last Feed Success";
}

void sendHtml() {

  String response = R"(
    <!DOCTYPE html><html>
    <head>
      <title>LatteLodon Feeder</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        html {
          font-family: sans-serif;
          text-align: center;
        }
        body {
          margin-top: 40px;
        }
        h1 {
          color: #333;
        }
        .card {
          max-width: 400px;
          margin: auto;
          padding: 20px;
          border-radius: 16px;
          box-shadow: 0 0 10px rgba(0,0,0,0.2);
        }
        .btn {
          background-color: #4CAF50;
          border: none;
          color: white;
          padding: 16px 32px;
          font-size: 24px;
          border-radius: 12px;
          text-decoration: none;
          display: inline-block;
          margin-top: 20px;
        }
        .status {
          margin-top: 20px;
          font-size: 20px;
        }
      </style>
    </head>
    <body>
      <div class="card">
        <h1>LatteLodon Feeder</h1>
        <p>ESP32 Automatic Pet Feeder</p>
        <a href="/feed" class="btn">
          FEED NOW
        </a>
        <div class="status">
          STATUS_TEXT
        </div>
      </div>
    </body>
    </html>
  )";

  response.replace("STATUS_TEXT", lastStatus);
  server.send(200, "text/html", response);
}

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("BOOTING...");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  myservo.attach(SERVO_PIN);
  myservo.write(0);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", sendHtml);
  server.on("/feed", []() {
    Serial.println("Web Feed Request");
    feed();
    sendHtml();
  });

  server.on("/status", []() {
    server.send(200, "text/plain", lastStatus);
  });

  server.begin();
  Serial.println("HTTP Server Started");
}

void loop() {

  server.handleClient();

  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Manual Button Feed");
    feed();
    delay(300);

    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(10);
    }
  }

  delay(2);
}