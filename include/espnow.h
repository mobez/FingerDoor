#ifndef ESPNOW_H_
#define ESPNOW_H_
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "config.h"

enum cnf_adapt{
  adpt_off = 0,
  adpt_on = 1,
};

enum camand_now{
  get_name_now = 0,
  send_name_now,
  start_measure,
  start_measure_ok,
  end_measure,
  get_measure_val,
  send_measure_val,
  set_conf,
  set_conf_ok,
  read_conf,
  send_conf,
  set_channel,
  add_me,
  add_ok,
  del_me,
  del_peer,
  del_peer_ok,
  cler_peer,
  clear_ok,
  get_peers,
  send_peers,
  save_peers,
  save_peers_ok,
  bluetooth_on,
  bluetooth_on_ok,
  bluetooth_off,
  bluetooth_off_ok,
  wifi_on,
  wifi_on_ok,
  wifi_off,
  wifi_off_ok,
  get_wifi_ble_cnf,
  send_wifi_ble_cnf,
  reboot_esp,
  reboot_esp_ok
};

typedef struct {
  uint16_t cnt;
  float value[];
}measure_now;

typedef struct {
  uint8_t mac[6];
  char name[];
}name_now;

typedef struct {
  camand_now camand;
  uint8_t data[];
}mess_now;

// typedef struct
// {
//   bool act;
//   uint8_t mac_addr[6];
// }esp_now_peers;
enum web_sts{
  web_no_act,
  web_work,
  web_fail,
  web_succs,
};

enum web_cmd{
  web_no_cmd,
  web_add_me,
  web_del_me,
  web_del_peer,
  web_clear_peers,
  web_get_peers,
  web_save_peers,
  web_ble_on_off,
  web_wifi_on_off,
  web_get_wifi_ble,
  web_start_meas,
  web_get_cnf,
  web_set_cnf,
  web_set_channel,
  web_reboot
};

typedef struct
{
  web_sts sts;
  web_cmd cmd;
  uint8_t channel;
  long time_start;
}web_dop;

typedef struct
{
  bool act;
  uint8_t channel;
  uint8_t mac_addr[6];
}esp_now_peers_re;

typedef struct
{
  bool act;
  int cnf_id;
  uint8_t mac_addr[6];
}esp_now_peers_act;

typedef struct 
{
  esp_now_peers_re peers[MAXPEERS];
  uint8_t channel;
  bool bluetooth;
  bool wifi;
  web_dop web;
}esp_now_cnf_dev;

typedef struct{  
  uint8_t mac[6];
  web_dop web;
}web_mac_no_peer;

enum meas_sts{
  No_work,
  Meas_work,
  Meas_fail,
  Meas_succs
};

typedef struct{
  uint8_t mac_id;
  meas_sts sts;
}measure_work;


int get_id_to_mac(const uint8_t *mac_addr); //WEB
bool check_bisy(int id); //WEB
bool chek_timout_now(int id); //WEB
web_dop web_status_peer(int id); //WEB
void send_web_cmd(const uint8_t *mac_addr, camand_now cmd, web_cmd cmd_web, uint8_t len_data = 0);
void send_now(const uint8_t *mac_addr, uint8_t *data, uint8_t len);
void set_channel_peer(const uint8_t *mac_addr, uint8_t channel);
void set_channel_peers(void);
void add_connects_me_to_peer(const uint8_t *mac_addr); //WEB
void del_connects_me_to_peer(const uint8_t *mac_addr); //WEB
void del_connects_mac_to_peer(const uint8_t *mac_addr, const uint8_t *mac_del); //WEB
void clear_connects_to_peer(const uint8_t *mac_addr); //WEB
void get_connects_to_peer(const uint8_t *mac_addr); //WEB
void bluetooth_now_cnf(cnf_adapt on_off, const uint8_t *mac_addr); //WEB
void wifi_now_cnf(cnf_adapt on_off, const uint8_t *mac_addr); //WEB
void get_cnf_esp_now(const uint8_t *mac_addr); //WEB
void set_cnf_esp_now(const uint8_t *mac_addr); //WEB
void measure_esp_now(const uint8_t *mac_addr); //WEB
void reboot_esp_now(const uint8_t *mac_addr); //WEB


int get_id_to_mac(const uint8_t *mac_addr); //WEB
void reinit_esp_channel(void);
void init_esp_now(void);
bool add_peer_fors(const uint8_t *mac_addr);
void del_peir(const uint8_t *mac_addr);
void del_act_peir(const uint8_t *mac_addr);

extern mess_now *mess_send;
extern volatile bool reafl;
extern esp_now_peers_act act_peers[MAXPEERS];
extern esp_now_cnf_dev cnf_con_peers[MAXPEERS];

extern web_mac_no_peer web_peer_no;

#endif /* ESPNOW_H_ */