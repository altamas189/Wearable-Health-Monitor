#define BLYNK_TEMPLATE_ID "TMPL3p7sIu8x9"
#define BLYNK_TEMPLATE_NAME "ELDERLY STUFF"
#define BLYNK_AUTH_TOKEN "ne535ooEtcJx4v-pUsI7WrkhFHBovYiB"
#define BLYNK_PRINT Serial

#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Venk101";
char pass[] = "28ehw8kf";

MPU9250_asukiaaa mySensor;
MAX30105 particleSensor;

BlynkTimer timer;

#define BUZZER_PIN 13

bool beatDetected = false;
unsigned long lastBeat = 0;
int beatAvg;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  mySensor.setWire(&Wire);
  mySensor.beginAccel();
  mySensor.beginGyro();

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found. Check wiring.");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0xFF);  // Turn off RED
  particleSensor.setPulseAmplitudeIR(0xFF);   // Max IR brightness
  particleSensor.setSampleRate(100);          // 100 Hz
  particleSensor.setPulseWidth(411);          // Max pulse width

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(1000L, sendSensorData);
  timer.setInterval(100L, monitorHeartRate);
}

void sendSensorData() {
  mySensor.accelUpdate();

  float ax = mySensor.accelX();
  float ay = mySensor.accelY();
  float az = mySensor.accelZ();
  float magnitude = sqrt(ax * ax + ay * ay + az * az);

  Serial.print("Accel Magnitude: "); Serial.print(magnitude);

  if (magnitude > 2.5) {
    Serial.print(" | Fall Detected!");
    Blynk.logEvent("fall_detected", "Fall detected!");
    
    digitalWrite(BUZZER_PIN, HIGH);
    Blynk.virtualWrite(V9, 1); // Fall Indicator
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    Blynk.virtualWrite(V9, 0);
  }

  Blynk.virtualWrite(V0, ax);
  Blynk.virtualWrite(V1, ay);
  Blynk.virtualWrite(V2, az);

void monitorHeartRate() {
  long irValue = particleSensor.getIR();
  Serial.print(" | IR: "); 
  Blynk.virtualWrite(V8, magnitude);
}Serial.print(irValue);
  Blynk.virtualWrite(V6,irValue);
  if (irValue > 10000) {
    int ird= irValue/(615*2);
    Blynk.virtualWrite(V7, ird);

   if (checkForBeat(irValue)) {
      unsigned long delta = millis() - lastBeat;
      lastBeat = millis();

      float bpm = 60.0 / (delta / 1000.0);
      beatAvg = (beatAvg * 4 + (int)bpm) / 4;
     
      Serial.print(" | BPM: "); Serial.println(beatAvg);
      Blynk.virtualWrite(V7, beatAvg);

      if (ird < 50) {
        Blynk.logEvent("low_hr", "Low Heart Rate Detected!");
       
        Blynk.virtualWrite(V12, ird);
        digitalWrite(BUZZER_PIN, HIGH);
      }
   } else {
      Serial.print(" | BPM: "); Serial.println(beatAvg);
      Blynk.virtualWrite(V12, beatAvg);//
    }
  } else {
    Serial.println(" | Finger Not Detected");
    Blynk.virtualWrite(V7, 0);
  }
}

void loop() {
  Blynk.run();
  timer.run();
}
