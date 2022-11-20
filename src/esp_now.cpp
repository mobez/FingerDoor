#include "espnow.h"
#include "times.h"
#include "wifi_conf.h"

extern wifi_lan_cnf conf_lan[2];
extern esp_now_cnf conf_esp_now[2];

extern SemaphoreHandle_t Mutex_cnf_lan;
extern SemaphoreHandle_t Mutex_cnf_now;
extern SemaphoreHandle_t Mutex_value;
extern SemaphoreHandle_t sendSemaphore;
// extern SemaphoreHandle_t measSemaphore;

uint8_t buf_now[250];
uint8_t buf_now_recb[250];
volatile bool reafl = false;
uint8_t mac_rev[6];


web_mac_no_peer web_peer_no;


mess_now *mess_send;
mess_now *mess_resive;

esp_now_peer_info_t peerInfo;

timeval tv_now;
measure_work sts_meas;

esp_now_peers_act act_peers[MAXPEERS];
esp_now_cnf_dev cnf_con_peers[MAXPEERS];


bool check_mac(uint8_t *mac){
    bool v=false;
    for (uint8_t i = 0; i < 6; i++)
    {
        if(mac[i] != 0){
            v=true;
            break;
        }
    }
    return v;
}
int get_id_to_mac(const uint8_t *mac_addr) {
  int r = -1;
  if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
    for (uint8_t i = 0; i < MAXPEERS; i++)
    {
      if (conf_esp_now[0].peers[i].act){
        if (memcmp (conf_esp_now[0].peers[i].mac_addr, mac_addr, 6) == 0) {
          r = i;
          break;
        }
      }
    }
    xSemaphoreGive( Mutex_cnf_now );
  }
  return r;
}
int get_id_to_act_mac(const uint8_t *mac_addr) {
  int r = -1;
  if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
    for (uint8_t i = 0; i < MAXPEERS; i++)
    {
      if (act_peers[i].act){
        if (memcmp (act_peers[i].mac_addr, mac_addr, 6) == 0) {
          r = i;
          break;
        }
      }
    }
    xSemaphoreGive( Mutex_cnf_now );
  }
  return r;
}

// Callback-функция для получения состояния отправки

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t sendStatus) {
  //Serial.print("Last Packet Send Status: ");
  if (sendStatus == ESP_NOW_SEND_SUCCESS){
    Serial.println("Delivery success");
  }
  else{ 
    if (sts_meas.sts == Meas_work){
      if (memcmp (conf_esp_now[0].peers[sts_meas.mac_id].mac_addr, mac_addr, 6) == 0){
        sts_meas.sts = Meas_fail;
        // xSemaphoreGiveFromISR(measSemaphore, NULL);
      }
    }else{
      if (memcmp (web_peer_no.mac, mac_addr, 6) == 0){
        web_peer_no.web.sts = web_fail;
      }else{
        int id = get_id_to_mac(mac_addr);
        if (id >= 0) cnf_con_peers[id].web.sts = web_fail;
      }
    }
    Serial.println("Delivery fail");
    del_act_peir(mac_addr);
  }
  xSemaphoreGiveFromISR(sendSemaphore, NULL);
}
// То же, для индикации состояния приема данных
bool check_bisy(int id){
  return ((!chek_timout_now(id))&&(((id>=0)&&(cnf_con_peers[id].web.sts))||((id<0)&&(web_peer_no.web.sts))));
}

bool chek_timout_now(int id){
  if (gettimeofday(&tv_now, NULL)!= 0) {
    Serial.println("Failed to obtain time");
  } 
  if (id >= 0){
    return tv_now.tv_sec > cnf_con_peers[id].web.time_start;
  }else{
    return tv_now.tv_sec > web_peer_no.web.time_start;
  }
}

