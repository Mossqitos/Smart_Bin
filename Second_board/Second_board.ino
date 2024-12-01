#define BLYNK_TEMPLATE_ID "TMPL6bzWWVz0V"
#define BLYNK_TEMPLATE_NAME "Blynk Smart Dustbin"
#define BLYNK_AUTH_TOKEN "m2AwC6IOqkGFUYams3erIs0IK67HLK72"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Tamao"; // wifi or hotspot name
char pass[] = "12345679"; // wifi or hotspot pass

BlynkTimer timer;

#define RXD2 33
#define TXD2 32

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
HardwareSerial humanSerial(2);
HardwareSerial binSerial(1);
HardwareSerial ultraSerial(3);

void setup() {
  Serial.begin(115200) ;
  humanSerial.begin(ESP_BAUD, SERIAL_8N1, RXD2, TXD2);
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
    binLevel=map(distance, 21, 0, 0,100); // 21 ปรับเป็นความสูง dustbin
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
  while (humanSerial.available() >0) {
    char sendIt = humanSerial.read();
    Serial.println(sendIt);
    if(sendIt == '1') {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(5);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      // store in distance var
      distance = duration * 0.034 / 2; //formula to calculate the distance for ultrasonic sensor
      binLevel=map(distance, 21, 0, 0,100); // 21 ปรับเป็นความสูง dustbin
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
      Serial.print("Measured Distance: ");
      Serial.print(distance);
      Serial.println(" cm");
      
      Serial.print("Bin Level: ");
      Serial.print(binLevel);
      Serial.println(" %");
      humanSerial.println(true);
      if(binLevel < 100) {
        Serial.println("good");
        binSerial.println(true);
        Serial.println("Realtime ultrasonic function started.");\
        timer.enable(ultrasonicTimerId);
        startMillis = millis();
        isRunning = true; 
        Serial.println("Realtime ultrasonic function stoped.");
      } else {
        binSerial.println(false);
      }
      // // while ( ultraSerial.available() > 0) {
      //   if(doUltra == '1') {
      //     // timer.enable(ultrasonicTimerId);
      //     Serial.println("Realtime ultrasonic function started.");
      //     delay(3000);
      //     // timer.disable(ultrasonicTimerId);
      //     Serial.println("Realtime ultrasonic function stoped.");
      //   }
      // // }
    }
    humanSerial.read();
    humanSerial.read();
    humanSerial.read();
    humanSerial.read();
    humanSerial.read();
    humanSerial.read();
    humanSerial.read();
  }

}

void loop() {
  Blynk.run();
  timer.run();

  if (isRunning) {
    if (millis() - startMillis >= 3000) { // Check if 3 seconds have passed
      timer.disable(ultrasonicTimerId); // Stop the ultrasonic timer
      isRunning = false; // Reset the flag
      Serial.println("Realtime ultrasonic function stopped.");
    }
  }
}
