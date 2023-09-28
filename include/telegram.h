#ifndef TELEGRAM_H_
#define TELEGRAM_H_

#include "Arduino.h"
#include "WiFi.h"
#include "WiFiClientSecure.h"
 #include "UniversalTelegramBot.h"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif


#endif /* TELEGRAM_H_ */
