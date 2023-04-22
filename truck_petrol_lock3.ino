/*
 * Example for smart lock with feedback
 * 
 * The lock must give a feedback signal on pin defined in LOCK_STATE_PIN
 * Alternative: Use a contact sensor, indicating the current lock state
 * HIGH on LOCK_STATE_PIN means lock is locked
 * LOW on LOCK_STATE_PIN means lock is unlocked
 * 
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

// Uncomment the following line to enable serial debug output
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#ifdef ESP8266 
       #include <ESP8266WiFi.h>
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif
#include <Servo.h>
#include "SinricPro.h"
#include "SinricProLock.h"

#define WIFI_SSID         "JioFi2_A04AAA"    
#define WIFI_PASS         "buft2xmvez"
#define APP_KEY           "bc80294b-aa26-4b73-9d68-3ac25e72be48"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "52f20656-7b44-46f2-a7b2-99c410ee4bbb-80eeddf9-b528-45f5-a7f4-225f333e6b0a"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define LOCK_ID           "61060f89fc89cf405088c131"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define BAUD_RATE         9600                     // Change baudrate to your need

//#define LOCK_PIN          D4                       // PIN where the lock is connected to: HIGH = locked, LOW = unlocked
#define LOCK_STATE_PIN    13
#define onpin    14              // PIN where the lock feedback is connected to (HIGH:locked, LOW:unlocked)

Servo myservo;
bool lastLockState;
bool recordLockState;
unsigned long tanktimer = 0;
unsigned long tankperiod = 2000;
unsigned long timenow = 0;
unsigned long  waitperiod = 5000;

bool onLockState(String deviceId, bool &lockState) {
  Serial.printf("Device %s is %s\r\n", deviceId.c_str(), lockState?"locked":"unlocked");
  if(lockState == 0){
    digitalWrite(onpin, HIGH);
    myservo.attach(12);
    //myservo.write(180);                    
    myservo.write(0);
    delay(1000);
    timenow = millis();
    while( digitalRead(LOCK_STATE_PIN)==1 && millis() <= timenow + waitperiod ){delay(100);}
    digitalWrite(onpin, LOW);
    myservo.write(180);
    delay(1000);
    myservo.detach();
  }
  recordLockState =   lockState;
  return true;
}

void checkLockState() {
  
  if(millis() >= tanktimer + tankperiod){
   Serial.println("checking");
   bool currentLockState = digitalRead(LOCK_STATE_PIN);                                   
   if (currentLockState != recordLockState) {
   Serial.println("dont match");                                                                                                                             
   SinricProLock &myLock = SinricPro[LOCK_ID];                                            
   myLock.sendLockStateEvent(currentLockState);
   recordLockState = currentLockState;
  }
  tanktimer = millis();
  }

}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250); 
    time(67890);
    delay(6780);
  }
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro() {
  SinricProLock &myLock = SinricPro[LOCK_ID];
  myLock.onLockState(onLockState);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n");
  digitalWrite(onpin, HIGH);
  delay(100);
  digitalWrite(onpin, LOW);
  delay(100);
    digitalWrite(onpin, HIGH);
  delay(100);
  digitalWrite(onpin, LOW);
  delay(100);
    digitalWrite(onpin, HIGH);
  delay(100);
  digitalWrite(onpin, LOW);
  delay(100);
    digitalWrite(onpin, HIGH);
  delay(100);
  digitalWrite(onpin, LOW);}); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
   SinricPro.begin(APP_KEY, APP_SECRET);
}

void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
    pinMode(LOCK_STATE_PIN, INPUT);
    pinMode(onpin, OUTPUT);
    digitalWrite(onpin, HIGH);
    delay(100);
    myservo.attach(12);
    myservo.write(180);
    delay(1000);
    myservo.detach();
    digitalWrite(onpin, LOW);
     

  //pinMode(LOCK_PIN, OUTPUT);

  setupWiFi();
  setupSinricPro();
  tanktimer = millis();
  
}

void loop() {
  SinricPro.handle();
  checkLockState();
  
}
