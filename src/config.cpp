#include "config.h"
#include "WiFi.h"
#include "finger.h"

extern SemaphoreHandle_t Mutex_cnf_lan;
extern SemaphoreHandle_t Mutex_cnf_server;
extern SemaphoreHandle_t Mutex_cnf_time;
extern SemaphoreHandle_t Mutex_cnf_finger;
extern SemaphoreHandle_t Mutex_cnf_now;
extern SemaphoreHandle_t Mutex_value;
extern SemaphoreHandle_t Mutex_file;

wifi_lan_cnf conf_lan[2];
esp_now_cnf conf_esp_now[2];
time_cnf conf_time[2];
servers_cnf conf_server[2];
fingers_cnf conf_finger[2][MAXFINGER];

float ver_s = 0.0;
const float ver = 1.0;

bool exists(String path){
  bool yes = false;
  if( xSemaphoreTake( Mutex_file, portMAX_DELAY ) == pdTRUE ){
    File file = FILESYSTEM.open(path, "r");
    if(!file.isDirectory()){
      yes = true;
    }
    file.close();
    xSemaphoreGive( Mutex_file );
  }
  return yes;
}

void defconfig(void) {
  String name = "NEW_"+WiFi.macAddress(); 
  Serial.print("STA mac: ");
  Serial.println(WiFi.macAddress());
  if( xSemaphoreTake( Mutex_cnf_lan, portMAX_DELAY ) == pdTRUE ){
    conf_lan[0].route = true;
    conf_lan[0].softap = true;    
    strcpy((char  *)conf_lan[0].mdns_name, name.c_str());
    //strcpy(conf_lan[0].name, name.c_str());
    strcpy((char  *)conf_lan[0].name, "Tech");
    strcpy((char  *)conf_lan[0].pass, "52905290123");
    strcpy((char  *)conf_lan[0].pass_esp, "12345678");
    strcpy((char  *)conf_lan[0].login, "admin");
    strcpy((char  *)conf_lan[0].pass_autch, "admin");
    xSemaphoreGive( Mutex_cnf_lan );
  }
  if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
    conf_esp_now[0].max_peer = MAXPEERS;
    conf_esp_now[0].channel = DEFCHANNEL;
    conf_esp_now[0].role = ROLEESP;
    memset( (void *)conf_esp_now[0].peers, 0, sizeof(esp_now_peers)*MAXPEERS );
    xSemaphoreGive( Mutex_cnf_now );
  }
  if( xSemaphoreTake( Mutex_cnf_time, portMAX_DELAY ) == pdTRUE ){
    strcpy(conf_time[0].ntpServer1, "ntp3.ntp-servers.net");
    strcpy(conf_time[0].ntpServer2, "ntp4.ntp-servers.net");
    strcpy(conf_time[0].ntpServer3, "ntp5.ntp-servers.net");
    // conf_time[0].ntpServer3[0] = 0;
    //conf_time[0].pereod_alarm = 60;
    conf_time[0].pereod_alarm = 10;
    xSemaphoreGive( Mutex_cnf_time );
  }
  if( xSemaphoreTake( Mutex_cnf_server, portMAX_DELAY ) == pdTRUE ){
    conf_server[0].max_serv = MAXSERVERS;
    strcpy((char  *)conf_server[0].serv[0].host, "10.11.25.78");
    //strcpy((char  *)conf_server[0].serv[0].host, "10.11.25.76");
    conf_server[0].serv[0].port = 49002;
    conf_server[0].serv[0].act = false;
    strcpy((char  *)conf_server[0].serv[1].host, "10.11.25.76");
    conf_server[0].serv[1].port = 49002;
    conf_server[0].serv[1].act = false;
    strcpy((char  *)conf_server[0].serv[2].host, "10.11.161.102");
    conf_server[0].serv[2].port = 21;
    conf_server[0].serv[2].act = false;
    xSemaphoreGive( Mutex_cnf_server );
  }
  if( xSemaphoreTake( Mutex_cnf_finger, portMAX_DELAY ) == pdTRUE ){
    for (uint8_t i = 0; i < MAXFINGER; i++)
    {
      conf_finger[0][i].act =  false;
      conf_finger[0][i].id = 0;
      conf_finger[0][i].name[0] = 0;
      conf_finger[0][i].phalanx = thumb;
    }
    xSemaphoreGive( Mutex_cnf_finger );
  }
}

