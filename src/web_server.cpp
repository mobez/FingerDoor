#include "web_server.h"
#include "espnow.h"
#include "wifi_conf.h"
#include "config.h"
#include "Update.h"
#include "finger.h"
#include "SI7021.h"
#include "times.h"

extern wifi_lan_cnf conf_lan[2];
extern esp_now_cnf conf_esp_now[2];
extern time_cnf conf_time[2];
extern servers_cnf conf_server[2];
extern fingers_cnf conf_finger[2][MAXFINGER];

extern SemaphoreHandle_t Mutex_cnf_lan;
extern SemaphoreHandle_t Mutex_cnf_server;
extern SemaphoreHandle_t Mutex_cnf_time;
extern SemaphoreHandle_t Mutex_cnf_finger;
extern SemaphoreHandle_t Mutex_cnf_now;
extern SemaphoreHandle_t Mutex_value;
extern SemaphoreHandle_t Mutex_file;
extern SemaphoreHandle_t Mutex_si_measure;

extern SemaphoreHandle_t openSemaphore;
extern SemaphoreHandle_t addSemaphore;

extern web_mac_no_peer web_peer_no;
extern esp_now_cnf_dev cnf_con_peers[MAXPEERS];
extern int freeIdFinger;

volatile status_figner figner_web;

WebServer server(80);
File fsUploadFile;
bool is_authentified();



void char_to_mac(const char *str, uint8_t *mac) {
  int buf_mac[6];
    //Serial.println(String(str));
    sscanf(str, "%X:%X:%X:%X:%X:%X",
           &buf_mac[0],
           &buf_mac[1],
           &buf_mac[2],
           &buf_mac[3],
           &buf_mac[4],
           &buf_mac[5]);
    mac[0] =buf_mac[0];
    mac[1] =buf_mac[1];
    mac[2] =buf_mac[2];
    mac[3] =buf_mac[3];
    mac[4] =buf_mac[4];
    mac[5] =buf_mac[5];
    Serial.printf(PSTR("MAC: [%02X:%02X:%02X:%02X:%02X:%02X]\r\n"),
                        mac[0], mac[1], mac[2],
                        mac[3], mac[4], mac[5]);
}

String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool is_authentified(void){
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")){   
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    String cookie_act = "ESPSESSIONID"+WiFi.macAddress()+"=1";
    if (cookie.indexOf(cookie_act) != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;  
}
void send_redir(String location){
  String header = "HTTP/1.1 301 OK\r\nLocation: /"+location+"\r\nCache-Control: no-cache\r\n\r\n";
  server.sendContent(header);
}
bool check_autch(void) {
  bool autch = is_authentified();
  if (!autch){
    send_redir("login");
  }
  return autch;
}

bool handleFileRead(String path) {
  Serial.println(String("handleFileRead: ") + path);
  if (path.endsWith("/")) {
    path += "index.htm";
    server.sendHeader("Cache-Control", "max-age=31536000");
  }
  if ((path == "/ace.js")||(path == "/mode-html.js")||(path == "/worker-html.js")||(path == "/edit.htm")||(path == "/config.htm")||(path == "/favicon.ico"))server.sendHeader("Cache-Control", "max-age=31536000");
  else if((path.endsWith(".js"))||(path.endsWith(".css"))||(path.endsWith(".jpg")))server.sendHeader("Cache-Control", "max-age=5400");
  else server.sendHeader("Cache-Control", "max-age=5");
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists(pathWithGz) || exists(path)) {
    if (exists(pathWithGz)) {
      path += ".gz";
    }
    if( xSemaphoreTake( Mutex_file, 10000/portTICK_RATE_MS ) == pdTRUE ){
      File file = FILESYSTEM.open(path, "r");
      server.streamFile(file, contentType);
      file.close();
      xSemaphoreGive( Mutex_file );
    }
    return true;
  }
  return false;
}

void handleFileUpload(void){
  if (!check_autch()) return;
  if(server.uri() != "/edit") return;
  if( xSemaphoreTake( Mutex_file, portMAX_DELAY ) == pdTRUE ){
    HTTPUpload& upload = server.upload();
    if(upload.status == UPLOAD_FILE_START){
      String filename = upload.filename;
      if(!filename.startsWith("/")) filename = "/"+filename;
      Serial.print("handleFileUpload Name: "); Serial.println(filename);
      fsUploadFile = FILESYSTEM.open(filename, "w");
      filename = String();
    } else if(upload.status == UPLOAD_FILE_WRITE){
      //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
      if(fsUploadFile)
        fsUploadFile.write(upload.buf, upload.currentSize);
    } else if(upload.status == UPLOAD_FILE_END){
      if(fsUploadFile)
        fsUploadFile.close();
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
    } else {
      Serial.println("Error!");
    }
    xSemaphoreGive( Mutex_file );
  }
}

void handleFileDelete(void){
  if (!check_autch()) return;
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
    String path = server.arg(0);
    Serial.println("handleFileDelete: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!FILESYSTEM.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  else
    if( xSemaphoreTake( Mutex_file, portMAX_DELAY ) == pdTRUE ){
      FILESYSTEM.remove(path);
      server.send(200, "text/plain", "");
      path = String();
      xSemaphoreGive( Mutex_file );
    };
}

void handleFileCreate(void){
  if (!check_autch()) return;
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(FILESYSTEM.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  if( xSemaphoreTake( Mutex_file, portMAX_DELAY ) == pdTRUE ){
    File file = FILESYSTEM.open(path, "w");
    if(file){
      file.close();
    }else{
      xSemaphoreGive( Mutex_file );
      return server.send(500, "text/plain", "CREATE FAILED");
    }
    xSemaphoreGive( Mutex_file );
  }
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList(void) {
  if (!check_autch()) return;
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}
  if( xSemaphoreTake( Mutex_file, portMAX_DELAY ) == pdTRUE ){
    StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
    int f=0;    
    String path = server.arg("dir");
    Serial.println("handleFileList: " + path);
    File root = FILESYSTEM.open(path);
    path = String();
    if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          myObject[f]["type"] = (file.isDirectory())?"dir":"file";
          myObject[f]["name"] = String(file.name());
          file = root.openNextFile();
          f++;
      }
    }
    xSemaphoreGive( Mutex_file );
    String output;
    serializeJson(myObject, output);
    //server.send(200, "text/json", JSON.stringify(myObject));  
    server.send(200, "text/json", output);  
  }
}



