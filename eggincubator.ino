#include <Adafruit_GFX.h>
#include <SH1106.h>
#include "DHT.h"
#include <Servo.h>

// -----------------------------
// PIN SETUP
// -----------------------------
int fanPin = 6;
int fanSpeed = 150;

int atomizerPin = 7;
int heatPadPin = 5;

// -----------------------------
// SENSOR + DISPLAY
// -----------------------------
DHT dht(2, DHT11);  //tenp hum sensor on pin 2
SH1106 display;     // SDA on A4, SCL on A5

float tempF = 0;
float hum = 0;

unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 2000;

// -----------------------------
// HUMIDIFIER CONTROL
// -----------------------------
bool humidifierState = false;

// -----------------------------
// HEATING PAD CONTROL
// -----------------------------
bool heatOn = false;

// -----------------------------
// SERVO CONTROL
// -----------------------------
Servo myServo;
const int servoPositions[] = { 120, 90, 60, 90 };
int servoIndex = 0;
unsigned long lastServoMove = 0;
const unsigned long servoInterval = 7200000;

// ========================================================
// SETUP
// ========================================================
void setup() {
  Serial.begin(9600);

  pinMode(atomizerPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(heatPadPin, OUTPUT);

  analogWrite(fanPin, fanSpeed);  // gentle airflow

  dht.begin();

  myServo.attach(9);  //servo on pin 9
  myServo.write(90);

  display.begin();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Init...");
  display.display();
}

// ========================================================
// LOOP
// ========================================================
void loop() {
  unsigned long now = millis();

  // -----------------------------------
  // SERVO (egg turner)
  // -----------------------------------
  if (now - lastServoMove >= servoInterval) {
    myServo.write(servoPositions[servoIndex]);
    servoIndex = (servoIndex + 1) % 4;
    lastServoMove = now;
  }

  // -----------------------------------
  // READ SENSORS
  // -----------------------------------
  if (now - lastSensorRead >= sensorInterval) {
    float tempC = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(tempC) && !isnan(h)) {
      tempF = tempC * 9.0 / 5.0 + 32.0;
      hum = h;
    }

    lastSensorRead = now;
  }

  // ========================================================
  //          HEATING PAD CONTROL (100.5°F target)
  // ========================================================
  if (tempF > 100.7) {
    heatOn = false;
  } else if (tempF < 100.3) {
    heatOn = true;
  }
  digitalWrite(heatPadPin, heatOn);


  // ========================================================
  //          HUMIDIFIER CONTROL (50–55%)
  // ========================================================
  if (hum > 55) {
    humidifierState = false;
    digitalWrite(atomizerPin, LOW);
  } else if (hum < 50) {
    humidifierState = true;
    digitalWrite(atomizerPin, HIGH);
  }

  // -----------------------------------
  // DISPLAY OLED
  // -----------------------------------
  display.clearDisplay();
  display.setTextSize(2);

  // Labels
  display.setCursor(0, 0);
  display.print("TempF");
  display.setCursor(80, 0);
  display.print("%Hum");

  // Values
  display.setCursor(0, 20);
  display.print(tempF, 1);
  display.setCursor(80, 20);
  display.print(hum, 1);

  // Status indicators (small text)
  display.setTextSize(1);
  display.setCursor(0, 48);
  display.print("HEAT:");

  display.setCursor(35, 48);
  display.print(heatOn ? "ON " : "OFF");

  display.setCursor(70, 48);
  display.print("HUM:");

  display.setCursor(100, 48);
  display.print(humidifierState ? "ON " : "OFF");

  display.display();

  // -----------------------------------
  // DEBUG OUTPUT
  // -----------------------------------
  Serial.print("TempF: ");
  Serial.print(tempF);
  Serial.print("  Hum: ");
  Serial.print(hum);
  Serial.print("  Heat: ");
  Serial.print(heatOn);
  Serial.print("  Humid: ");
  Serial.println(humidifierState);
}
