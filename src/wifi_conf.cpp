#include "config.h"
#include "wifi_conf.h"
#include "esp_wifi.h"
#include "espnow.h"
#include "ETH.h"

extern wifi_lan_cnf conf_lan[2];
extern esp_now_cnf conf_esp_now[2];

char buf_scan[1024];
bool fl_dns = false;

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);

void scan_ap(void) {
  JSONVar myObject;
  String ssid;
  int32_t rssi;
  uint8_t encryptionType;
  uint8_t* bssid;
  int32_t channel;
  bool hidden;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(50);
  uint8_t scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);
  if (scanResult == 0) {
    Serial.println(F("No networks found"));
  } else if (scanResult > 0) {
    Serial.printf(PSTR("%d networks found:\r\n"), scanResult);

    // Print unsorted scan results
    for (int8_t i = 0; i < scanResult; i++) {
      WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel);
      myObject[i]["ssid"] = ssid;
      myObject[i]["rssi"] = rssi;
      myObject[i]["bssid"][0] = bssid[0];
      myObject[i]["bssid"][1] = bssid[1];
      myObject[i]["bssid"][2] = bssid[2];
      myObject[i]["bssid"][3] = bssid[3];
      myObject[i]["bssid"][4] = bssid[4];
      myObject[i]["bssid"][5] = bssid[5];
      myObject[i]["channel"] = channel;

      Serial.printf(PSTR("  %02d: [CH %02d] [%02X:%02X:%02X:%02X:%02X:%02X] %ddBm  %s\r\n"),
                  i,
                  channel,
                  bssid[0], bssid[1], bssid[2],
                  bssid[3], bssid[4], bssid[5],
                  rssi,
                  ssid.c_str());
      delay(0);
    }
    strcpy(buf_scan, JSON.stringify(myObject).c_str());
  } else {
    Serial.printf(PSTR("WiFi scan error %d\r\n"), scanResult);
  }
  wifi_init();
}

void wifi_init(uint8_t indx) {
  WiFi.disconnect();   
  if ((conf_lan[0].route) && (conf_lan[0].softap)) {
    WiFi.mode(WIFI_AP_STA);
    if (WiFi.SSID() != String((char  *)conf_lan[0].name)) {
      WiFi.begin((char  *)conf_lan[0].name, (char  *)conf_lan[0].pass);
    }
    uint8_t fr=0;
    while (fr < 20) {    
      delay(500);
      if (WiFi.status() == WL_CONNECTED){
          conf_esp_now[0].channel = WiFi.channel();
          reinit_esp_channel();
          break;
      }
      fr++;
    }
    WiFi.softAP((char  *)conf_lan[0].mdns_name, (char  *)conf_lan[0].pass_esp, conf_esp_now[indx].channel, 0);
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    // esp_wifi_get_mac((wifi_interface_t)ESP_IF_WIFI_STA, buf_mac);
    // // char_to_mac(WiFi.macAddress().c_str(), buf_mac);  
    // esp_wifi_set_mac(WIFI_IF_AP, buf_mac);
  }else if ((conf_lan[0].route) && (!conf_lan[0].softap)) {
    WiFi.mode(WIFI_STA);
    if (WiFi.SSID() != String((char  *)conf_lan[0].name)) {
      WiFi.begin((char  *)conf_lan[0].name, (char  *)conf_lan[0].pass);
    }
    uint8_t fr=0;
    while (fr < 20) {    
      delay(500);
      if (WiFi.status() == WL_CONNECTED){
        conf_esp_now[0].channel = WiFi.channel();
        //reinit_esp_channel();
        break;
      }
      fr++;
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }else if ((!conf_lan[0].route) && (conf_lan[0].softap)) {
    // WiFi.mode(WIFI_AP);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP((char  *)conf_lan[0].mdns_name, (char  *)conf_lan[0].pass_esp, conf_esp_now[indx].channel);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }else{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    //esp_wifi_set_channel(conf_esp_now[0].channel, WIFI_SECOND_CHAN_ABOVE);
    esp_wifi_set_channel(conf_esp_now[indx].channel, WIFI_SECOND_CHAN_NONE);
    // WiFi.begin((char  *)conf_lan[0].name, (char *)__null, conf_esp_now[0].channel,(const uint8_t *)__null, false);
    // WiFi.channel(conf_esp_now[0].channel);
    //wifi_set_channel(conf_esp_now[0].channel);
    Serial.println("WiFi off");
  }
  esp_wifi_set_max_tx_power(127);
  esp_wifi_set_ps(WIFI_PS_NONE);
  
  Serial.print("Open http://");
  Serial.print((char  *)conf_lan[0].mdns_name);
  Serial.println(" to see the file browser");
}


   //***************Parse bytes from string******************//
void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {    

  for (int i = 0; i < maxBytes; i++) {        

    bytes[i] = strtoul(str, NULL, base);  

    str = strchr(str, sep);          

    if (str == NULL || *str == '\0') {          

      break;                         

    }
    str++;          

  }
}