web_dop web_status_peer(int id){
  web_dop stat;
  if (gettimeofday(&tv_now, NULL)!= 0) {
    Serial.println("Failed to obtain time");
  }  
  stat.time_start = tv_now.tv_sec;
  if (id >= 0){
    stat.cmd = cnf_con_peers[id].web.cmd;
    if ((cnf_con_peers[id].web.sts == web_work) && (chek_timout_now(id))){
      cnf_con_peers[id].web.sts = web_fail;
      stat.sts = cnf_con_peers[id].web.sts;
    }else{
      stat.sts = cnf_con_peers[id].web.sts;
    }
    if ((cnf_con_peers[id].web.sts == web_fail) || (cnf_con_peers[id].web.sts == web_succs)){
      cnf_con_peers[id].web.sts = web_no_act;
      cnf_con_peers[id].web.cmd = web_no_cmd;
    }
  }else{
    stat.cmd = web_peer_no.web.cmd;
    if ((web_peer_no.web.sts == web_work) && (chek_timout_now(id))){
      web_peer_no.web.sts = web_fail;
      stat.sts = web_peer_no.web.sts;
    }else{
      stat.sts = web_peer_no.web.sts;
    }
    if ((web_peer_no.web.sts == web_fail) || (web_peer_no.web.sts == web_succs)){
      web_peer_no.web.sts = web_no_act;
      web_peer_no.web.cmd = web_no_cmd;
      if (stat.sts == web_fail) del_act_peir(web_peer_no.mac);
      memset(web_peer_no.mac, 0, 6);
    }
  } 
  return stat;
}

void send_web_cmd(const uint8_t *mac_addr, camand_now cmd, web_cmd cmd_web, uint8_t len_data){
  mess_send->camand = cmd;
  Serial.println("send web cmd");
  send_now(mac_addr, (uint8_t *)mess_send, sizeof(camand_now)+len_data);
  Serial.println("send web end");
  int id = get_id_to_mac(mac_addr);
  if (gettimeofday(&tv_now, NULL)!= 0) {
    Serial.println("Failed to obtain time");
  }
  if (id >= 0){
    cnf_con_peers[id].web.cmd = cmd_web;
    cnf_con_peers[id].web.sts = web_work;
    if (cmd_web == web_start_meas){
      cnf_con_peers[id].web.time_start = tv_now.tv_sec+70;
    }else{
      cnf_con_peers[id].web.time_start = tv_now.tv_sec+10;
    }
  }else{
    memcpy(web_peer_no.mac, mac_addr, 6);
    web_peer_no.web.cmd = web_add_me;
    web_peer_no.web.sts = web_work;
    web_peer_no.web.time_start = tv_now.tv_sec+10;
  }
}

void send_now(const uint8_t *mac_addr, uint8_t *data, uint8_t len) {
  if (add_peer_fors(mac_addr)){ 
    xSemaphoreTake(sendSemaphore, 5000);
    esp_now_send(mac_addr, (uint8_t *)data, (int)len);
  }else{
    web_peer_no.web.sts == web_fail;
  }
}

void set_channel_peer(const uint8_t *mac_addr, uint8_t channel){
    conf_esp_now[1].peers->channel = channel;
    wifi_init(1);
    mess_send->data[0] = channel;
    send_web_cmd(mac_addr, set_channel, web_set_channel);
    vTaskDelay(500);
    wifi_init();
}
void set_channel_peers(void){  
  for (uint8_t i = 0; i < MAXPEERS; i++)
  {
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
      if ((conf_esp_now[0].peers[i].act)&&(conf_esp_now[0].peers->channel != conf_esp_now[0].channel)){
        xSemaphoreGive( Mutex_cnf_now );
        set_channel_peer(conf_esp_now[0].peers->mac_addr, conf_esp_now[0].peers->channel);
        if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
          conf_esp_now[0].peers[i].channel = conf_esp_now[0].channel;
          xSemaphoreGive( Mutex_cnf_now );
        }
      }else{
        xSemaphoreGive( Mutex_cnf_now );
      }
    }
  }
}

