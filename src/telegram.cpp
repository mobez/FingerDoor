#include "config.h"
#include "telegram.h"
#include "SI7021.h"

extern SemaphoreHandle_t Mutex_cnf_telegram;
extern SemaphoreHandle_t Mutex_si_measure;
extern SemaphoreHandle_t openSemaphore;
extern telegram_cnf conf_telegram[2];

WiFiClientSecure client;
UniversalTelegramBot bot(conf_telegram[0].token, client);

unsigned long lastTimeBotRan;
char text_send[512];

// Задаем действия при получении новых сообщений 
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
 
  for (int i=0; i<numNewMessages; i++) {
    // Идентификатор чата запроса
    String chat_id = String(bot.messages[i].chat_id);
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1); 
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    if( xSemaphoreTake( Mutex_cnf_telegram, portMAX_DELAY ) == pdTRUE ){
      if (chat_id != conf_telegram[0].idgrup){
        bot.sendMessage(chat_id, "Unauthorized user", "");
        continue;
      }
      xSemaphoreGive( Mutex_cnf_telegram );
    }
    // Выводим полученное сообщение
    String text = bot.messages[i].text;
    Serial.println(text);
 
    String from_name = bot.messages[i].from_name;
 
    if (text == "/help") {
      bot.sendMessage(chat_id, "Help:\r\n", "");
    }else if(text == "/weather"){
      // time_t now;
      // char strftime_buf[64];
      // struct tm timeinfo;

      // time(&now);
      // // Set timezone to China Standard Time
      // setenv("TZ", "CST-8", 1);
      // tzset();

      // localtime_r(&now, &timeinfo);
      // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
      // ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
      if( xSemaphoreTake( Mutex_si_measure, portMAX_DELAY ) == pdTRUE ){
        char t_0[6];
        char t_1[6];
        dtostrf(meas_si.temp, 6, 2, t_0); 
        dtostrf(meas_si.temp_ds, 6, 2, t_1);
        sprintf(text_send, "Температура ds: %s\xB0\\C\r\n\\Температура: %s\r\n\\Влажность: %d\r\n", t_1, t_0, meas_si.humidi);        
        xSemaphoreGive( Mutex_si_measure );;
      }
      bot.sendMessage(chat_id, text_send, "");
    }

    // if (text == "/led_on") {
    //   bot.sendMessage(chat_id, "LED state set to ON", "");
    //   ledState = HIGH;
    //   digitalWrite(ledPin, ledState);
    // }
    
    // if (text == "/led_off") {
    //   bot.sendMessage(chat_id, "LED state set to OFF", "");
    //   ledState = LOW;
    //   digitalWrite(ledPin, ledState);
    // }
    
    // if (text == "/state") {
    //   if (digitalRead(ledPin)){
    //     bot.sendMessage(chat_id, "LED is ON", "");
    //   }
    //   else{
    //     bot.sendMessage(chat_id, "LED is OFF", "");
    //   }
    // }
  }
}

void telegraminit(void){
  #ifdef ESP8266
  configTime(0, 0, "pool.ntp.org");      // получаем всемирное координированное время (UTC) через NTP
  client.setTrustAnchors(&cert); // Получаем сертификат api.telegram.org
  #endif
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Добавляем корневой сертификат для api.telegram.org
  #endif
  bot.sendMessage(conf_telegram[0].idgrup, "Bot Started", "");
}