bool saveconfig(conf_type type) {
  StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
  bool ss = true;
  String name_file;
  switch (type)
  {
  case ver_cnf:
    name_file = VERFILE;
    myObject["ver"] = ver;
    break;
  case lan_cnf:
    name_file = LANFILE;
    if( xSemaphoreTake( Mutex_cnf_lan, portMAX_DELAY ) == pdTRUE ){
      myObject["route"] = conf_lan[0].route;
      myObject["softap"] = conf_lan[0].softap;    
      myObject["mdns_name"] = conf_lan[0].mdns_name;
      myObject["name"] = conf_lan[0].name;
      myObject["pass"] = conf_lan[0].pass;
      myObject["pass_esp"] = conf_lan[0].pass_esp;
      myObject["login"] = conf_lan[0].login;
      myObject["pass_autch"] = conf_lan[0].pass_autch;
      xSemaphoreGive( Mutex_cnf_lan );
    }
    break;
  case now_cnf:
    name_file = NOWFILE;
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
      myObject["max_peer"] = MAXPEERS;
      myObject["channel"] = conf_esp_now[0].channel;
      myObject["role"] = conf_esp_now[0].role;
      for (uint8_t i = 0; i < MAXPEERS; i++)
      {
        myObject["peers"][i]["act"] = conf_esp_now[0].peers[i].act;
        myObject["peers"][i]["channel"] =conf_esp_now[0].peers[i].channel;
        myObject["peers"][i]["name"] = conf_esp_now[0].peers[i].name;
        myObject["peers"][i]["lastname"] = conf_esp_now[0].peers[i].lastname;
        myObject["peers"][i]["mac_addr"][0] = conf_esp_now[0].peers[i].mac_addr[0];
        myObject["peers"][i]["mac_addr"][1] = conf_esp_now[0].peers[i].mac_addr[1];
        myObject["peers"][i]["mac_addr"][2] = conf_esp_now[0].peers[i].mac_addr[2];
        myObject["peers"][i]["mac_addr"][3] = conf_esp_now[0].peers[i].mac_addr[3];
        myObject["peers"][i]["mac_addr"][4] = conf_esp_now[0].peers[i].mac_addr[4];
        myObject["peers"][i]["mac_addr"][5] = conf_esp_now[0].peers[i].mac_addr[5];
        myObject["peers"][i]["mac_ap"][0] = conf_esp_now[0].peers[i].mac_ap[0];
        myObject["peers"][i]["mac_ap"][1] = conf_esp_now[0].peers[i].mac_ap[1];
        myObject["peers"][i]["mac_ap"][2] = conf_esp_now[0].peers[i].mac_ap[2];
        myObject["peers"][i]["mac_ap"][3] = conf_esp_now[0].peers[i].mac_ap[3];
        myObject["peers"][i]["mac_ap"][4] = conf_esp_now[0].peers[i].mac_ap[4];
        myObject["peers"][i]["mac_ap"][5] = conf_esp_now[0].peers[i].mac_ap[5];
      }
      xSemaphoreGive( Mutex_cnf_now );
    }
    break;
  case times_cnf:
    name_file = TIMEFILE;
    if( xSemaphoreTake( Mutex_cnf_time, portMAX_DELAY ) == pdTRUE ){
      myObject["ntpServer1"] = conf_time[0].ntpServer1;
      myObject["ntpServer2"] = conf_time[0].ntpServer2;
      myObject["ntpServer3"] = conf_time[0].ntpServer3;
      myObject["pereod_alarm"] = conf_time[0].pereod_alarm;
      xSemaphoreGive( Mutex_cnf_time );
    }
    break;
  case server_cnf:
    name_file = SERVERFILE;
    if( xSemaphoreTake( Mutex_cnf_server, portMAX_DELAY ) == pdTRUE ){
      myObject["max_serv"]=conf_server[0].max_serv;
      for (uint8_t i = 0; i < MAXSERVERS; i++)
      {
        myObject["serv"][i]["act"] = conf_server[0].serv[i].act;
        myObject["serv"][i]["host"] = conf_server[0].serv[i].host;
        myObject["serv"][i]["port"] = conf_server[0].serv[i].port;
      }
      xSemaphoreGive( Mutex_cnf_server );
    }
    break;
  case finger_cnf:
    name_file = FINGERFILE;
    if( xSemaphoreTake( Mutex_cnf_finger, portMAX_DELAY ) == pdTRUE ){
      for (uint8_t i = 0; i < MAXFINGER; i++)
      {
        myObject[i]["act"] = conf_finger[0][i].act;
        myObject[i]["id"] = conf_finger[0][i].id;
        myObject[i]["name"] = conf_finger[0][i].name;
        myObject[i]["phalanx"] = conf_finger[0][i].phalanx;
      }
      xSemaphoreGive( Mutex_cnf_finger );
    }
    break;
  default:
    ss=false;
    break;
  }
  if (ss) {
    if( xSemaphoreTake( Mutex_file, portMAX_DELAY ) == pdTRUE ){
      File configFile = FILESYSTEM.open(name_file, "w");
      if (!configFile) {
        Serial.printf("Failed to open %s for writing\r\n", name_file.c_str());
        configFile.close();
        xSemaphoreGive( Mutex_file );
        return false;
      }else{
        Serial.printf("Save file: %s\r\n", name_file.c_str());
      }
      if (serializeJson(myObject, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
      }
      //configFile.print(myObject);
      configFile.close();
      xSemaphoreGive( Mutex_file );
    }    
  }
  return true;
}

