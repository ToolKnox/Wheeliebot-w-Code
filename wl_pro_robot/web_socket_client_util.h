#pragma once

#include "eeprom_util.h"
#include "wifi.h"
#include "robot.h"


enum WSS_STATE {
  WSS_STATE_OFF_LINE = 0,
  WSS_STATE_IDLE = 1,
  WSS_STATE_RECEIVE_OK = 2,
  WSS_STATE_RECEIVE_WAIT = 3,
  WSS_STATE_WAITING_PROCSSING = 4,
};

class MyWebSocketsClientData
{
public:
  uint16_t length;
  uint8_t  state;
  char* buffer;
};
extern MyWebSocketsClientData my_wss_data;

void web_sockets_client_init();

void web_sockets_client_loop();

void web_sockets_client_send_message(String value);

void wss_reset(void);

