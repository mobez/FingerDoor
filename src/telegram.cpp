#include "config.h"
#include "telegram.h"

extern SemaphoreHandle_t Mutex_cnf_telegram;
extern telegram_cnf conf_telegram[2];

WiFiClientSecure client;
UniversalTelegramBot bot(conf_telegram[0].token, client);

unsigned long lastTimeBotRan;

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

