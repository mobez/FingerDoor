#ifndef FINGER_H_
#define FINGER_H_

#include <Adafruit_Fingerprint.h>
#include <Arduino.h>

#define RXD2 16
#define TXD2 17

#define GPIOpin 33

#define mySerial Serial2


extern Adafruit_Fingerprint finger;

int add_phalanx(void);
uint8_t getFingerprintID(void);
int getFingerprintIDez(void);
int getFingerprintEnroll(uint8_t id);
int deleteFingerprint(uint8_t id);
int deleteAllFingerprint(void);
void test_led(void);

#endif /* FINGER_H_ */