void handleConfig(void){
  Serial.println("Enter handleConfig");
  String header;
  if (check_autch()){
    handleFileRead("/config.htm");
  } 
}

//login page, also called for disconnect
void handleLogin(void){
  String msg;
  if (server.hasHeader("Cookie")){   
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")){
    Serial.println("Disconnection");
    String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID"+WiFi.macAddress()+"=0\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")){
    if( xSemaphoreTake( Mutex_cnf_lan, portMAX_DELAY ) == pdTRUE ){      
      if (server.arg("USERNAME") == (char  *)conf_lan[0].login &&  server.arg("PASSWORD") == (char  *)conf_lan[0].pass_autch ){
        xSemaphoreGive( Mutex_cnf_lan );
        String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID"+WiFi.macAddress()+"=1; Max-Age=18000\r\nLocation: /config\r\nCache-Control: no-cache\r\n\r\n";
        server.sendContent(header);
        Serial.println("Log in Successful");
        return;
      }else{
        xSemaphoreGive( Mutex_cnf_lan );
      }
    }
  }
 handleFileRead("/auth.htm");
}

void init_web(void){
  server.on("/def", HTTP_GET,[](){
    if (!check_autch()) return;
    defconfig();
    send_redir("login");
  });
  server.on("/login", handleLogin);  
 //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, [](){
    if (!check_autch()) return;
    if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.on("/config", handleConfig);
  server.on("/config.htm", handleConfig);
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works without need of authentification");
  });
  server.onNotFound([](){
    bool autch = false;
    if(server.uri() == LANFILE){
        autch=true;
    }
    else if (server.uri() == NOWFILE) {
        autch=true;
    }
    else if (server.uri() == TIMEFILE) {
        autch=true;
    }
    else if (server.uri() == SERVERFILE) {
        autch=true;
    }
    else if (server.uri() == FINGERFILE) {
        autch=true;
    }
    else if (server.uri() == "/config") {
        autch=true;
    }
    if ((autch) &&(!check_autch())) return;
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  server.on("/update", HTTP_POST, []() {
    if (!check_autch()) return;
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/now", HTTP_GET, [](){
    uint8_t buf_mac[6];
    char_to_mac(WiFi.macAddress().c_str(), buf_mac);
    StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){    
      myObject["max_peer"] = MAXPEERS;
      myObject["channel"] = conf_esp_now[0].channel;
      myObject["role"] = conf_esp_now[0].role;
      myObject["my_mac"][0] = buf_mac[0];
      myObject["my_mac"][1] = buf_mac[1];
      myObject["my_mac"][2] = buf_mac[2];
      myObject["my_mac"][3] = buf_mac[3];
      myObject["my_mac"][4] = buf_mac[4];
      myObject["my_mac"][5] = buf_mac[5];
      // for (uint8_t i = 0; i < MAXPEERS; i++) 
      // {
      //   myObject["act_peers"][i]["act"] = act_peers[i].act;
      //   myObject["act_peers"][i]["mac_addr"][0] = act_peers[i].mac_addr[0];
      //   myObject["act_peers"][i]["mac_addr"][1] = act_peers[i].mac_addr[1];
      //   myObject["act_peers"][i]["mac_addr"][2] = act_peers[i].mac_addr[2];
      //   myObject["act_peers"][i]["mac_addr"][3] = act_peers[i].mac_addr[3];
      //   myObject["act_peers"][i]["mac_addr"][4] = act_peers[i].mac_addr[4];
      //   myObject["act_peers"][i]["mac_addr"][5] = act_peers[i].mac_addr[5];
      //   myObject["peers"][i]["act"] = conf_esp_now[0].peers[i].act;
      //   myObject["peers"][i]["mac_addr"][0] = conf_esp_now[0].peers[i].mac_addr[0];
      //   myObject["peers"][i]["mac_addr"][1] = conf_esp_now[0].peers[i].mac_addr[1];
      //   myObject["peers"][i]["mac_addr"][2] = conf_esp_now[0].peers[i].mac_addr[2];
      //   myObject["peers"][i]["mac_addr"][3] = conf_esp_now[0].peers[i].mac_addr[3];
      //   myObject["peers"][i]["mac_addr"][4] = conf_esp_now[0].peers[i].mac_addr[4];
      //   myObject["peers"][i]["mac_addr"][5] = conf_esp_now[0].peers[i].mac_addr[5];
      //   myObject["peers"][i]["meas"]["null_id"] = conf_esp_now[0].meas[i].null_id;
      //   myObject["peers"][i]["meas"]["cnt"] = conf_esp_now[0].meas[i].cnt;
      // }
      xSemaphoreGive( Mutex_cnf_now );
    }
    String output;
    serializeJson(myObject, output);
    server.send(200, "text/json", output);
    //server.send(200, "text/json", JSON.stringify(myObject));
  });  
  server.on("/peers", HTTP_GET, [](){
    StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
      for (uint8_t i = 0; i < MAXPEERS; i++)
      {
        myObject[i]["act"] = conf_esp_now[0].peers[i].act;
        myObject[i]["cnf"] = conf_esp_now[0].peers[i].cnf;
        myObject[i]["name"] = conf_esp_now[0].peers[i].name;
        myObject[i]["lastname"] = conf_esp_now[0].peers[i].lastname;
        myObject[i]["mac_addr"][0] = conf_esp_now[0].peers[i].mac_addr[0];
        myObject[i]["mac_addr"][1] = conf_esp_now[0].peers[i].mac_addr[1];
        myObject[i]["mac_addr"][2] = conf_esp_now[0].peers[i].mac_addr[2];
        myObject[i]["mac_addr"][3] = conf_esp_now[0].peers[i].mac_addr[3];
        myObject[i]["mac_addr"][4] = conf_esp_now[0].peers[i].mac_addr[4];
        myObject[i]["mac_addr"][5] = conf_esp_now[0].peers[i].mac_addr[5];
        myObject[i]["mac_ap"][0] = conf_esp_now[0].peers[i].mac_ap[0];
        myObject[i]["mac_ap"][1] = conf_esp_now[0].peers[i].mac_ap[1];
        myObject[i]["mac_ap"][2] = conf_esp_now[0].peers[i].mac_ap[2];
        myObject[i]["mac_ap"][3] = conf_esp_now[0].peers[i].mac_ap[3];
        myObject[i]["mac_ap"][4] = conf_esp_now[0].peers[i].mac_ap[4];
        myObject[i]["mac_ap"][5] = conf_esp_now[0].peers[i].mac_ap[5];
      }
      xSemaphoreGive( Mutex_cnf_now );
    }
    String output;
    serializeJson(myObject, output);
    server.send(200, "text/json", output);
    //server.send(200, "text/json", JSON.stringify(myObject));
  });
  server.on("/peers_act", HTTP_GET, [](){
    StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
      for (uint8_t i = 0; i < MAXPEERS; i++)
      {
        myObject[i]["act"] = act_peers[i].act;
        myObject[i]["cnf_id"] = act_peers[i].cnf_id;
        myObject[i]["mac_addr"][0] = act_peers[i].mac_addr[0];
        myObject[i]["mac_addr"][1] = act_peers[i].mac_addr[1];
        myObject[i]["mac_addr"][2] = act_peers[i].mac_addr[2];
        myObject[i]["mac_addr"][3] = act_peers[i].mac_addr[3];
        myObject[i]["mac_addr"][4] = act_peers[i].mac_addr[4];
        myObject[i]["mac_addr"][5] = act_peers[i].mac_addr[5];
      }
      xSemaphoreGive( Mutex_cnf_now );
    }
    String output;
    serializeJson(myObject, output);
    server.send(200, "text/json", output);
    //server.send(200, "text/json", JSON.stringify(myObject));
  });
  server.on("/open", HTTP_PUT, [](){
    xSemaphoreGiveFromISR(openSemaphore, NULL);
    server.send(200, "text/plain", "Калитка открыта!");
  });
  server.on("/addPhalanx", HTTP_POST, [](){
    if (!check_autch()) return;
    if (figner_web != figner_start){
      if (add_phalanx()>=0){
        figner_web = figner_start;
        if( xSemaphoreTake( Mutex_cnf_finger, portMAX_DELAY ) == pdTRUE ){
          conf_finger[1][freeIdFinger].id = freeIdFinger+1;
          conf_finger[1][freeIdFinger].phalanx = (fingers)atoi(server.arg("ph").c_str());
          strcpy((char  *)conf_finger[1][freeIdFinger].name, server.arg("nm").c_str());
          xSemaphoreGive( Mutex_cnf_finger );
        }
        xSemaphoreGiveFromISR(addSemaphore, NULL);
        server.send(200, "text/plain", "Добавте палец!");
      }else{
        server.send(404, "text/plain", "Нет свободных пальцев!");
      }
    }else{
      server.send(404, "text/plain", "Занято другими действиями!");
    }
  });
  server.on("/sts_finger", HTTP_GET, [](){
    if (!check_autch()) return;
    StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
    myObject["sts"] = figner_web;
    myObject["id"] = freeIdFinger+1;
    if ((figner_web == figner_ok) || (figner_web == figner_err)) figner_web = figner_none;
    String output;
    serializeJson(myObject, output);
    server.send(200, "text/json", output);
    //server.send(200, "text/json", JSON.stringify(myObject));
  });
  server.on("/delPhalanx", HTTP_POST, [](){
    if (!check_autch()) return;
    StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
    if (figner_web != figner_start){
      figner_web = figner_start;
      myObject["p"] = deleteFingerprint(atoi(server.arg("id").c_str()));
      figner_web = figner_none;        
      String output;
      serializeJson(myObject, output);
      server.send(200, "text/json", output);
      //server.send(200, "text/json", JSON.stringify(myObject));
    }else{
      server.send(404, "text/plain", "Занято другими действиями!");
    }
  });
  server.on("/delFingers", HTTP_PUT, [](){
    if (!check_autch()) return;
    StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
    if (figner_web != figner_start){
      figner_web = figner_start;
      myObject["p"] = deleteAllFingerprint();
      figner_web = figner_none;
      String output;
      serializeJson(myObject, output);
      server.send(200, "text/json", output);
      //server.send(200, "text/json", JSON.stringify(myObject));
    }else{
      server.send(404, "text/plain", "Занято другими действиями!");
    }
  });
  server.on("/val", HTTP_GET, [](){
    StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
    timeval tv;
    gettimeofday(&tv, NULL);
    if( xSemaphoreTake( Mutex_si_measure, portMAX_DELAY ) == pdTRUE ){
      myObject["vol"]["temp"] = meas_si.temp;
      myObject["vol"]["hud"] = meas_si.humidi;
      myObject["vol"]["temp2"] = meas_si.temp_ds;
      myObject["tm"] = tv.tv_sec;
      xSemaphoreGive( Mutex_si_measure );;
    }
    String output;
    serializeJson(myObject, output);
    server.send(200, "text/json", output);
    //server.send(200, "text/json", JSON.stringify(myObject));
  });
  server.on("/search_ap", HTTP_GET, [](){
    server.send(200, "text/plain", "Start search!");
    vTaskDelay(150);
    scan_ap();
  });
  server.on("/get_ap", HTTP_GET, [](){
    server.send(200, "text/json", buf_scan);
  });
  server.on("/res", HTTP_PUT, [](){ /* fetch("/res",{method: "PUT"}); */
    if (!check_autch()) return;
    server.send(200, "text/plain", "Перезагрузка!");
    vTaskDelay(50);
    ESP.restart();
  });
  /*--------REMOVE ESP--------------*/
  server.on("/sts_now", HTTP_GET, [](){
    if (!check_autch()) return;
    StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    web_dop sts = web_status_peer(get_id_to_mac(buf_mac));
    myObject["sts"] = sts.sts;
    myObject["cmd"] = sts.cmd;
    String output;
    serializeJson(myObject, output);
    server.send(200, "text/json", output);
    //server.send(200, "text/json", JSON.stringify(myObject));
    if ((sts.cmd == web_add_me)&&(sts.sts == web_succs)) get_cnf_esp_now(buf_mac);
  });
  server.on("/rem_cnf", HTTP_GET, [](){
    StaticJsonDocument<1024> myObject;
  //JSONVar myObject;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      if (id >= 0){
        if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
          for (uint8_t i = 0; i < MAXPEERS; i++)
          {
            myObject["peers"][i]["act"] = cnf_con_peers[id].peers[i].act;
            myObject["peers"][i]["cnl"] = cnf_con_peers[id].peers[i].channel;
            myObject["peers"][i]["mac_addr"][0] = cnf_con_peers[id].peers[i].mac_addr[0];
            myObject["peers"][i]["mac_addr"][1] = cnf_con_peers[id].peers[i].mac_addr[1];
            myObject["peers"][i]["mac_addr"][2] = cnf_con_peers[id].peers[i].mac_addr[2];
            myObject["peers"][i]["mac_addr"][3] = cnf_con_peers[id].peers[i].mac_addr[3];
            myObject["peers"][i]["mac_addr"][4] = cnf_con_peers[id].peers[i].mac_addr[4];
            myObject["peers"][i]["mac_addr"][5] = cnf_con_peers[id].peers[i].mac_addr[5];
          }
          myObject["bluetooth"] = cnf_con_peers[id].bluetooth;
          myObject["wifi"] = cnf_con_peers[id].wifi;
          xSemaphoreGive( Mutex_cnf_now );
        }
        String output;
        serializeJson(myObject, output);
        server.send(200, "text/json", output);
        //server.send(200, "text/json", JSON.stringify(myObject));
      }else{
        server.send(404, "text/plain", "Device NotFound");
      }
    }
  });
  server.on("/rem_cnf", HTTP_POST, [](){ 
    if (!check_autch()) return; 
    if (!server.hasArg("plain")){
      server.send(501, "text/plain", "");
    }else{ 
      DynamicJsonDocument myObject(1024);
      DeserializationError error = deserializeJson(myObject, server.arg("plain"));
      if (error){
      //JSONVar myObject = JSON.parse(server.arg("plain"));
      //if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        server.send(510, "text/plain", "ERROR json not type");
      }else{
        uint8_t buf_mac[6];
        buf_mac[0] = (int)myObject["mac"][0];
        buf_mac[1] = (int)myObject["mac"][1];
        buf_mac[2] = (int)myObject["mac"][2];
        buf_mac[3] = (int)myObject["mac"][3];
        buf_mac[4] = (int)myObject["mac"][4];
        buf_mac[5] = (int)myObject["mac"][5];
        int id = get_id_to_mac(buf_mac);
        if (check_bisy(id)){
          server.send(502, "text/plain", "Device busy");
        }else{
          if (id >= 0 ){
            if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
              for (uint8_t i = 0; i < MAXPEERS; i++)
              {
                cnf_con_peers[id].peers[i].act = myObject["peers"][i]["act"];
                cnf_con_peers[id].peers[i].mac_addr[0] = (int)myObject["peers"][i]["mac_addr"][0];
                cnf_con_peers[id].peers[i].mac_addr[1] = (int)myObject["peers"][i]["mac_addr"][1];
                cnf_con_peers[id].peers[i].mac_addr[2] = (int)myObject["peers"][i]["mac_addr"][2];
                cnf_con_peers[id].peers[i].mac_addr[3] = (int)myObject["peers"][i]["mac_addr"][3];
                cnf_con_peers[id].peers[i].mac_addr[4] = (int)myObject["peers"][i]["mac_addr"][4];
                cnf_con_peers[id].peers[i].mac_addr[5] = (int)myObject["peers"][i]["mac_addr"][5];
              }
              cnf_con_peers[id].bluetooth = myObject["bluetooth"];
              cnf_con_peers[id].wifi = myObject["wifi"];
              xSemaphoreGive( Mutex_cnf_now );   
              server.send(200, "text/plain", "Start set cnf");
              vTaskDelay(50);     
              set_cnf_esp_now(buf_mac);
            }
          }else{
            server.send(404, "text/plain", "Device NotFound");
          }
        }
      }
    }
  });
  server.on("/add_me", HTTP_POST, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      web_peer_no.web.channel = atoi(server.arg("cnl").c_str());
      if ((!web_peer_no.web.channel)||(web_peer_no.web.channel > 13)){
        server.send(404, "text/plain", "chanell error");
      }else{
        server.send(200, "text/plain", "start add me");
        vTaskDelay(150);
        add_connects_me_to_peer(buf_mac);
      }
    }
  });
  server.on("/del_act", HTTP_POST, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    del_act_peir(buf_mac);
    vTaskDelay(50);
    server.send(200, "text/plain", "Delete ok!");
  });
  server.on("/del_me", HTTP_POST, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      server.send(200, "text/plain", "start delete me");
      vTaskDelay(150);
      del_connects_me_to_peer(buf_mac);
    }
  });
  server.on("/del_old", HTTP_POST, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    uint8_t del_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    char_to_mac(server.arg("mac_d").c_str(), del_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      server.send(200, "text/plain", "start del old peer");
      vTaskDelay(150);
      del_connects_mac_to_peer(buf_mac, del_mac);
    }
  });
  server.on("/clr_peer", HTTP_POST, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      server.send(200, "text/plain", "start clean peers");
      vTaskDelay(150);
      clear_connects_to_peer(buf_mac);
    }
  });
  server.on("/get_cnct", HTTP_GET, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      server.send(200, "text/plain", "start get connect peers");
      vTaskDelay(150);
      get_connects_to_peer(buf_mac);
    }
  });
  server.on("/set_ble", HTTP_POST, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      server.send(200, "text/plain", "start set ble");
      vTaskDelay(150);
      if (!strcmp(server.arg("set").c_str(), "true")) bluetooth_now_cnf(adpt_on, buf_mac);    
      else bluetooth_now_cnf(adpt_off, buf_mac);
    }
  });
  server.on("/set_wifi", HTTP_POST, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      server.send(200, "text/plain", "start set wifi");
      vTaskDelay(150);
      if (!strcmp(server.arg("set").c_str(), "true")) wifi_now_cnf(adpt_on, buf_mac);    
      else wifi_now_cnf(adpt_off, buf_mac);
    }
  });
  server.on("/get_now_cnf", HTTP_GET, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      server.send(200, "text/plain", "start get cnf");
      vTaskDelay(150);
      get_cnf_esp_now(buf_mac);
    }
  });
  server.on("/set_now_cnf", HTTP_POST, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      server.send(200, "text/plain", "start set cnf device");
      vTaskDelay(150);
      set_cnf_esp_now(buf_mac);
    }
  });
  // server.on("/meas_now", HTTP_GET, [](){ 
  //   if (!check_autch()) return;
  //   uint8_t buf_mac[6];
  //   char_to_mac(server.arg("mac").c_str(), buf_mac);
  //   int id = get_id_to_mac(buf_mac);
  //   if (check_bisy(id)){
  //     server.send(502, "text/plain", "Device busy");
  //   }else{
  //     server.send(200, "text/plain", "start measure");
  //     vTaskDelay(150);
  //     measure_esp_now(buf_mac);
  //   }
  // });
  server.on("/res_now", HTTP_POST, [](){ 
    if (!check_autch()) return;
    uint8_t buf_mac[6];
    char_to_mac(server.arg("mac").c_str(), buf_mac);
    int id = get_id_to_mac(buf_mac);
    if (check_bisy(id)){
      server.send(502, "text/plain", "Device busy");
    }else{
      server.send(200, "text/plain", "start reboot device");
      vTaskDelay(150);
      reboot_esp_now(buf_mac);
    }
  });
  /*--------END REMOVE ESP--------------*/
   server.on("/set_lan", HTTP_POST, [](){
    if (!check_autch()) return;
    if (!server.hasArg("plain")){ //Check if body received
      server.send(501, "text/plain", "Body not received");
      return;
    }
    uint8_t channel_old = conf_esp_now[0].channel;
    DynamicJsonDocument myObject(1024);
    DeserializationError error = deserializeJson(myObject, server.arg("plain"));
    if (error){
    // JSONVar myObject = JSON.parse(server.arg("plain"));
    // if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      server.send(510, "text/plain", "ERROR json not type");
    }else{
      if( xSemaphoreTake( Mutex_cnf_lan, portMAX_DELAY ) == pdTRUE ){  
        if (strcmp(myObject["pass_old"], (char  *)conf_lan[0].pass_autch)){
          xSemaphoreGive( Mutex_cnf_lan );
          server.send(510, "text/plain", "ERROR password!");
          return;
        }
        conf_lan[1].route = myObject["route"];
        conf_lan[1].softap = myObject["softap"];
        strcpy((char  *)conf_lan[1].mdns_name, myObject["mdns_name"]);
        strcpy((char  *)conf_lan[1].name, myObject["name"]);
        strcpy((char  *)conf_lan[1].pass, myObject["pass"]);
        strcpy((char  *)conf_lan[1].pass_esp, myObject["pass_esp"]);
        strcpy((char  *)conf_lan[1].pass_autch, myObject["pass_autch"]);
        strcpy((char  *)conf_lan[1].login, myObject["login"]);

        if (memcmp ((void*)&conf_lan[0], (void*)&conf_lan[1], sizeof(wifi_lan_cnf)) != 0){
            memcpy((void*)&conf_lan[0], (void*)&conf_lan[1], sizeof(wifi_lan_cnf));
            xSemaphoreGive( Mutex_cnf_lan );
            if (!saveconfig(lan_cnf)) {
                Serial.println("Failed to save config");
            } else {
                Serial.println("Config saved");
            }
        }else{
          xSemaphoreGive( Mutex_cnf_lan );
        }
      }
      server.send(200, "text/plain", "set lan");
      vTaskDelay(150);
      wifi_init();
      if (channel_old != conf_esp_now[0].channel) reinit_esp_channel();
    }
  });
  // server.on("/set_now_peer", HTTP_POST, [](){
  //   if (!check_autch()) return; 
  //   int id = atoi(server.arg("id").c_str());
  //   if(id>=0){
  //     if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
  //       conf_esp_now[1].meas[id].null_id = atoi(server.arg("null_id").c_str());
  //       conf_esp_now[1].meas[id].cnt = atoi(server.arg("cnt").c_str());
  //       if (!strcmp(server.arg("auto_measure").c_str(), "true")) conf_esp_now[1].meas[id].auto_measure =true;
  //       else conf_esp_now[1].meas[id].auto_measure = false;
  //       strcpy(conf_esp_now[1].peers[id].lastname, server.arg("lastname").c_str());        
  //       if (memcmp((void*)&conf_esp_now[0].meas[id], (void*)&conf_esp_now[1].meas[id], sizeof(conf_measure))||memcmp((void*)&conf_esp_now[0].peers[id], (void*)&conf_esp_now[1].peers[id], sizeof(esp_now_peers))){
  //           //strcpy(conf_esp_now[1].peers[id].name, conf_esp_now[0].peers[id].name); 
  //           strcpy(conf_esp_now[0].peers[id].lastname, server.arg("lastname").c_str()); 
  //           memcpy((void*)&conf_esp_now[0].meas[id], (void*)&conf_esp_now[1].meas[id], sizeof(conf_measure));
  //          // memcpy((void*)&conf_esp_now[0].peers[id], (void*)&conf_esp_now[1].peers[id], sizeof(esp_now_peers));
  //           xSemaphoreGive( Mutex_cnf_now );
  //           if (!saveconfig(now_cnf)) {
  //               Serial.println("Failed to save config");
  //           } else {
  //               Serial.println("Config saved");
  //           }
  //       }else{
  //         xSemaphoreGive( Mutex_cnf_now );
  //       }
  //       server.send(200, "text/plain", "set now to peer");
  //     }
  //   }else{
  //     server.send(404, "text/plain", "Device NotFound");
  //   }
  // });
  server.on("/set_now", HTTP_POST, [](){
    if (!check_autch()) return; 
    uint8_t channel_old = conf_esp_now[0].channel;
    if (!server.hasArg("plain")){
      server.send(501, "text/plain", "");
    }else{ 
      DynamicJsonDocument myObject(1024);
      DeserializationError error = deserializeJson(myObject, server.arg("plain"));
      if (error){
      // JSONVar myObject = JSON.parse(server.arg("plain"));
      // if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        server.send(510, "text/plain", "ERROR json not type");
      }else{        
        if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
          conf_esp_now[1].max_peer = MAXPEERS;
          conf_esp_now[1].channel = (int)myObject["channel"];
          conf_esp_now[1].role = conf_esp_now[0].role;
          for (uint8_t i = 0; i < MAXPEERS; i++)
          {
            conf_esp_now[1].peers[i].act = myObject["peers"][i]["act"];
            strcpy(conf_esp_now[1].peers[i].name, conf_esp_now[0].peers[i].name);
            strcpy(conf_esp_now[1].peers[i].lastname, myObject["peers"][i]["lastname"]);
            conf_esp_now[1].peers[i].mac_addr[0] = (int)myObject["peers"][i]["mac_addr"][0];
            conf_esp_now[1].peers[i].mac_addr[1] = (int)myObject["peers"][i]["mac_addr"][1];
            conf_esp_now[1].peers[i].mac_addr[2] = (int)myObject["peers"][i]["mac_addr"][2];
            conf_esp_now[1].peers[i].mac_addr[3] = (int)myObject["peers"][i]["mac_addr"][3];
            conf_esp_now[1].peers[i].mac_addr[4] = (int)myObject["peers"][i]["mac_addr"][4];
            conf_esp_now[1].peers[i].mac_addr[5] = (int)myObject["peers"][i]["mac_addr"][5];
            conf_esp_now[1].peers[i].cnf = conf_esp_now[0].peers[i].cnf;
          }
          if (memcmp((void*)&conf_esp_now[0], (void*)&conf_esp_now[1], sizeof(esp_now_cnf)) != 0){
              memcpy((void*)&conf_esp_now[0], (void*)&conf_esp_now[1], sizeof(esp_now_cnf));
              xSemaphoreGive( Mutex_cnf_now );
              if (!saveconfig(now_cnf)) {
                Serial.println("Failed to save config");
                server.send(200, "text/plain", "Failed set now");
              }else{
                server.send(200, "text/plain", "set now");
                Serial.println("Config saved");
                if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
                  xSemaphoreGive( Mutex_cnf_now );
                  vTaskDelay(150);
                  wifi_init();
                  if (channel_old != conf_esp_now[0].channel) reinit_esp_channel();
                }
              }
          }else{
            server.send(200, "text/plain", "set now");
            xSemaphoreGive( Mutex_cnf_now );
          }
        }
      }
    }
  });
  server.on("/set_time", HTTP_POST, [](){
    if (!check_autch()) return;
    if( xSemaphoreTake( Mutex_cnf_time, portMAX_DELAY ) == pdTRUE ){
      strcpy(conf_time[1].ntpServer1, server.arg("ntpServer1").c_str());
      strcpy(conf_time[1].ntpServer2, server.arg("ntpServer2").c_str());
      strcpy(conf_time[1].ntpServer3, server.arg("ntpServer3").c_str());
      conf_time[1].pereod_alarm = atoi(server.arg("pereod_alarm").c_str());
      if (!conf_time[1].pereod_alarm) conf_time[1].pereod_alarm = 10;
      if (memcmp ((void*)&conf_time[0], (void*)&conf_time[1], sizeof(time_cnf)) != 0){
          memcpy((void*)&conf_time[0], (void*)&conf_time[1], sizeof(time_cnf));
          xSemaphoreGive( Mutex_cnf_time );
          if (!saveconfig(times_cnf)) {
              Serial.println("Failed to save config");
          } else {
              Serial.println("Config saved");
          }
      }else{
        xSemaphoreGive( Mutex_cnf_time );
      }
    }
    server.send(200, "text/plain", "set time");
  });
  server.on("/set_server", HTTP_POST, [](){
    if (!check_autch()) return;
    if (!server.hasArg("plain")){
      server.send(501, "text/plain", "");
    }else{  
      DynamicJsonDocument myObject(1024);
      DeserializationError error = deserializeJson(myObject, server.arg("plain"));
      if (error){
      // JSONVar myObject = JSON.parse(server.arg("plain"));
      // if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        server.send(510, "text/plain", "ERROR json not type");
      }else{
        if( xSemaphoreTake( Mutex_cnf_server, portMAX_DELAY ) == pdTRUE ){
          conf_server[1].max_serv = conf_server[0].max_serv;
          for (uint8_t i = 0; i < conf_server[0].max_serv; i++)
          {
            conf_server[1].serv[i].act = myObject[i]["act"];
            strcpy((char  *)conf_server[1].serv[i].host, myObject[i]["host"]);
            conf_server[1].serv[i].port = (int)myObject[i]["port"];
          }
          if (memcmp ((void*)&conf_server[0], (void*)&conf_server[1], sizeof(servers_cnf)) != 0){
            memcpy((void*)&conf_server[0], (void*)&conf_server[1], sizeof(servers_cnf));
            xSemaphoreGive( Mutex_cnf_server );
            if (!saveconfig(server_cnf)) {
                Serial.println("Failed to save config");
            } else {
                Serial.println("Config saved");
            }
          }else{
            xSemaphoreGive( Mutex_cnf_server );
          }
        }
        server.send(200, "text/plain", "set server");
      }
    }    
  });
  server.on("/set_fingers", HTTP_POST, [](){
    if (!check_autch()) return;
      // String message = "Body received:\n";
        //       message += server.arg("plain");
        //       message += "\n";
        // Serial.println(message);     
      if (!server.hasArg("plain")){
        server.send(501, "text/plain", "");
      }else{
        DynamicJsonDocument myObject(1024);
        DeserializationError error = deserializeJson(myObject, server.arg("plain"));
        if (error){
        // JSONVar myObject = JSON.parse(server.arg("plain"));
        // if (JSON.typeof(myObject) == "undefined") {
          Serial.println("Parsing input failed!");
          server.send(510, "text/plain", "ERROR json not type");
        }else{
          if( xSemaphoreTake( Mutex_cnf_finger, portMAX_DELAY ) == pdTRUE ){
            for (uint8_t i = 0; i < MAXFINGER; i++)
            {
              conf_finger[1][i].act =  myObject[i]["act"];
              conf_finger[1][i].id = (int)myObject[i]["id"];
              strcpy((char  *)conf_finger[1][i].name, myObject[i]["name"]);
              conf_finger[1][i].phalanx = (fingers)(int)myObject[i]["phalanx"];
            }
            if (memcmp ((void*)&conf_finger[0], (void*)&conf_finger[1], sizeof(finger_cnf)) != 0){
              memcpy((void*)&conf_finger[0], (void*)&conf_finger[1], sizeof(fingers_cnf));
              xSemaphoreGive( Mutex_cnf_finger );
              if (!saveconfig(finger_cnf)) {
                  Serial.println("Failed to save config");
              } else {
                  Serial.println("Config saved");
              }
            }else{
              xSemaphoreGive( Mutex_cnf_finger );
            }
          }
          server.send(200, "text/plain", "set station");
        }    
      }
  });
  server.on("/cnf_set", HTTP_POST, [](){
    if (!check_autch()) return;
    if (!server.hasArg("plain")){ //Check if body received
      server.send(501, "text/plain", "Body not received");
      return;
    }
    DynamicJsonDocument myObject(1024);
    DeserializationError error = deserializeJson(myObject, server.arg("plain"));
    if (error){
    // JSONVar myObject = JSON.parse(server.arg("plain"));
    // if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      server.send(510, "text/plain", "ERROR json not type");
    }else{
      ver_s = (double)myObject["ver"];
      if (ver_s != ver){
        server.send(502, "text/plain", "ERROR version conflict!");
      }else{
        if (myObject["conf"]){
          if( xSemaphoreTake( Mutex_cnf_lan, portMAX_DELAY ) == pdTRUE ){
            conf_lan[1].route = myObject["lan"]["route"];
            conf_lan[1].softap = myObject["lan"]["softap"];    
            strcpy((char  *)conf_lan[1].mdns_name, myObject["lan"]["mdns_name"]);
            strcpy((char  *)conf_lan[1].name, myObject["lan"]["name"]);
            strcpy((char  *)conf_lan[1].pass, myObject["lan"]["pass"]);
            strcpy((char  *)conf_lan[1].pass_esp, myObject["lan"]["pass_esp"]);
            strcpy((char  *)conf_lan[1].login, myObject["lan"]["login"]);
            strcpy((char  *)conf_lan[1].pass_autch, myObject["lan"]["pass_autch"]);
            if (memcmp ((void*)&conf_lan[0], (void*)&conf_lan[1], sizeof(wifi_lan_cnf)) != 0){
              memcpy((void*)&conf_lan[0], (void*)&conf_lan[1], sizeof(wifi_lan_cnf));
              xSemaphoreGive( Mutex_cnf_lan );
              if (!saveconfig(lan_cnf)) {
                Serial.println("Failed to save config");
              } else {
                Serial.println("Config saved");
              }
            }else{
              xSemaphoreGive( Mutex_cnf_lan );
            }
          }
          if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
            conf_esp_now[1].max_peer = (int)myObject["now"]["max_peer"];
            conf_esp_now[1].channel = (int)myObject["now"]["channel"];
            conf_esp_now[1].role = (role_esp)(int)myObject["now"]["role"];
            if (memcmp((void*)&conf_esp_now[0], (void*)&conf_esp_now[1], sizeof(esp_now_cnf)) != 0){
              memcpy((void*)&conf_esp_now[0], (void*)&conf_esp_now[1], sizeof(esp_now_cnf));
              xSemaphoreGive( Mutex_cnf_now );
              if (!saveconfig(now_cnf)) {
                Serial.println("Failed to save config");
              } else {
                Serial.println("Config saved");
              }
            }else{
              xSemaphoreGive( Mutex_cnf_now );
            }
          }
          if( xSemaphoreTake( Mutex_cnf_time, portMAX_DELAY ) == pdTRUE ){
            strcpy(conf_time[1].ntpServer1, myObject["tim"]["ntpServer1"]);
            strcpy(conf_time[1].ntpServer2, myObject["tim"]["ntpServer2"]);
            strcpy(conf_time[1].ntpServer3, myObject["tim"]["ntpServer3"]);
            conf_time[1].pereod_alarm = (int)myObject["tim"]["pereod_alarm"];
            if (!conf_time[1].pereod_alarm) conf_time[1].pereod_alarm = 10;
            if (memcmp ((void*)&conf_time[0], (void*)&conf_time[1], sizeof(time_cnf)) != 0){
              memcpy((void*)&conf_time[0], (void*)&conf_time[1], sizeof(time_cnf));
              xSemaphoreGive( Mutex_cnf_time );
              if (!saveconfig(times_cnf)) {
                Serial.println("Failed to save config");
              } else {
                Serial.println("Config saved");
              }
            }else{
              xSemaphoreGive( Mutex_cnf_time );
            }
          }
          if( xSemaphoreTake( Mutex_cnf_server, portMAX_DELAY ) == pdTRUE ){
            conf_server[1].max_serv = (int)myObject["ser"]["max_serv"];
            for (uint8_t i = 0; i < conf_server[0].max_serv; i++)
            {
              conf_server[1].serv[i].act = myObject["ser"]["serv"][i]["act"];
              strcpy((char  *)conf_server[1].serv[i].host, myObject["ser"]["serv"][i]["host"]);
              conf_server[1].serv[i].port = (int)myObject["ser"]["serv"][i]["port"];
            }
            if (memcmp ((void*)&conf_server[0], (void*)&conf_server[1], sizeof(servers_cnf)) != 0){
              memcpy((void*)&conf_server[0], (void*)&conf_server[1], sizeof(servers_cnf));
              xSemaphoreGive( Mutex_cnf_server );
              if (!saveconfig(server_cnf)) {
                Serial.println("Failed to save config");
              } else {
                Serial.println("Config saved");
              }
            }else{
              xSemaphoreGive( Mutex_cnf_server );
            }
          }
          if( xSemaphoreTake( Mutex_cnf_finger, portMAX_DELAY ) == pdTRUE ){
            for (uint8_t i = 0; i < MAXFINGER; i++)
            {      
              conf_finger[1][i].act =  myObject[i]["act"];
              conf_finger[1][i].id = (int)myObject[i]["id"];
              strcpy((char  *)conf_finger[1][i].name, myObject[i]["name"]);
              conf_finger[1][i].phalanx = (fingers)(int)myObject[i]["phalanx"];
            }
            if (memcmp ((void*)&conf_finger[0], (void*)&conf_finger[1], sizeof(fingers_cnf)) != 0){
              memcpy((void*)&conf_finger[0], (void*)&conf_finger[1], sizeof(fingers_cnf));
              xSemaphoreGive( Mutex_cnf_finger );
              if (!saveconfig(finger_cnf)) {
                Serial.println("Failed to save config");
              } else {
                Serial.println("Config saved");
              }
            }else{
              xSemaphoreGive( Mutex_cnf_finger );
            }
          }
          send_redir("config");
          //server.send(200, "text/plain", "");
          //if (!conf_esp_now[0].new_device){
            vTaskDelay(500);
            wifi_init();
          //}
        }else{
          server.send(510, "text/plain", "ERROR json not type");
        }
      }
    }

      // String message = "Body received:\n";
      //        message += server.arg("plain");
      //        message += "\n";
 
      // server.send(200, "text/plain", message);
      // Serial.println(message);
  });
  server.on("/cnf_get", HTTP_GET, [](){
    StaticJsonDocument<1024> myObject;
    //JSONVar myObject;
    myObject["conf"] = true;
    myObject["ver"] = ver;
    if( xSemaphoreTake( Mutex_cnf_lan, portMAX_DELAY ) == pdTRUE ){
      myObject["lan"]["route"] = conf_lan[0].route;
      myObject["lan"]["softap"] = conf_lan[0].softap;    
      myObject["lan"]["mdns_name"] = conf_lan[0].mdns_name;
      myObject["lan"]["name"] = conf_lan[0].name;
      myObject["lan"]["pass"] = conf_lan[0].pass;
      myObject["lan"]["pass_esp"] = conf_lan[0].pass_esp;
      myObject["lan"]["login"] = conf_lan[0].login;
      myObject["lan"]["pass_autch"] = conf_lan[0].pass_autch;
      xSemaphoreGive( Mutex_cnf_lan );
    }
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
      myObject["now"]["max_peer"] = MAXPEERS;
      myObject["now"]["channel"] = conf_esp_now[0].channel;
      myObject["now"]["role"] = conf_esp_now[0].role;
      xSemaphoreGive( Mutex_cnf_now );
    }
    if( xSemaphoreTake( Mutex_cnf_time, portMAX_DELAY ) == pdTRUE ){
      myObject["tim"]["ntpServer1"] = conf_time[0].ntpServer1;
      myObject["tim"]["ntpServer2"] = conf_time[0].ntpServer2;
      myObject["tim"]["ntpServer3"] = conf_time[0].ntpServer3;
      myObject["tim"]["pereod_alarm"] = conf_time[0].pereod_alarm;
      xSemaphoreGive( Mutex_cnf_time );
    }
    if( xSemaphoreTake( Mutex_cnf_server, portMAX_DELAY ) == pdTRUE ){
      myObject["ser"]["max_serv"]=conf_server[0].max_serv;
      for (uint8_t i = 0; i < MAXSERVERS; i++)
      {
        myObject["ser"]["serv"][i]["act"] = conf_server[0].serv[i].act;
        myObject["ser"]["serv"][i]["host"] = conf_server[0].serv[i].host;
        myObject["ser"]["serv"][i]["port"] = conf_server[0].serv[i].port;
      }
      xSemaphoreGive( Mutex_cnf_server );
    }
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
    String output;
    serializeJson(myObject, output);
    server.send(200, "text/json", output);
    //server.send(200, "text/json", JSON.stringify(myObject));
  });
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );
  server.begin();
  Serial.println("HTTP server started");
 }