void add_connects_me_to_peer(const uint8_t *mac_addr){
  if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){  
    if (conf_esp_now[0].channel != web_peer_no.web.channel) {   
      xSemaphoreGive( Mutex_cnf_now );   
      set_channel_peer(mac_addr, web_peer_no.web.channel);  
      Serial.println("Send set channel");
      if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){ 
        if (conf_esp_now[0].channel != web_peer_no.web.channel) {  
          web_peer_no.web.channel = conf_esp_now[0].channel;
        }
        xSemaphoreGive( Mutex_cnf_now ); 
      }
    }else{
      xSemaphoreGive( Mutex_cnf_now );
    }    
  }
  Serial.println("Send add me"); 
  send_web_cmd(mac_addr, add_me, web_add_me);  
}
void del_connects_me_to_peer(const uint8_t *mac_addr){
  send_web_cmd(mac_addr, del_me, web_no_cmd);
  vTaskDelay(100);
  del_peir(mac_addr);
  saveconfig(now_cnf);
}
void del_connects_mac_to_peer(const uint8_t *mac_addr, const uint8_t *mac_del){
  memcpy(mess_send->data, mac_del, 6);
  send_web_cmd(mac_addr, del_peer, web_del_peer, 6);
}
void clear_connects_to_peer(const uint8_t *mac_addr){
  send_web_cmd(mac_addr, cler_peer, web_clear_peers);
}
void get_connects_to_peer(const uint8_t *mac_addr){
  send_web_cmd(mac_addr, get_peers, web_get_peers);
}
void bluetooth_now_cnf(cnf_adapt on_off, const uint8_t *mac_addr){
  camand_now cmd = bluetooth_off;
  if (on_off)  cmd = bluetooth_on;
  send_web_cmd(mac_addr, cmd, web_ble_on_off);
}
void wifi_now_cnf(cnf_adapt on_off, const uint8_t *mac_addr){
  camand_now cmd = wifi_off;
  if (on_off)  cmd = wifi_on;
  send_web_cmd(mac_addr, cmd, web_wifi_on_off);
}
void wifi_ble_now_cnf(const uint8_t *mac_addr){
  send_web_cmd(mac_addr, get_wifi_ble_cnf, web_get_wifi_ble);
}
void get_cnf_esp_now(const uint8_t *mac_addr){
  send_web_cmd(mac_addr, read_conf, web_get_cnf);
}
void set_cnf_esp_now(const uint8_t *mac_addr){
  int id = get_id_to_mac(mac_addr);
  if (id >= 0){
    memcpy(mess_send->data, (void *)&cnf_con_peers[id], sizeof(esp_now_cnf_dev)-sizeof(web_dop));
    send_web_cmd(mac_addr, set_conf, web_set_cnf, sizeof(esp_now_cnf_dev)-sizeof(web_dop));
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println("Send save config "); Serial.println(macStr);
  }else{
    cnf_con_peers[id].web.cmd = web_set_cnf;
    cnf_con_peers[id].web.sts = web_fail;
  }
}
// void measure_esp_now(const uint8_t *mac_addr){
//   int id = get_id_to_mac(mac_addr);
//   if (id >= 0){
//     if (!conf_esp_now[0].meas[id].auto_measure){
//       send_web_cmd(mac_addr, start_measure, web_start_meas);
//     }else{
//       cnf_con_peers[id].web.cmd = web_start_meas;
//       cnf_con_peers[id].web.sts = web_succs;
//     }
//   }else{
//     cnf_con_peers[id].web.cmd = web_start_meas;
//     cnf_con_peers[id].web.sts = web_fail;    
//   }
// }
void reboot_esp_now(const uint8_t *mac_addr){
  send_web_cmd(mac_addr, reboot_esp, web_reboot);
}

void act_peer(const uint8_t *mac_addr){
  
  for (uint8_t i = 0; i < MAXPEERS; i++)
  {
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
      if (!act_peers[i].act){
        act_peers[i].act = true;
        memcpy(act_peers[i].mac_addr, mac_addr, 6);
        xSemaphoreGive( Mutex_cnf_now );
        act_peers[i].cnf_id = get_id_to_mac(mac_addr);
        break;
      }else{
        xSemaphoreGive( Mutex_cnf_now );
      }      
    }
  }
}
void add_conf_peer(const uint8_t *mac_addr, name_now *nm){
  if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
    for (uint8_t i = 0; i < MAXPEERS; i++)
    {
      if (!conf_esp_now[0].peers[i].act){
        conf_esp_now[0].peers[i].act = true;
        conf_esp_now[0].peers[i].channel = conf_esp_now[0].channel;
        memcpy(conf_esp_now[0].peers[i].mac_addr, mac_addr, 6);
        memcpy(conf_esp_now[0].peers[i].mac_ap, nm->mac, 6);
        strcpy(conf_esp_now[0].peers[i].name, nm->name);     
        break;
      }
    }
    xSemaphoreGive( Mutex_cnf_now );
  }
}


