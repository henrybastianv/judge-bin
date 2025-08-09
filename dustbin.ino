#include <ESP32Servo.h>

// Function prototype
void sendCommand(uint8_t command, uint8_t param1, uint8_t param2 = 0);

// Ultrasonic Sensor Pins
const int trigPin = 5;
const int echoPin = 18;

// Servo Pin
const int servoPin = 13;
Servo myservo;

// MP3 Player Serial
#include "HardwareSerial.h"
HardwareSerial mp3Serial(1); // Use UART1
#define MP3_RX 16
#define MP3_TX 17

// Distance Threshold
int distanceThreshold = 20;
bool isOpen = false;
unsigned long previousMillis = 0;
const long closeDelay = 3000;  // ms

void setup() {
  Serial.begin(115200);
  mp3Serial.begin(9600, SERIAL_8N1, MP3_RX, MP3_TX);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  myservo.attach(servoPin);
  myservo.write(0); // closed

  // MP3 Player init
  sendCommand(0x3F, 0); // Init
  delay(500);
  sendCommand(0x06, 0x00, 0x1E); // Volume 30 (max)

  Serial.println("Smart Dustbin Ready");
}

void loop() {
  int distance = readDistance();
  unsigned long currentMillis = millis();

  if (distance < distanceThreshold && !isOpen) {
    openLid();
    isOpen = true;
    previousMillis = currentMillis;
  }

  if (isOpen && (currentMillis - previousMillis >= closeDelay)) {
    closeLid();
    isOpen = false;
  }
}

int readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;
  return distance;
}

void openLid() {
  myservo.write(90); // Open
  Serial.println("Lid opened");
}

void closeLid() {
  myservo.write(0); // Closed
  Serial.println("Lid closed");
  playSound(1); // Play track001.mp3
}

void playSound(uint8_t track) {
  sendCommand(0x03, 0x00, track); // Play specific track
}

// Send command to MP3
void sendCommand(uint8_t command, uint8_t param1, uint8_t param2) {
  uint8_t cmd[10] = {
    0x7E, 0xFF, 0x06, command, 0x00, param1, param2, 0x00, 0x00, 0xEF
  };

  uint16_t checksum = 0 - (cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5] + cmd[6]);
  cmd[7] = (checksum >> 8) & 0xFF;
  cmd[8] = checksum & 0xFF;

  for (int i = 0; i < 10; i++) {
    mp3Serial.write(cmd[i]);
  }
}
