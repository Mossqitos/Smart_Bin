#define BLYNK_TEMPLATE_ID "TMPL6bzWWVz0V"
#define BLYNK_TEMPLATE_NAME "Blynk Smart Dustbin"
#define BLYNK_AUTH_TOKEN "m2AwC6IOqkGFUYams3erIs0IK67HLK72"
#define BLYNK_PRINT Serial

#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP_Mail_Client.h>
#include <Arduino.h>

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL "beer.tarit@gmail.com"
#define AUTHOR_PASSWORD "tolv vmov gxgb rmsj"

#define RECIPIENT_EMAIL "thanakitmoss@gmail.com"

void smtpCallback(SMTP_Status status);

#define RXD1 14 //รับว่าขยะเต็มไหม
#define TXD1 27

#define RXD2 33 //รับว่าเป็นคนไหม
#define TXD2 32

#define RXD3 12 
#define TXD3 13 //ให้ultrasonic ทำงาาน

int threshold = 300;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Tamao"; // wifi or hotspot name
char pass[] = "12345679"; // wifi or hotspot pass

BlynkTimer timer;

SMTPSession smtp;

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define MQ_PIN 19
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
  pinMode(MQ_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(NGANG_PIN, INPUT);
  servo.attach(SERVO_PIN);
  servo.write(0);

  

  Serial.println("Serial 2 started at 115200 baud rate");
  timer.setInterval(1000L, MQSensor);
  timer.setInterval(1000L, SMESensor);
}

void MQSensor() {
  int MQData = analogRead(MQ_PIN);
  Serial.print("MQ Value: ");
  Serial.println(MQData);

  if(MQData > threshold) {
    MailClient.networkReconnect(true);

    smtp.debug(1);

    Session_Config config;

    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASSWORD;
    config.login.user_domain = "";

    config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
    config.time.gmt_offset = 3;
    config.time.day_light_offset = 0;

    SMTP_Message message;

    message.sender.name = F("SmartBIN");
    message.sender.email = AUTHOR_EMAIL;
    message.subject = F("Notification From Bin");
    message.addRecipient(F("Beer"), RECIPIENT_EMAIL);

    String subject = " 🗑️ Reminder: Time to Check Your Smart Bin!"

    String textMsg = "Dear User,\n\n"
    "This is a friendly reminder to check your Smart Bin to ensure it is operating efficiently.\n\n"
    "Here are some tips to keep it in good condition:\n\n"
    E"mpty the bin regularly to prevent overfilling.\n\n"
    "Ensure the lid is properly closed after use.\n\n"
    "Inspect for any unusual activity, such as fire alerts or malfunctions.\n\n"
    "Keeping your Smart Bin maintained helps promote hygiene and safety.\n\n\n\n"

    "Thank you for being a valued Smart Bin user!\n\n";
    message.subject = subject;
    message.text.content = textMsg.c_str();
    message.text.charSet = "us-ascii";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

    if (!smtp.connect(&config)){
      ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      return;
    }

    if (!smtp.isLoggedIn()){
      Serial.println("\nNot yet logged in.");
    }
    else{
      if (smtp.isAuthenticated())
        Serial.println("\nSuccessfully logged in.");
      else
        Serial.println("\nConnected with no Auth.");
    }
            
    if (!MailClient.sendMail(&smtp, &message)) {
      ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    } else {
      Serial.printf("mail Sended")
    }
  }
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
            MailClient.networkReconnect(true);

            smtp.debug(1);

            Session_Config config;

            config.server.host_name = SMTP_HOST;
            config.server.port = SMTP_PORT;
            config.login.email = AUTHOR_EMAIL;
            config.login.password = AUTHOR_PASSWORD;
            config.login.user_domain = "";

            config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
            config.time.gmt_offset = 3;
            config.time.day_light_offset = 0;

            SMTP_Message message;

            message.sender.name = F("SmartBIN");
            message.sender.email = AUTHOR_EMAIL;
            message.subject = F("Notification From Bin");
            message.addRecipient(F("Beer"), RECIPIENT_EMAIL);

            String subject = " 🗑️ Reminder: Time to Check Your Smart Bin!"

            String textMsg = "Dear User,\n\n"
            "This is a friendly reminder to check your Smart Bin to ensure it is operating efficiently.\n\n"
            "Here are some tips to keep it in good condition:\n\n"
            E"mpty the bin regularly to prevent overfilling.\n\n"
            "Ensure the lid is properly closed after use.\n\n"
            "Inspect for any unusual activity, such as fire alerts or malfunctions.\n\n"
            "Keeping your Smart Bin maintained helps promote hygiene and safety.\n\n\n\n"

            "Thank you for being a valued Smart Bin user!\n\n";
            message.subject = subject;
            message.text.content = textMsg.c_str();
            message.text.charSet = "us-ascii";
            message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

            message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
            message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

            if (!smtp.connect(&config)){
              ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
              return;
            }

            if (!smtp.isLoggedIn()){
              Serial.println("\nNot yet logged in.");
            }
            else{
              if (smtp.isAuthenticated())
                Serial.println("\nSuccessfully logged in.");
              else
                Serial.println("\nConnected with no Auth.");
            }
            
            if (!MailClient.sendMail(&smtp, &message)) {
              ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
            } else {
              Serial.printf("mail Sended")
            }
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