bool add_peer_fors(const uint8_t *mac_addr){
  bool exists = esp_now_is_peer_exist(mac_addr);
  if ( exists) {
      // Slave already paired.
      //Serial.print("Slave Status: ");
      //Serial.println("Already Paired");
      return true;
    }else{
      Serial.print("Slave Status: ");

      // Slave not paired, attempt pair
      memcpy(peerInfo.peer_addr, mac_addr, 6);
      if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
        peerInfo.channel = conf_esp_now[0].channel;  
        xSemaphoreGive( Mutex_cnf_now );
      }
      peerInfo.encrypt = false;
      //peerInfo.ifidx = WIFI_IF_AP;
      peerInfo.ifidx = WIFI_IF_STA;
      esp_err_t addStatus = esp_now_add_peer(&peerInfo);
      if (addStatus == ESP_OK) {
        // Pair success
        Serial.println("Pair success");
        act_peer(mac_addr);
        return true;
      } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW Not Init");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        Serial.println("Peer list full");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("Out of memory");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Peer Exists");
        return true;
      } else {
        Serial.println("Not sure what happened");
        return false;
      } 
    }   
}

void del_peir(const uint8_t *mac_addr){
  int id = get_id_to_mac(mac_addr);
  if (id>-1){
    esp_err_t delStatus;
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){ 
      delStatus = esp_now_del_peer(conf_esp_now[0].peers[id].mac_addr);
      xSemaphoreGive( Mutex_cnf_now ); 
    }
    Serial.print("Slave Delete Status: ");
    if (delStatus == ESP_OK) {
      // Delete success
      Serial.println("Success");
    } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
      // How did we get so far!!
      Serial.println("ESPNOW Not Init");
    } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
      Serial.println("Invalid Argument");
    } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
      Serial.println("Peer not found.");
    } else {
      Serial.println("Not sure what happened");
    }
    conf_esp_now[0].peers[id].act = false;
    id = get_id_to_act_mac(mac_addr);
    if (id>-1) act_peers[id].act = false;
  }
}

void del_act_peir(const uint8_t *mac_addr){
  int id = get_id_to_act_mac(mac_addr);
  if (id>-1){
    esp_err_t delStatus;
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){ 
      delStatus = esp_now_del_peer(act_peers[id].mac_addr);
      xSemaphoreGive( Mutex_cnf_now );
    }
    Serial.print("Slave Delete Status: ");
    if (delStatus == ESP_OK) {
      // Delete success
      Serial.println("Success");
    } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
      // How did we get so far!!
      Serial.println("ESPNOW Not Init");
    } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
      Serial.println("Invalid Argument");
    } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
      Serial.println("Peer not found.");
    } else {
      Serial.println("Not sure what happened");
    }
    act_peers[id].act = false;
  }
}

