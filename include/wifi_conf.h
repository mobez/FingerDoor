#ifndef WIFICONF_H_
#define WIFICONF_H_
#include <Arduino.h>
#include "WiFi.h"
//#include "ESP8266mDNS.h"
#include "NetBIOS.h"

void wifi_init(uint8_t indx=0);
void scan_ap(void);

extern char buf_scan[1024];

#endif /* WIFICONF_H_ */