bool loadConfig(conf_type type) {
  String name_file;
  switch (type)
  {
  case lan_cnf:
    name_file = LANFILE;
    break;
  case now_cnf:
    name_file = NOWFILE;
    break;
  case times_cnf:
    name_file = TIMEFILE;
    break;
  case server_cnf:
    name_file = SERVERFILE;
    break;
  case finger_cnf:
    name_file = FINGERFILE;
    break;
  case ver_cnf:
    name_file = VERFILE;
    break;
  default:
    return false;
    break;
  }
  //StaticJsonDocument<1024> myObject;
  DynamicJsonDocument myObject(1024);
  //JSONVar myObject;
  if( xSemaphoreTake( Mutex_file, portMAX_DELAY ) == pdTRUE ){
    File configFile = FILESYSTEM.open(name_file, "r");
    if (!configFile) {
      Serial.printf("Failed to open %s\r\n", name_file.c_str());
      configFile.close();
      xSemaphoreGive( Mutex_file );
      saveconfig(type);
      return false;
    }

    size_t size = configFile.size();
    Serial.printf("Open config: %s, size %d\r\n", name_file.c_str(), size);
    if ((size > 4096) && (size <= 0)) {
      Serial.println("Config file size is too large");
      xSemaphoreGive( Mutex_file );
      saveconfig(type);
      return false;
    }
    DeserializationError error = deserializeJson(myObject, configFile);
    if (error){
      Serial.println(F("Failed to read file, using default configuration"));
      Serial.println("Parsing input failed!");
      xSemaphoreGive( Mutex_file );
      saveconfig(type);
      return false;
    }
    configFile.close();
    xSemaphoreGive( Mutex_file );
  }
  switch (type)
  {
  case ver_cnf:
    ver_s = (double)myObject["ver"];
    break;
  case lan_cnf:
    if( xSemaphoreTake( Mutex_cnf_lan, portMAX_DELAY ) == pdTRUE ){
      conf_lan[0].route = myObject["route"];
      conf_lan[0].softap = myObject["softap"];    
      strcpy((char  *)conf_lan[0].mdns_name, myObject["mdns_name"]);
      strcpy((char  *)conf_lan[0].name, myObject["name"]);
      strcpy((char  *)conf_lan[0].pass, myObject["pass"]);
      strcpy((char  *)conf_lan[0].pass_esp, myObject["pass_esp"]);
      strcpy((char  *)conf_lan[0].login, myObject["login"]);
      strcpy((char  *)conf_lan[0].pass_autch, myObject["pass_autch"]);

      conf_lan[1].route = conf_lan[0].route;
      conf_lan[1].softap = conf_lan[0].softap;    
      strcpy((char  *)conf_lan[1].mdns_name, (char  *)conf_lan[0].mdns_name);
      strcpy((char  *)conf_lan[1].name, (char  *)conf_lan[0].name);
      strcpy((char  *)conf_lan[1].pass, (char  *)conf_lan[0].pass);
      strcpy((char  *)conf_lan[1].pass_esp, (char  *)conf_lan[0].pass_esp);
      strcpy((char  *)conf_lan[1].login, (char  *)conf_lan[0].login);
      strcpy((char  *)conf_lan[1].pass_autch, (char  *)conf_lan[0].pass_autch);
      xSemaphoreGive( Mutex_cnf_lan );
    }
    break;
  case now_cnf:
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
      conf_esp_now[0].channel = (int)myObject["channel"];
      conf_esp_now[0].role = (role_esp)(int)myObject["role"];
      for (uint8_t i = 0; i < MAXPEERS; i++)
      {
        conf_esp_now[0].peers[i].act = myObject["peers"][i]["act"];
        conf_esp_now[0].peers[i].channel = (int)myObject["peers"][i]["channel"];
        strcpy(conf_esp_now[0].peers[i].name, myObject["peers"][i]["name"]);
        strcpy(conf_esp_now[0].peers[i].lastname, myObject["peers"][i]["lastname"]);
        conf_esp_now[0].peers[i].mac_addr[0] = (int)myObject["peers"][i]["mac_addr"][0];
        conf_esp_now[0].peers[i].mac_addr[1] = (int)myObject["peers"][i]["mac_addr"][1];
        conf_esp_now[0].peers[i].mac_addr[2] = (int)myObject["peers"][i]["mac_addr"][2];
        conf_esp_now[0].peers[i].mac_addr[3] = (int)myObject["peers"][i]["mac_addr"][3];
        conf_esp_now[0].peers[i].mac_addr[4] = (int)myObject["peers"][i]["mac_addr"][4];
        conf_esp_now[0].peers[i].mac_addr[5] = (int)myObject["peers"][i]["mac_addr"][5];
        conf_esp_now[0].peers[i].mac_ap[0] = (int)myObject["peers"][i]["mac_ap"][0];
        conf_esp_now[0].peers[i].mac_ap[1] = (int)myObject["peers"][i]["mac_ap"][1];
        conf_esp_now[0].peers[i].mac_ap[2] = (int)myObject["peers"][i]["mac_ap"][2];
        conf_esp_now[0].peers[i].mac_ap[3] = (int)myObject["peers"][i]["mac_ap"][3];
        conf_esp_now[0].peers[i].mac_ap[4] = (int)myObject["peers"][i]["mac_ap"][4];
        conf_esp_now[0].peers[i].mac_ap[5] = (int)myObject["peers"][i]["mac_ap"][5];
        conf_esp_now[1].peers[i].act =conf_esp_now[0].peers[i].act;
        conf_esp_now[1].peers[i].channel =conf_esp_now[0].peers[i].channel;
        strcpy(conf_esp_now[1].peers[i].name, conf_esp_now[0].peers[i].name);
        strcpy(conf_esp_now[1].peers[i].lastname, conf_esp_now[0].peers[i].lastname);
        conf_esp_now[1].peers[i].mac_addr[0] = conf_esp_now[0].peers[i].mac_addr[0];
        conf_esp_now[1].peers[i].mac_addr[1] = conf_esp_now[0].peers[i].mac_addr[1];
        conf_esp_now[1].peers[i].mac_addr[2] = conf_esp_now[0].peers[i].mac_addr[2];
        conf_esp_now[1].peers[i].mac_addr[3] = conf_esp_now[0].peers[i].mac_addr[3];
        conf_esp_now[1].peers[i].mac_addr[4] = conf_esp_now[0].peers[i].mac_addr[4];
        conf_esp_now[1].peers[i].mac_addr[5] = conf_esp_now[0].peers[i].mac_addr[5];
        conf_esp_now[1].peers[i].mac_ap[0] = conf_esp_now[0].peers[i].mac_ap[0];
        conf_esp_now[1].peers[i].mac_ap[1] = conf_esp_now[0].peers[i].mac_ap[1];
        conf_esp_now[1].peers[i].mac_ap[2] = conf_esp_now[0].peers[i].mac_ap[2];
        conf_esp_now[1].peers[i].mac_ap[3] = conf_esp_now[0].peers[i].mac_ap[3];
        conf_esp_now[1].peers[i].mac_ap[4] = conf_esp_now[0].peers[i].mac_ap[4];
        conf_esp_now[1].peers[i].mac_ap[5] = conf_esp_now[0].peers[i].mac_ap[5];
      }
      conf_esp_now[1].channel = conf_esp_now[0].channel;
      conf_esp_now[1].role = conf_esp_now[0].role;
      xSemaphoreGive( Mutex_cnf_now );
    }
    break;
  case times_cnf:
    if( xSemaphoreTake( Mutex_cnf_time, portMAX_DELAY ) == pdTRUE ){
      strcpy(conf_time[0].ntpServer1, myObject["ntpServer1"]);
      strcpy(conf_time[0].ntpServer2, myObject["ntpServer2"]);
      strcpy(conf_time[0].ntpServer3, myObject["ntpServer3"]);
      conf_time[0].pereod_alarm = (int)myObject["pereod_alarm"];
      strcpy(conf_time[1].ntpServer1, conf_time[0].ntpServer1);
      strcpy(conf_time[1].ntpServer2, conf_time[0].ntpServer2);
      strcpy(conf_time[1].ntpServer3, conf_time[0].ntpServer3);
      conf_time[1].pereod_alarm = conf_time[0].pereod_alarm;
      xSemaphoreGive( Mutex_cnf_time );
    }
    break;
  case server_cnf:
    if( xSemaphoreTake( Mutex_cnf_server, portMAX_DELAY ) == pdTRUE ){
      conf_server[0].max_serv = (int)myObject["max_serv"];
      conf_server[1].max_serv = conf_server[0].max_serv;
      for (uint8_t i = 0; i < conf_server[0].max_serv; i++)
      {
        conf_server[0].serv[i].act = myObject["serv"][i]["act"];
        strcpy((char  *)conf_server[0].serv[i].host, myObject["serv"][i]["host"]);
        conf_server[0].serv[i].port = (int)myObject["serv"][i]["port"];
        conf_server[1].serv[i].act = conf_server[0].serv[i].act;
        strcpy((char  *)conf_server[1].serv[i].host, (char  *)conf_server[0].serv[i].host);
        conf_server[1].serv[i].port = conf_server[0].serv[i].port;
        Serial.printf("Server act: %d, host: %s, port: %d\r\n",conf_server[0].serv[i].act, conf_server[0].serv[i].host, conf_server[0].serv[i].port);
      }
      xSemaphoreGive( Mutex_cnf_server );
    }
    break;
  case finger_cnf:
    if( xSemaphoreTake( Mutex_cnf_finger, portMAX_DELAY ) == pdTRUE ){
      for (uint8_t i = 0; i < MAXFINGER; i++)
      {
        conf_finger[0][i].act =  myObject[i]["act"];
        conf_finger[0][i].id = (int)myObject[i]["id"];
        strcpy((char  *)conf_finger[0][i].name, myObject[i]["name"]);
        conf_finger[0][i].phalanx = (fingers)(int)myObject[i]["phalanx"];
      }
      xSemaphoreGive( Mutex_cnf_finger );
    }
    break;
  default:
    break;
  }
  

  return true;
}

void initconfig(void) {  
  //if (!FILESYSTEM.begin()) FILESYSTEM.begin(true);
  defconfig();
  if (FILESYSTEM.begin(true)){
    Serial.println("Init file system!");

    Serial.print("totalBytes: ");
    Serial.println(FILESYSTEM.totalBytes());
    Serial.print("usedBytes: ");
    Serial.println(FILESYSTEM.usedBytes());
    loadConfig(ver_cnf);
    //saveconfig(lan_cnf);
    loadConfig(lan_cnf);
    if (ver != ver_s){
      deleteAllFingerprint();
      //saveconfig(ver_cnf);
      saveconfig(now_cnf);
      saveconfig(times_cnf);
      saveconfig(server_cnf);
      saveconfig(finger_cnf);
    }else{    
      loadConfig(now_cnf);
      loadConfig(times_cnf);
      loadConfig(server_cnf);
      loadConfig(finger_cnf);
    }
  }else{
    Serial.println("Failed init file system!");
  }
}