void ret_send(void){
  reafl=false;
  mess_resive = (mess_now*)buf_now_recb;
  switch (mess_resive->camand)
    {
      case get_name_now:
      {
        mess_send->camand = send_name_now;
        char *name_now_send = (char*)mess_send->data;
        if( xSemaphoreTake( Mutex_cnf_lan, portMAX_DELAY ) == pdTRUE ){
          memcpy(name_now_send, (char  *)conf_lan[0].mdns_name, 21);
          xSemaphoreGive( Mutex_cnf_lan );
        }

        send_now(mac_rev, (uint8_t *)mess_send, sizeof(camand_now)+21);
        break;
      }
    }
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    mess_resive = (mess_now*)data;
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println("Last Packet Recv from: "); Serial.println(macStr);
    add_peer_fors(mac_addr);
    // int id =get_id_to_act_mac(mac_addr);
    int id_cnf = get_id_to_mac(mac_addr);
    switch (mess_resive->camand)
    {
      case get_name_now:
        {
          Serial.println("get name");
          // memcpy(buf_now_recb, data, data_len);
          // memcpy(mac_rev, mac_addr, 6);
          // reafl=true;
            mess_send->camand = send_name_now;
            name_now *nm = (name_now*)mess_send->data;            
            char_to_mac(WiFi.softAPmacAddress().c_str(), nm->mac);
            //char *name_now_send = (char*)mess_send->data;
            // memcpy(name_now_send, conf_lan[0].mdns_name, 35);
            if( xSemaphoreTake( Mutex_cnf_lan, portMAX_DELAY ) == pdTRUE ){
              strcpy(nm->name, (char  *)conf_lan[0].mdns_name);
              xSemaphoreGive( Mutex_cnf_lan );
            }
            //esp_now_send(mac_addr, (uint8_t *)mess_send, sizeof(camand_now)+35);
            send_now(mac_addr, (uint8_t *)mess_send, sizeof(camand_now)+sizeof(nm->mac)+strlen(nm->name));
            break;
        }
      case send_name_now:
        {
            Serial.print("revert name:");
            name_now *nm = (name_now*)mess_resive->data;
            add_conf_peer(mac_addr, nm);
            Serial.println(nm->name);
            snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
            nm->mac[0], nm->mac[1], nm->mac[2], nm->mac[3], nm->mac[4], nm->mac[5]);
              Serial.print("Name from: "); Serial.println(macStr);
            del_act_peir(nm->mac);
            memset(web_peer_no.mac, 0, 6);
            cnf_con_peers[id_cnf].web.cmd = web_add_me;
            cnf_con_peers[id_cnf].web.sts = web_succs;
            web_peer_no.web.sts = web_succs;
            saveconfig(now_cnf);
            break;
        }
      // case send_measure_val:
      //   {
      //     timeval tv;
      //     gettimeofday(&tv, NULL);          
      //     measure_now *meass_dt = (measure_now*)mess_resive->data;
      //     // Serial.print("Temp: ");
      //     // Serial.println(meass_dt->value[1]);
      //     Serial.println("Recv measure");          
      //     if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
      //       if ((id_cnf >= 0) && (conf_esp_now[0].meas[id_cnf].cnt)){
      //         if ( xSemaphoreTake( Mutex_value, portMAX_DELAY ) == pdTRUE ){    
      //           uint8_t cnts_vl=  conf_esp_now[0].meas[id_cnf].cnt;
      //           if (conf_esp_now[0].meas[id_cnf].cnt > meass_dt->cnt) cnts_vl = meass_dt->cnt;
      //           cnts_vl += conf_esp_now[0].meas[id_cnf].null_id;
      //           for (uint8_t i = conf_esp_now[0].meas[id_cnf].null_id; i < cnts_vl; i++)
      //           {
      //             vals.volue[i] = meass_dt->value[i];    
      //           }
      //           vals.time = tv.tv_sec;
      //           xSemaphoreGive( Mutex_value );
      //         }              
      //       }
      //       xSemaphoreGive( Mutex_cnf_now );
      //     }
      //     if ((sts_meas.sts == Meas_work)&&(sts_meas.mac_id == id_cnf)){
      //       sts_meas.sts = Meas_succs;
      //       xSemaphoreGiveFromISR(measSemaphore, NULL);
      //     }
      //     break;
      //   }
      case set_conf_ok:
        cnf_con_peers[id_cnf].web.sts = web_succs;
        Serial.println("Set config ok!");
        break;
      case send_conf:
        Serial.println("Resive config");
        if (id_cnf >= 0) memcpy((void *)&cnf_con_peers[id_cnf], mess_resive->data, sizeof(esp_now_cnf_dev)-sizeof(web_dop));
        cnf_con_peers[id_cnf].web.sts = web_succs;
        conf_esp_now[0].peers[id_cnf].cnf = true;
        Serial.println("Copy ok!");
        break;
      case add_ok:
        mess_send->camand = get_name_now;
        send_now(mac_addr, buf_now, sizeof(mess_now));
        Serial.println("My addr add to peer!");
        break;
      case del_peer_ok:
        if (id_cnf >= 0) cnf_con_peers[id_cnf].web.sts = web_succs;
        break;
      case clear_ok:
        del_peir(mac_addr);
        if (id_cnf >= 0) cnf_con_peers[id_cnf].web.sts = web_succs;
        web_peer_no.web.sts = web_succs;
        break;
      case send_peers:
        if (id_cnf >= 0) memcpy((void *)cnf_con_peers[id_cnf].peers, mess_resive->data, sizeof(esp_now_peers)*MAXPEERS);
        if (id_cnf >= 0) cnf_con_peers[id_cnf].web.sts = web_succs;
        break;
      case save_peers_ok:
        Serial.println("Save peers!");
        if (id_cnf >= 0) cnf_con_peers[id_cnf].web.sts = web_succs;
        break;
      case bluetooth_on_ok:
        Serial.println("Bluetooth on!");
        if (id_cnf >= 0) cnf_con_peers[id_cnf].bluetooth = true;
        cnf_con_peers[id_cnf].web.sts = web_succs;
        break;
      case bluetooth_off_ok:
        Serial.println("Bluetooth off!");
        if (id_cnf >= 0) cnf_con_peers[id_cnf].bluetooth = false;
        cnf_con_peers[id_cnf].web.sts = web_succs;
        break;
      case wifi_on_ok:
        Serial.println("Wifi on!");
        if (id_cnf >= 0) cnf_con_peers[id_cnf].wifi = true;
        cnf_con_peers[id_cnf].web.sts = web_succs;
        break;
      case wifi_off_ok:
        Serial.println("Wifi off!");
        if (id_cnf >= 0) cnf_con_peers[id_cnf].wifi = false;
        cnf_con_peers[id_cnf].web.sts = web_succs;
        break;
      case reboot_esp_ok:
        Serial.println("Reboot!");
        cnf_con_peers[id_cnf].web.sts = web_succs;
        break;
      case send_wifi_ble_cnf:
        if (id_cnf >= 0) memcpy((void *)&cnf_con_peers[id_cnf].bluetooth, mess_resive->data, sizeof(bool)*2);
        cnf_con_peers[id_cnf].web.sts = web_succs;
        break;
      case start_measure_ok:
        Serial.println("Start measure!");
        if (id_cnf >= 0) cnf_con_peers[id_cnf].web.sts = web_work;
        break;
      case end_measure:
        mess_send->camand = get_measure_val;
        send_now(mac_addr, (uint8_t *)mess_send, sizeof(camand_now));
        if (id_cnf >= 0) cnf_con_peers[id_cnf].web.sts = web_succs;
        break;
      default:
        break;
    }
    Serial.print("Bytes received: ");
    Serial.println(data_len);
}

