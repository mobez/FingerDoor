#ifndef WEB_SERVER_H_
#define WEB_SERVER_H_
#include <Arduino.h>
#include "WebServer.h"

enum status_figner{
  figner_none = 0,
  figner_start,
  figner_err,
  figner_ok
};

void init_web(void);
extern WebServer server;

#endif /* WEB_SERVER_H_ */