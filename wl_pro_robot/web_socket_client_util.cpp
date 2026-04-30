#include "web_socket_client_util.h"
#include <WebSocketsClient.h>
#include "robot.h"

bool wss_reset_flag = false;
bool wss_delay_reset_flag = false;

MyWebSocketsClientData my_wss_data;

WebSocketsClient webSocketClient = WebSocketsClient();

void eventCallback(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      {
        printf("WEB SOCKET CLIENT:DISCONNECTED !!\n");
        rp.socket_connected = true;
        break;
      }
    case WStype_CONNECTED:
      {
        printf("WEB SOCKET CLIENT:CONNECTED !!\n");
        rp.socket_connected = false;
      }

      break;
  }

  if (type == WStype_TEXT) {

    my_wss_data.length = length;

    my_wss_data.buffer = (char*)malloc(length+1);
    memcpy(my_wss_data.buffer,payload,length);
    *(my_wss_data.buffer+length) = 0;

    my_wss_data.state = WSS_STATE_WAITING_PROCSSING;


    return;

  }
}
void web_sockets_client_init(void){
  wss_reset_flag = true;
}
void _web_sockets_client_init(void) {
  return;
  if (get_wifi_state() == WIFI_STATE.CLIENT) {
    String web_socket_client_host = "hub.navbot.com";       //eeprom_util.read(&EepromParam.ADDR_WEB_SOCKET_HOST);
    uint16_t web_socket_client_port = 443;                    //eeprom_util.readToUint16T(&EepromParam.ADDR_WEB_SOCKET_PORT);
    String web_socket_client_path = rp.config_json[CONFIG_KEY.CLOUD_TOKEN];
    
    if (web_socket_client_host.length() > 0 && web_socket_client_port > 0 && web_socket_client_path.length() > 0) {
      web_socket_client_path = "/ws/" + web_socket_client_path;
      String connectionString = "wss://" + web_socket_client_host + ":" + String(web_socket_client_port) + web_socket_client_path;

      Serial.println("WebSocket Connecting Address: " + connectionString);

      if (443 == web_socket_client_port) {
        webSocketClient.beginSSL(web_socket_client_host, web_socket_client_port, web_socket_client_path);
      } else {
        webSocketClient.begin(web_socket_client_host, web_socket_client_port, web_socket_client_path);
      }

      // Set event callback handler function
      webSocketClient.onEvent(eventCallback);

      // Optional: Set the reconnection interval (in milliseconds)
      webSocketClient.setReconnectInterval(10000);

      // Ping every 15 seconds, with a 3-second timeout and 2 failed attempts resulting in disconnection.
      webSocketClient.enableHeartbeat(15000, 3000, 2);
    } else {
      String param = "{web_socket_client_host:" + web_socket_client_host + ",web_socket_client_port:" + web_socket_client_port + ",web_socket_client_path:" + web_socket_client_path + "}";
      Serial.println("Connecting to WebSocket: ERROR->Parameter verification failed->" + param);
    }
  }
}
void web_sockets_client_processing(void)
{
  if(my_wss_data.state == WSS_STATE_WAITING_PROCSSING)
  {
    my_wss_data.state = WSS_STATE_IDLE;
    delay(0);
    String payload_str = String((char *)my_wss_data.buffer);
    Serial.print("Web Socket Client Receive A Message:");
    Serial.println(payload_str);

    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, payload_str);

    printf("eFuse Two Point: Supported\n");
    String mode_str = doc["mode"];
    rp.parseJson(doc);
    free(my_wss_data.buffer);
  }
}

void wss_delay_reset(void)
{
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  if(currentMillis - lastMillis > 1000){
    lastMillis =  currentMillis;
  }else{
    return;
  }
  /*
    Run once every 1 second, and initialize after 3 seconds.
  */
  static char count_s = 0;

  if(count_s++ > 3){
    count_s = 0;
    wss_delay_reset_flag = false;
    _web_sockets_client_init();
    Serial.println("Web Socket Reset");
  }
}
void web_sockets_client_loop(void) {


  /*
  If there is no Wi-Fi connection, the socket will not be established.
  */
  if (WiFi.status() != WL_CONNECTED) return;



  web_sockets_client_processing();
  webSocketClient.loop();
  if(wss_reset_flag == true)
  {
    wss_reset_flag = false;
    webSocketClient.disconnect();
    Serial.println("Web socket close, and initialize after 3 seconds");
    wss_delay_reset_flag = true;
  }
  if(wss_delay_reset_flag == true)
  {
    wss_delay_reset();
  }
}


void web_sockets_client_send_message(String value) {
  if (rp.socket_connected == true) {
    webSocketClient.sendTXT(value);
  }
}


void wss_reset(void)
{
  wss_reset_flag = true;
}







