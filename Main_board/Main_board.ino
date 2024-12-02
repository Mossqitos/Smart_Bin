// #define BLYNK_TEMPLATE_ID "TMPL6bzWWVz0V"
// #define BLYNK_TEMPLATE_NAME "Blynk Smart Dustbin"
// #define BLYNK_AUTH_TOKEN "m2AwC6IOqkGFUYams3erIs0IK67HLK72"

#define BLYNK_TEMPLATE_ID "TMPL6JY0-HgUW"
#define BLYNK_TEMPLATE_NAME "embed"
#define BLYNK_AUTH_TOKEN "ojSk5RFycVV4Xn7HuV8G2dqFPhmNSAjI"

#define BLYNK_PRINT Serial

#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP_Mail_Client.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include "time.h"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL "beer.tarit@gmail.com"
#define AUTHOR_PASSWORD "tolv vmov gxgb rmsj"

#define RECIPIENT_EMAIL "thanakitmoss@gmail.com"

#define WIFI_SSID "Tamao"
#define WIFI_PASSWORD "12345679"

#define PROJECT_ID "embed-443407"
#define CLIENT_EMAIL "smartbin@embed-443407.iam.gserviceaccount.com"


void smtpCallback(SMTP_Status status);

#define RXD1 14 //รับว่าขยะเต็มไหม
#define TXD1 27

#define RXD3 12 
#define TXD3 13 //ให้ultrasonic ทำงาาน

int threshold = 3000;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Tamao"; // wifi or hotspot name
char pass[] = "12345679"; // wifi or hotspot pass
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;
String GOOGLE_SCRIPT_ID = "AKfycbyuVB_tDFAogT7WiTtPVmfveEFUizFMBbrrlNpSYHn3zykYqiGzSEju7JBml805nzZeKg";

BlynkTimer timer;

SMTPSession smtp;

#define MQ_PIN 19
#define ESP_BAUD 115200
#define IR_PIN 23
#define SERVO_PIN 22
#define NGANG_PIN 21
HardwareSerial binSerial(1);
HardwareSerial ultraSerial(3);
Servo servo;
bool objectDetected = false;



// Variable to save the epoch time
unsigned long epochTime;



void setup() {
  Serial.begin(115200);
  binSerial.begin(ESP_BAUD, SERIAL_8N1, RXD1, TXD1);
  ultraSerial.begin(ESP_BAUD, SERIAL_8N1, RXD3, TXD3);
  Blynk.begin(auth, ssid, pass);
  pinMode(MQ_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(NGANG_PIN, INPUT);
  servo.attach(SERVO_PIN);
  servo.write(0);

  // configTime(0, 0, ntpServer);



  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi not connected.");
    return;
  
  
  }
  // GSheet.setDebugLevel(1)

  
  Serial.println("Serial 2 started at 115200 baud rate");
  timer.setInterval(10000L, MQSensor);
  timer.setInterval(1000L, SMESensor);
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}



void MQSensor() {
  int sensorValue = digitalRead(MQ_PIN); // Read analog value from the MQ sensor
  Serial.print("Raw Sensor Value: ");
  Serial.println(sensorValue);

  if(sensorValue == 1) {
    Blynk.virtualWrite(V4, "Your bin is on fire");
    Serial.print("Bin on fire");
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

    String subject = " 🗑️ Your Smart Bin is detect some cause of fire.";

    String textMsg =  "Dear User,\n\n"
                      "This is a friendly reminder to check your Smart Bin to ensure it is operating efficiently.\n\n"
                      "Here are some tips to keep it in good condition:\n\n"
                      "Inspect for any unusual activity, such as fire alerts or malfunctions.\n\n"
                      "Please checking your bin there might be an accidently fire happen\n\n"
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
      Serial.printf("mail Sended");

    }
  } else {
    Blynk.virtualWrite(V4, "  ");
    Serial.println("your bin not on fire");
  }
}

void SMESensor() {
  int irState = digitalRead(IR_PIN);
  Serial.print("irState : ");
  Serial.println(irState);
  if(irState == 0 && !objectDetected) {
    //ส่งค่าirไปยังอีกบอร์ด
    binSerial.println(true);
    delay(500);
    Serial.println("send done");
    while (binSerial.available() > 0 && !objectDetected) {
      Serial.print("BIN status: ");
      char isBin = binSerial.read();
      Serial.println(isBin);
      objectDetected = true;

      Serial.print("BIN status: ");
      Serial.println(binSerial.read());

      Serial.print("BIN status: ");
      Serial.println(binSerial.read());

      binSerial.println(false);
      if(isBin == '1') {
        ultraSerial.println(true);
        Serial.println("HIII");
        servo.write(150);
        if (WiFi.status() == WL_CONNECTED) {
          // static bool flag = false;
          // struct tm timeinfo;
          // if (!getLocalTime(&timeinfo)) {
          //   Serial.println("Failed to obtain time");
          //   return;
          // }
          // char timeStringBuff[50]; //50 chars should be enough
          // strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
          // String asString(timeStringBuff);
          // asString.replace(" ", "-");
          // Serial.print("Time:");
          // Serial.println(asString);
          // String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+"sensor=" + asString + "&date=" + "urbinopen";
          // Serial.print("POST data to spreadsheet:");
          // Serial.println(urlFinal);
          // HTTPClient http;
          // http.begin(urlFinal.c_str());
          // http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
         
          Blynk.virtualWrite(V2, 150);
          delay(5000);
          servo.write(0);
          Blynk.virtualWrite(V2, 0);
          ultraSerial.println(false);
          delay(2000);
          int ngangState = digitalRead(NGANG_PIN);
          Serial.println(ngangState);
          
          if(ngangState == 0) {
            MailClient.networkReconnect(true);
            Blynk.virtualWrite(V3, "Bin didn't closed");
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

            String subject = " 🗑️ Your Smart Bin can not close perfectly, there might be something struct.";

            String textMsg =  "Dear User,\n\n"
                              "This is a friendly reminder to check your Smart Bin to ensure it is operating efficiently.\n\n"
                              "Here are some tips to keep it in good condition:\n\n"
                              "Ensure the lid is properly closed after use.\n\n"
                              "Please checking your bin there might something struct that lead to bin does not close perfectly\n\n"
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
              Serial.printf("mail Sended");
            }
          } else {
            Blynk.virtualWrite(V3, "Bin closed");
            delay(2000);
          }
        }
      }
      else {
        Serial.print("BIN TEM I KWAUY");
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