void reinit_esp_channel(void) {
  set_channel_peers();
  if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
    for (uint8_t i = 0; i < MAXPEERS; i++)
    {
      if (conf_esp_now[0].peers[i].act){
        memcpy(peerInfo.peer_addr, conf_esp_now[0].peers[i].mac_addr, 6);
        peerInfo.channel = conf_esp_now[0].channel;  
        peerInfo.encrypt = false;
        peerInfo.ifidx = WIFI_IF_STA;
        esp_now_mod_peer(&peerInfo);
      }
    }
    xSemaphoreGive( Mutex_cnf_now );
  }
}

void get_cnf_peers_all(void){
  for (uint8_t i = 0; i < MAXPEERS; i++)
  {
    if( xSemaphoreTake( Mutex_cnf_now, portMAX_DELAY ) == pdTRUE ){
      if (conf_esp_now[0].peers[i].act){
        mess_send->camand = read_conf;
        xSemaphoreGive( Mutex_cnf_now );
        send_now(conf_esp_now[0].peers[i].mac_addr, (uint8_t *)mess_send, sizeof(camand_now));
        sts_meas.mac_id = i;
        sts_meas.sts = Meas_work;
        mess_send->camand = start_measure;
        send_now(conf_esp_now[0].peers[i].mac_addr, (uint8_t *)mess_send, sizeof(camand_now));
        // xSemaphoreTake(measSemaphore, 60000);
      }else{
        xSemaphoreGive( Mutex_cnf_now );
      } 
    }   
  }
}

void init_esp_now(void) {
    Serial.println("init bufer");
    mess_send = (mess_now*)buf_now;
    sts_meas.sts = No_work;
    Serial.println("esp-now init!");
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    Serial.println("esp-now ok!");
    // Регистрируем callback-функцию для получения статуса отправки
    Serial.println("init send cb");
    esp_now_register_send_cb(OnDataSent);
    // Регистрируем callback-функцию для получения статуса приема
    Serial.println("init recv cb");
    esp_now_register_recv_cb(OnDataRecv);
    get_cnf_peers_all();
}







