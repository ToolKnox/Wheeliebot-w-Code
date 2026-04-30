#include "wifi.h"
#include "robot.h"
#include "eeprom_util.h"
#include "ble.h"

bool wifi_restart_flag = false;

WIFI_STATE_TypeDef WIFI_STATE;


// Configure the parameters related to the AP (hotspot) mode
// const char *ssid = "navbot-en01-xxxxxx";
const char *ap_password = "12345678";

IPAddress AP_IP(192, 168, 1, 11);
IPAddress AP_GATEWAY(192, 168, 1, 11);
IPAddress AP_SUBNET(255, 255, 255, 0);

char wifi_mode;


// ESP-01S access the WiFi network in AP mode
void wifi_set_sta() {
  //Read the wifi information
  String wifi_ssid = rp.wifi_info_json[WIFI_INFO_KEY.SSID];
  String wifi_password = rp.wifi_info_json[WIFI_INFO_KEY.PASSWORD];

  String dev_name = rp.config_json[CONFIG_KEY.NAME];
  const char* sta_name = dev_name.c_str();

  WiFi.setHostname(sta_name);
  wifi_mode = WIFI_STA;
  WiFi.mode(WIFI_STA);
  char* ssid_arr = (char*)wifi_ssid.c_str();
  char* password_arr = (char*)wifi_password.c_str();

  WiFi.begin(ssid_arr, password_arr);

  String wifi_info = String("ssid:") + wifi_ssid + ",password:" + wifi_password;
  Serial.println("Connecting to Wifi: " + wifi_info);
  Serial.printf("connect %s...\r\n", wifi_ssid);
}

void wifi_set_ap(void) {
  char ap_name[20] = { 0 };
  rp.build_dev_name(ap_name);
  wifi_mode = WIFI_AP;
  WiFi.mode(WIFI_AP);
  Serial.printf("Reset Reason: %d%%\n", ESP.getFreeHeap()*100 / ESP.getHeapSize());
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
  Serial.printf("Reset Reason: %d%%\n", ESP.getFreeHeap()*100 / ESP.getHeapSize());
  WiFi.softAP(ap_name, ap_password);
  // Serial.println();
  // Serial.print("AP IP address = ");
  // Serial.println(WiFi.softAPIP());
}
bool is_wifi_state(String str)
{
  if(str == WIFI_STATE.CLIENT){
    return true;
  }else if(str == WIFI_STATE.SERVER){
    return true;
  }else if(str == WIFI_STATE.CLOSE){
    return true;
  }
  return false;
}
void get_wifi_info_json(void) {

  char state_arr[EepromParam.ADDR_WIFI_INFO_JSON.size];
  eeprom_util.read(&EepromParam.ADDR_WIFI_INFO_JSON, (char*)state_arr);
  String jsonStr = String(state_arr);
  deserializeJson(rp.wifi_info_json, jsonStr);

  if (is_wifi_state(rp.wifi_info_json[WIFI_INFO_KEY.STATE]) == false) {
    rp.wifi_info_json[WIFI_INFO_KEY.STATE] = WIFI_STATE.SERVER;
    rp.save_wifi_info_json();
  }
}
String get_wifi_state(void){
  String str = rp.wifi_info_json[WIFI_INFO_KEY.STATE];
  return str;
}

void wifi_init(void) {
  get_wifi_info_json();
  Serial.printf("wifi_state: %s\n", get_wifi_state());

  if (get_wifi_state() == WIFI_STATE.CLIENT) {
    Serial.println("start wifi client...");
    wifi_set_sta();  // wifi client
    rp.wifi_state = WIFI_CLIENT;
  } else if (get_wifi_state() == WIFI_STATE.SERVER) {
    Serial.println("start wifi server...");
    wifi_set_ap();  // wifi server
    rp.wifi_state = WIFI_SERVOR;
  } else {

  }
}
void wifi_restart(void)
{
  wifi_restart_flag = true;
}


bool wifi_status_now = false;
bool wifi_status_last = false;

bool wifi_connect_ok()
{
  return wifi_status_now;
}


void wifi_loop(void) {
  static char connect_count_down = 10;

  if(wifi_restart_flag == true)
  {
    wifi_status_now = false;
    wifi_status_last = true;
    WiFi.disconnect(true);  // true = close
  }

  if (wifi_mode != WIFI_STA) return;

  //If the wifi is not connected, it will actively connect every 10 seconds
  if (WiFi.status() != WL_CONNECTED) {
    connect_count_down--;
    if (connect_count_down == 0) {
      wifi_set_sta();
      connect_count_down = 10;
    }
    wifi_status_now = false;

  } else  //Print the IP information once when connecting via wifi
  {
    connect_count_down = 1;
    if (wifi_status_now == false) {
      // Print the IP address of ESP-01S
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
    }
    wifi_status_now = 1;
  }

  if(wifi_status_last != wifi_status_now){
    wifi_status_last = wifi_status_now;
    StaticJsonDocument<1024> doc;
    
    doc["type"]     = "NetWorkChanged";
    doc["Status"]   = wifi_status_now==true? "connected":"disconnected";
    doc["IP"]       = WiFi.localIP();
    doc["Wifi"]     = rp.wifi_info_json[WIFI_INFO_KEY.SSID];
    doc["Password"] = rp.wifi_info_json[WIFI_INFO_KEY.PASSWORD];
    ble_tx_add_json(doc);
  }
  if(wifi_restart_flag == true)
  {
    wifi_restart_flag = false;
    wifi_init();
  }
  
}
