#include "WiFi.h"
#include "time.h"
#include "times.h"
#include "config.h"

extern time_cnf conf_time[2];
extern SemaphoreHandle_t Mutex_cnf_time;

const long  gmtOffset_sec = 36000; //3600;
const int   daylightOffset_sec = 0;
struct tm timeinfo;


void printLocalTime(void)
{
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void times_init(void){
  int epoch_time = 1649149529;
  //timeval epoch = {0, 0};
  timeval epoch = {epoch_time, 0};
  const timeval *tv = &epoch;
  timezone utc = {gmtOffset_sec,daylightOffset_sec};
  const timezone *tz = &utc;
  settimeofday(tv, tz);
  // if (connet) 
  if( xSemaphoreTake( Mutex_cnf_time, portMAX_DELAY ) == pdTRUE ){
  configTime(gmtOffset_sec, daylightOffset_sec, conf_time[0].ntpServer1, conf_time[0].ntpServer2, conf_time[0].ntpServer3);
   xSemaphoreGive( Mutex_cnf_time );
  }
  printLocalTime();
}
