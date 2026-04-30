#pragma once

#include "robot.h"

#define EEPROM_SIZE 2048


typedef struct EepromObject {
  char name[30];
  int index;
  int size;
} EepromObject;

struct {
  //  robot info
  EepromObject ADDR_INFO = { "addr_info", 0, 10 };

  // wifi info
  EepromObject ADDR_WIFI_INFO_JSON{"addr_wifi_info_json", 10, 110};
  // EepromObject ADDR_WIFI_SSID = { "addr_wifi_ssid", 10, 50 };
  // EepromObject ADDR_WIFI_PASSWORD = { "addr_wifi_password", 60, 30 };
  // EepromObject ADDR_WIFI_STATE = { "addr_wifi_state", 90, 30 };

  // web socket info
  EepromObject ADDR_WEB_SOCKET_HOST = { "addr_web_socket_host", 120, 20 };
  EepromObject ADDR_WEB_SOCKET_PORT = { "addr_web_socket_port", 140, 10 };
  EepromObject ADDR_WEB_SOCKET_PATH = { "addr_web_socket_path", 150, 50 };

  EepromObject ADDR_JSON = { "addr_json", 200, 1024 };
} EepromParam;

class EepromUtil {
public:
  String read(EepromObject *param);

  size_t read(EepromObject *param, char *value);

  uint16_t readToUint16T(EepromObject *param);

  size_t write(EepromObject *param, String value);

  size_t writeUint16T(EepromObject *param, uint16_t value);

  void init(void);
};

extern EepromUtil eeprom_util;
