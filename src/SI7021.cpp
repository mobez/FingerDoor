
#include "SI7021.h"

extern SemaphoreHandle_t Mutex_si_measure;

Adafruit_Si7021 sensor = Adafruit_Si7021();
meas_si7021 meas_si;

void init_temp_si(void){
  if (!sensor.begin()) {
    Serial.println("Did not find Si7021 sensor!");
  }
  
  Serial.print(" Rev(");
  Serial.print(sensor.getRevision());
  Serial.print(")");
  Serial.print(" Serial #"); Serial.print(sensor.sernum_a, HEX); Serial.println(sensor.sernum_b, HEX);
}

void measuring(void){
  if( xSemaphoreTake( Mutex_si_measure, 5000/portTICK_RATE_MS ) == pdTRUE ){
    meas_si.temp = sensor.readTemperature();
    meas_si.humidi = sensor.readHumidity();
    xSemaphoreGive( Mutex_si_measure );;
  }
}
