#define BLYNK_TEMPLATE_ID   "TMPL3qS-okmbvD"
#define BLYNK_DEVICE_NAME   "tts"
#define BLYNK_AUTH_TOKEN    "YlqRQ-AFJYb76rCdZLcEUQkaGfpruut_"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>

// ---------- Pin Definitions ----------
#define SOIL_PIN     34
#define PH_PIN       35
#define NUTRIENT_PIN 32
#define CO2_PIN      33
#define DHT_PIN      15
#define PUMP_PIN     2
#define FAN_PIN      18

#define DHTTYPE      DHT22

// ---------- LCD (I2C) ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- DHT22 ----------
DHT dht(DHT_PIN, DHTTYPE);

// ---------- WiFi Credentials ----------
char ssid[] = "Airtel_anki_8343";       // my WiFi SSID
char pass[] = "Airfiber@121";           // my WiFi Password

// ---------- Blynk Auth ----------
char auth[] = BLYNK_AUTH_TOKEN;

// ---------- Timers ----------
BlynkTimer timer;

// ---------- Thresholds ----------
int soilThreshold = 1500;     // adjust for dry soil
int humidityThreshold = 40;   // % minimum humidity

// ---------- Sensor Read + Control ----------
void sendSensorData() {
  int soil = analogRead(SOIL_PIN);
  int ph   = analogRead(PH_PIN);
  int nut  = analogRead(NUTRIENT_PIN);
  int co2  = analogRead(CO2_PIN);

  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  // --- Send to Blynk ---
  Blynk.virtualWrite(V0, soil);
  Blynk.virtualWrite(V1, ph);
  Blynk.virtualWrite(V2, nut);
  Blynk.virtualWrite(V3, co2);
  Blynk.virtualWrite(V4, temp);
  Blynk.virtualWrite(V5, hum);

  // --- LCD Display ---
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T:"); lcd.print(temp);
  lcd.print(" H:"); lcd.print(hum);

  lcd.setCursor(0,1);
  lcd.print("Soil:"); lcd.print(soil);

  // --- Serial Monitor ---
  Serial.println("---- Sensor Data ----");
  Serial.print("Soil: "); Serial.println(soil);
  Serial.print("pH: "); Serial.println(ph);
  Serial.print("Nutrients: "); Serial.println(nut);
  Serial.print("CO2: "); Serial.println(co2);
  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Humidity: "); Serial.println(hum);

  // --- Automatic Control ---
  if (soil < soilThreshold) {
    digitalWrite(PUMP_PIN, HIGH);   // Pump ON
    Blynk.virtualWrite(V6, 1);      // update button
  } else {
    digitalWrite(PUMP_PIN, LOW);    // Pump OFF
    Blynk.virtualWrite(V6, 0);
  }

  if (hum < humidityThreshold) {
    digitalWrite(FAN_PIN, HIGH);    // Fan ON
    Blynk.virtualWrite(V7, 1);
  } else {
    digitalWrite(FAN_PIN, LOW);     // Fan OFF
    Blynk.virtualWrite(V7, 0);
  }
}

// ---------- Blynk Manual Override ----------
BLYNK_WRITE(V6) {  // Pump control
  int val = param.asInt();
  digitalWrite(PUMP_PIN, val);
}

BLYNK_WRITE(V7) {  // Fan control
  int val = param.asInt();
  digitalWrite(FAN_PIN, val);
}

void setup() {
  Serial.begin(115200);

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  dht.begin();
  lcd.init();
  lcd.backlight();

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(2000L, sendSensorData);  // every 2s
}

void loop() {
  Blynk.run();
  timer.run();
}