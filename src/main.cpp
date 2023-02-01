#include "finger.h"

#include <WiFi.h>
#include <NetBIOS.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include "Update.h"

#include <Ticker.h>
#include "time.h"

#include "espnow.h"
#include "config.h"
#include "wifi_conf.h"
#include "times.h"
#include "web_server.h"

#include <Arduino_JSON.h>

#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif

#include "SI7021.h"
#include "ds18b.h"

#define LED 2
//#define LED 5
#define DOOR 32

volatile SemaphoreHandle_t timerSemaphore;
volatile SemaphoreHandle_t timerSemaphoreInt;


Ticker dz;
bool upd_oled = false;

TaskHandle_t SenderTask;
SemaphoreHandle_t Mutex_cnf_lan;
SemaphoreHandle_t Mutex_cnf_server;
SemaphoreHandle_t Mutex_cnf_time;
SemaphoreHandle_t Mutex_cnf_finger;
SemaphoreHandle_t Mutex_cnf_now;
SemaphoreHandle_t Mutex_value;
SemaphoreHandle_t Mutex_file;
SemaphoreHandle_t Mutex_send_now;
SemaphoreHandle_t Mutex_si_measure;
SemaphoreHandle_t sendSemaphore;
SemaphoreHandle_t openSemaphore;
SemaphoreHandle_t addSemaphore;


void sender(void * parameter);
void getID(void);

extern int freeIdFinger;
extern volatile status_figner figner_web;
extern fingers_cnf conf_finger[2][MAXFINGER];

void opendoor(void){
  Serial.println("Open door!");
  digitalWrite(DOOR, HIGH);
  vTaskDelay(500);
  digitalWrite(DOOR, LOW);
}

void gtm(void) {
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}


void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  pinMode(DOOR, OUTPUT);
  digitalWrite(DOOR, LOW);
  pinMode(GPIOpin, INPUT_PULLUP);

  Serial.begin(115200);
  //Serial2.begin(57600, SERIAL_8N1, RXD2, TXD2);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }

  //test_led();
  //finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 255, FINGERPRINT_LED_PURPLE);
  finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_PURPLE);


  Mutex_cnf_lan = xSemaphoreCreateMutex();
  Mutex_cnf_server = xSemaphoreCreateMutex();
  Mutex_cnf_time = xSemaphoreCreateMutex();
  Mutex_cnf_finger = xSemaphoreCreateMutex();
  Mutex_cnf_now = xSemaphoreCreateMutex();
  Mutex_value = xSemaphoreCreateMutex();
  Mutex_file = xSemaphoreCreateMutex();
  Mutex_si_measure = xSemaphoreCreateMutex();
  timerSemaphore = xSemaphoreCreateBinary();
  sendSemaphore = xSemaphoreCreateBinary();
  openSemaphore = xSemaphoreCreateBinary();
  addSemaphore = xSemaphoreCreateBinary();

  scan_ap();
  initconfig();
  wifi_init();
  init_esp_now();
  init_web();
  timerSemaphoreInt = xSemaphoreCreateBinary();
  dz.attach(1, gtm);
  times_init();
  init_temp_si();
  init_ds();
  measuring();
  measuring_ds();

  attachInterrupt(GPIOpin, getID, FALLING);
  // attachInterrupt(GPIOpin, getID, RISING);
  
  //detachInterrupt(GPIOpin);

  xTaskCreatePinnedToCore(sender,"Task1",10000,NULL,1,&SenderTask,0);                         
  delay(500);
  digitalWrite(LED, LOW);
}

void loop() {
  while(1){
    server.handleClient();
    vTaskDelay(5);
  }
}


void sender(void * parameter){
  boolean act_fingther=false, fingther_ok=false;
  uint8_t figner_web_cnt=0, mesur_cnt=0;
  while(1){
    if (xSemaphoreTake(openSemaphore, 0) == pdTRUE){  
      finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_BLUE, 10);
      opendoor();
    }
    if (xSemaphoreTake(addSemaphore, 0) == pdTRUE){  
      if (getFingerprintEnroll(freeIdFinger) == FINGERPRINT_OK){
        if( xSemaphoreTake( Mutex_cnf_finger, portMAX_DELAY ) == pdTRUE ){
          conf_finger[1][freeIdFinger].act =  true;
          if (memcmp ((void*)&conf_finger[0][freeIdFinger], (void*)&conf_finger[1][freeIdFinger], sizeof(finger_cnf)) != 0){
            memcpy((void*)&conf_finger[0][freeIdFinger], (void*)&conf_finger[1][freeIdFinger], sizeof(fingers_cnf));
            xSemaphoreGive( Mutex_cnf_finger );
            if (!saveconfig(finger_cnf)) {
              Serial.println("Failed to save config");
            } else {
              Serial.println("Config saved");
            }            
          }else{
            Serial.println("No saved");
            xSemaphoreGive( Mutex_cnf_finger );
          }
        }
        figner_web = figner_ok;
      }else{
        figner_web = figner_err;
        finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
      }
      attachInterrupt(GPIOpin, getID, FALLING);
    }
    if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE){  
      if ((figner_web == figner_ok) || (figner_web == figner_err)){
        if (figner_web_cnt >= 40){
          figner_web = figner_none;
          figner_web_cnt=0;
        }else{
          figner_web_cnt++;
        }
      }else if (figner_web == figner_start){
        figner_web_cnt=0;
      }
      if (mesur_cnt >= 9){
        measuring();
        measuring_ds();
        mesur_cnt=0;
      }else{
        mesur_cnt++;
      }
    }
    // if (!digitalRead(GPIOpin)){
    if (xSemaphoreTake(timerSemaphoreInt, 0) == pdTRUE){
      digitalWrite(LED, HIGH);  
      act_fingther = true; 
      vTaskDelay(30);
      if (getFingerprintID() >= 60){
        fingther_ok = true;
      }
      digitalWrite(LED, LOW);
    }
    else{
      if (act_fingther){
        if ((fingther_ok)&&(conf_finger[0][finger.fingerID].act)){
          finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_BLUE, 10);
          Serial.println("Image ok");
          opendoor();
        }else{
          // finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 2);
          Serial.println("Image error");
        }
        act_fingther = false;
        fingther_ok = false;
      }
    }
    vTaskDelay(10);
  }
}

void IRAM_ATTR getID(void){
  xSemaphoreGiveFromISR(timerSemaphoreInt, NULL);
}

