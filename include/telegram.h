#ifndef TELEGRAM_H_
#define TELEGRAM_H_

#include "Arduino.h"
#include "WiFi.h"
#include "WiFiClientSecure.h"
#include "UniversalTelegramBot.h"
#include "ArduinoJson.h" 

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

extern WiFiClientSecure client;
extern UniversalTelegramBot bot;
extern unsigned long lastTimeBotRan;
void handleNewMessages(int numNewMessages);
void telegraminit(void);

#endif /* TELEGRAM_H_ */
