#include "SI7021.h"
#include "ds18b.h"

extern SemaphoreHandle_t Mutex_si_measure;

#define ONE_WIRE_BUS            26

OneWire oneWire(ONE_WIRE_BUS);
DS18B20 sensords(&oneWire);

uint32_t start, stop;

void init_ds(void){
  Serial.print("DS18B20 Library version: ");
  Serial.println(DS18B20_LIB_VERSION);

  // wait until address found
  if (sensords.begin() == false)
  {
    Serial.println("ERROR: No device found");
    while (!sensords.begin()); // wait until device comes available.
  }

  sensords.setResolution(12);
  sensords.setConfig(DS18B20_CRC);  // or 1
  sensords.requestTemperatures();
}

void measuring_ds(void){
  start = millis();
  sensords.requestTemperatures();

  // wait for data AND detect disconnect
  uint32_t timeout = millis();
  while (!sensords.isConversionComplete())
  {
    if (millis() - timeout >= 800) // check for timeout
    {
      Serial.println("ERROR: timeout or disconnect");
      break;
    }
  }

  float t = sensords.getTempC();

  if (t == DEVICE_CRC_ERROR)
  {
    Serial.println("ERROR: CRC error");
  }
  stop = millis();
  if( xSemaphoreTake( Mutex_si_measure, portMAX_DELAY ) == pdTRUE ){
    meas_si.temp_ds = t;
    xSemaphoreGive( Mutex_si_measure );;
  }
}
