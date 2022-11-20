#ifndef CONFIG_H_
#define CONFIG_H_
#include <Arduino.h>
#include "FS.h"
//#include "FFat.h"
//#include "LittleFS.h"
#include <Arduino_JSON.h>

#define FILESYSTEM SPIFFS

#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif

#define MAXPEERS 20

enum role_esp {
    device = 0,
    server_i
};

typedef struct 
{
  bool route;
  bool softap;
  char mdns_name[35];
  char name[35];
  char pass[21];
  char pass_esp[21];
  char login[21];
  char pass_autch[21];
}wifi_lan_cnf;

typedef struct
{
  bool act;
  uint8_t channel;
  bool cnf;
  char name[31];
  char lastname[31];
  uint8_t mac_addr[6];
  uint8_t mac_ap[6];
}esp_now_peers;


typedef struct
{
  uint8_t max_peer;
  uint8_t channel;
  role_esp role;
  esp_now_peers peers[MAXPEERS];
}esp_now_cnf;

typedef struct
{
  char ntpServer1[31];
  char ntpServer2[31];
  char ntpServer3[31];
  uint16_t pereod_alarm;
}time_cnf;

#define MAXSERVERS 3

typedef struct{
  bool act;
  char host[31];
  uint16_t port;
}ser_cnf;

typedef struct{
  uint16_t max_serv;
  ser_cnf serv[MAXSERVERS];
}servers_cnf;


#define MAXFINGER 128

enum fingers{
  thumb = 0, //Большой палец
  index_finger, //Указательный палец
  middle_finger, //Средний палец
  ring_finger, //Безимянный палец
  pinky, //Мизинец
};

typedef struct{
  bool act;
  uint8_t id;
  char name[31];
  fingers phalanx;
}fingers_cnf;

enum conf_type{
  lan_cnf = 0,
  now_cnf,
  times_cnf,
  server_cnf,
  finger_cnf,
  ver_cnf
};




#define ROLEESP device
#define LANFILE "/lan.json"
#define NOWFILE "/now.json"
#define TIMEFILE "/time.json"
#define SERVERFILE "/servers.json"
#define FINGERFILE "/finger.json"
#define VERFILE "/ver.json"
#define DEFCHANNEL 6

// extern wifi_lan_cnf conf_lan[2];
// extern esp_now_cnf conf_esp_now[2];
// extern time_cnf conf_time[2];
// extern servers_cnf conf_server[2];
// extern station_cnf conf_station[2];

bool exists(String path);
void char_to_mac(const char *str, uint8_t *mac);
void defconfig(void);
bool saveconfig(conf_type type);
bool loadConfig(conf_type type);
void initconfig(void);

extern float ver_s;
extern const float ver;

#endif /* CONFIG_H_ */