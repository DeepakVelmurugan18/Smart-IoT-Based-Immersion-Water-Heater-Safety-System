// smart immersion water heater.............
#define BLYNK_TEMPLATE_ID "TMPL3jLL3htmp"
#define BLYNK_TEMPLATE_NAME "waterheater"
#define BLYNK_AUTH_TOKEN "PvL6skaIRySnR-Cx7mMLwEy0ohTfiZOp"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// ----- Blynk Configuration -----

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Deepak";
char pass[] = "Deepak18";

// ----- Pin definitions -----
#define ONE_WIRE_BUS D4      // DS18B20 data pin
#define TRIG_PIN D5          // Ultrasonic Trigger
#define ECHO_PIN D6          // Ultrasonic Echo
#define RELAY_PIN D7         // Relay control
#define BUZZER_PIN D8        // Buzzer

// ----- Relay logic -----
#define RELAY_ON  LOW    // Active LOW relay: LOW = ON, HIGH = OFF
#define RELAY_OFF HIGH

// ----- Objects -----
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

String scrollMsg = "  Object Detected, Heater OFF  ";

void setup() {
  Serial.begin(9600);
  sensors.begin();
  lcd.init();
  lcd.backlight();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, RELAY_ON);   // Heater starts ON
  digitalWrite(BUZZER_PIN, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Smart Water Heater");
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi...");
  Serial.println("Connecting to WiFi...");
  Blynk.begin(auth, ssid, pass);
  lcd.clear();
}

void loop() {
  Blynk.run();

  // --- Read Temperature ---
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  // Handle invalid readings
  if (tempC == -127.0 || tempC == 85.0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp Sensor Error");
    Serial.println("Temperature sensor error!");
    delay(1000);
    return;
  }

  tempC = abs(tempC);  // Ensure no negative values

  // --- Read Distance ---
  long duration, distance;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH, 40000); // timeout for longer range
  distance = duration * 0.034 / 2;           // Convert to cm

  // --- Display on Serial Monitor ---
  Serial.print("Temp: ");
  Serial.print(tempC);
  Serial.print(" °C | Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // ---------- CONTROL LOGIC ----------
  String heaterStatus = "";
  String buzzerStatus = "";

  // 1️⃣ Temperature safety cutoff
  if (tempC >= 80.0) {
    digitalWrite(RELAY_PIN, RELAY_OFF);  // Heater OFF
    digitalWrite(BUZZER_PIN, HIGH);      // Buzzer ON
    heaterStatus = "OFF";
    buzzerStatus = "ON";

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp:");
    lcd.print(tempC);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.print("Heater OFF (80C)");
    Serial.println("Heater OFF (Temp >= 80C)");
    delay(3000);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // 2️⃣ Object detected (< 100 cm)
  else if (distance > 0 && distance < 100) {
    digitalWrite(RELAY_PIN, RELAY_OFF);   // Heater OFF
  for (int i = 0; i < 5; i++) {   // 5 fast beeps
  digitalWrite(BUZZER_PIN, HIGH);
  delay(300);
  digitalWrite(BUZZER_PIN, LOW);
  delay(200);
}


    heaterStatus = "OFF";
    buzzerStatus = "ALERT";

    // Scrolling message
    for (int pos = 0; pos < scrollMsg.length(); pos++) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.print(tempC);
      lcd.print((char)223);
      lcd.print("C");
      lcd.setCursor(0, 1);
      for (int i = 0; i < 16; i++) {
        int index = (pos + i) % scrollMsg.length();
        lcd.print(scrollMsg[index]);
      }
      delay(300);
    }
    Serial.println("Object detected (<=100cm) -> Heater OFF");
  }

  // 3️⃣ Normal condition
  else {
    digitalWrite(RELAY_PIN, RELAY_ON);    // Heater ON
    digitalWrite(BUZZER_PIN, LOW);
    heaterStatus = "ON";
    buzzerStatus = "OFF";

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp:");
    lcd.print(tempC);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.print("Heater: ON      ");
    Serial.println("Heater ON (Normal)");
    delay(1000);
  }

  // --- Send data to Blynk ---
  Blynk.virtualWrite(V0, tempC);           // Temperature
  Blynk.virtualWrite(V1, distance);        // Distance
  Blynk.virtualWrite(V2, heaterStatus);    // Heater Status
  Blynk.virtualWrite(V3, buzzerStatus);    // Buzzer Status
}

