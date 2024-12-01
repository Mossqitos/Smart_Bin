#define BLYNK_TEMPLATE_ID "TMPL6bzWWVz0V"
#define BLYNK_TEMPLATE_NAME "Blynk Smart Dustbin"
#define BLYNK_AUTH_TOKEN "m2AwC6IOqkGFUYams3erIs0IK67HLK72"
#define BLYNK_PRINT Serial

#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define RXD1 14 //รับว่าขยะเต็มไหม
#define TXD1 27

#define RXD2 33 //รับว่าเป็นคนไหม
#define TXD2 32

#define RXD3 12 
#define TXD3 13 //ให้ultrasonic ทำงาาน

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Tamao"; // wifi or hotspot name
char pass[] = "12345679"; // wifi or hotspot pass

BlynkTimer timer;

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define ESP_BAUD 115200
#define IR_PIN 23
#define SERVO_PIN 22
#define NGANG_PIN 21
HardwareSerial humanSerial(2);
HardwareSerial binSerial(1);
HardwareSerial ultraSerial(3);
Servo servo;
bool objectDetected = false;

void setup() {
  Serial.begin(115200);
  humanSerial.begin(ESP_BAUD, SERIAL_8N1, RXD2, TXD2);
  binSerial.begin(ESP_BAUD, SERIAL_8N1, RXD1, TXD1);
  ultraSerial.begin(ESP_BAUD, SERIAL_8N1, RXD3, TXD3);
  Blynk.begin(auth, ssid, pass);
  pinMode(IR_PIN, INPUT);
  pinMode(NGANG_PIN, INPUT);
  servo.attach(SERVO_PIN);
  servo.write(0);
  Serial.println("Serial 2 started at 115200 baud rate");
  timer.setInterval(1000L, SMESensor);
}

void SMESensor() {
  int irState = digitalRead(IR_PIN);
  Serial.print("irState : ");
  Serial.println(irState);
  if(irState == 0 && !objectDetected) {
    //ส่งค่าirไปยังอีกบอร์ด
    humanSerial.println(true);
    binSerial.println(true);
    Serial.println("send done");
    while (humanSerial.available() > 0 && binSerial.available() > 0 && !objectDetected) {
      Serial.print("HUMAN status: ");
      char isHuman = humanSerial.read();
      Serial.println(isHuman);
      Serial.print("BIN status: ");
      char isBin = binSerial.read();
      Serial.println(isBin);

      Serial.print("HUMAN status: ");
      Serial.println(humanSerial.read());
      Serial.print("BIN status: ");
      Serial.println(binSerial.read());

      Serial.print("HUMAN status: ");
      Serial.println(humanSerial.read());
      Serial.print("BIN status: ");
      Serial.println(binSerial.read());

      humanSerial.println(false);
      binSerial.println(false);
      objectDetected = true;
      if(isHuman == '1') {
        if(isBin == '1') {
          ultraSerial.println(true);
          Serial.println("HIII");
          servo.write(90);
          Blynk.virtualWrite(V2, 90);
          delay(3000);
          servo.write(0);
          int ngangState = digitalRead(NGANG_PIN);
          Serial.println(ngangState);
          if(ngangState == 0) {
            Serial.println("Bin didn't close");
            Blynk.virtualWrite(V2, 1);
          } else {
            Blynk.virtualWrite(V2, 0);
          }
          ultraSerial.println(false);
        }
        else {
          Serial.print("BIN TEM I KWAUY");
        }
      } else {
        Serial.println("U R NOT HUMAN    U R GAY");
      }
    }
  }
  if(irState == 1 && objectDetected) {
    objectDetected = false;
  }
}

void loop() {
  Blynk.run();
  timer.run();
}
