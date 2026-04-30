#pragma once

#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "ble.h"
#include "robot.h"
#include "web_socket_client_util.h"


struct {
  int ALL = 0;
  int BLE = 1;
  int WEB_SOCKET_CLIENT = 2;
} FEEDBACK_CHANNEL;


void feedback_util_send_message(int send_channel);
