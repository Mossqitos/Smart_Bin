// #define BLYNK_TEMPLATE_ID "TMPL6bzWWVz0V"
// #define BLYNK_TEMPLATE_NAME "Blynk Smart Dustbin"
// #define BLYNK_AUTH_TOKEN "m2AwC6IOqkGFUYams3erIs0IK67HLK72"

#define BLYNK_TEMPLATE_ID "TMPL6JY0-HgUW"
#define BLYNK_TEMPLATE_NAME "embed"
#define BLYNK_AUTH_TOKEN "ojSk5RFycVV4Xn7HuV8G2dqFPhmNSAjI"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Tamao"; // wifi or hotspot name
char pass[] = "12345679"; // wifi or hotspot pass

BlynkTimer timer;

#define RXD1 14
#define TXD1 27

#define RXD3 12
#define TXD3 13

#define echoPin 25
#define trigPin 26

long duration;
int distance; 
int binLevel=0;
int ultrasonicTimerId;
unsigned long startMillis;
bool isRunning = false;

#define ESP_BAUD 115200
HardwareSerial binSerial(1);
HardwareSerial ultraSerial(3);

void setup() {
  Serial.begin(115200) ;
  binSerial.begin(ESP_BAUD, SERIAL_8N1, RXD1, TXD1);
  ultraSerial.begin(ESP_BAUD, SERIAL_8N1, RXD3, TXD3);
  pinMode(trigPin, OUTPUT);  
  pinMode(echoPin, INPUT); 
  Blynk.begin(auth, ssid, pass);
  Serial.println("Serial 2 started at 115200 baud rate");
  timer.setInterval(1000L, SMESensor);
  ultrasonicTimerId = timer.setInterval(100L, ultrasonic);
  timer.disable(ultrasonicTimerId);
}

void ultrasonic() // measure height in dustbin
{
    // measure height
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    // store in distance var
    distance = duration * 0.034 / 2; //formula to calculate the distance for ultrasonic sensor
    binLevel=map(distance, 19, 0, 0,100); // 19 ปรับเป็นความสูง dustbin
    if(distance >21 ) {
      if(distance > 1000) {
        distance = 0;
        binLevel = 100;
      }
      else {
        distance = 21;
        binLevel = 0;
      }
    }
    Blynk.virtualWrite(V0, distance);
    Blynk.virtualWrite(V1, binLevel);

    Serial.print("Measured Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    
    Serial.print("Bin Level: ");
    Serial.print(binLevel);
    Serial.println(" %");
}
void SMESensor() {
  // put your main code here, to run repeatedly:
  while (binSerial.available() >0) {
    char sendIt = binSerial.read();
    Serial.println(sendIt);
    delay(500);
    if(sendIt == '1') {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(5);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      // store in distance var
      distance = duration * 0.034 / 2; //formula to calculate the distance for ultrasonic sensor
      binLevel=map(distance, 19, 0, 0,100); // 21 ปรับเป็นความสูง dustbin
      if(distance >19 ) {
        if(distance > 1000) {
          distance = 0;
          binLevel = 100;
        }
        else {
          distance = 19;
          binLevel = 0;
        }
      }
      Serial.print("Measured Distance: ");
      Serial.print(distance);
      Serial.println(" cm");
      
      Serial.print("Bin Level: ");
      Serial.print(binLevel);
      Serial.println(" %");
      if(binLevel < 90) {
        Serial.println("good");
        binSerial.println(true);
        delay(500);
        Serial.println("Realtime ultrasonic function started.");\
        timer.enable(ultrasonicTimerId);
        startMillis = millis();
        isRunning = true; 
        delay(3000);
        Serial.println("Realtime ultrasonic function stoped.");
      } else {
        binSerial.println(false);
      }
    }
    binSerial.read();
    binSerial.read();
    binSerial.read();
    binSerial.read();
    binSerial.read();
    binSerial.read();
    binSerial.read();
  }

}

void loop() {
  Blynk.run();
  timer.run();

  if (isRunning) {
    if (millis() - startMillis >= 5000) { // Check if 3 seconds have passed
      timer.disable(ultrasonicTimerId); // Stop the ultrasonic timer
      isRunning = false; // Reset the flag
      Serial.println("Realtime ultrasonic function stopped.");
    }
  }
}
