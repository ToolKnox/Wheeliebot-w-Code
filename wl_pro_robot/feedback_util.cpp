#include "feedback_util.h"
#include "robot.h"


static String get_dev_mac() {
  uint8_t mac[7];
  esp_read_mac(mac, ESP_MAC_BT);
  mac[6] = 0;  // Ensure null-termination

  char mac_str[13];  // 12 chars (6 bytes * 2) + null terminator
  for (int i = 0; i < 6; i++) {
    // Convert each byte to two uppercase hex characters
    snprintf(&mac_str[i * 2], 3, "%02X", mac[i]);
  }
  mac_str[12] = '\0';  // Explicit null termination

  return String(mac_str);
}
String get_wifi_ip(void)
{
  if(rp.wifi_state == WIFI_CLIENT)
  {
    IPAddress ip = WiFi.localIP();
    String ipString = ip.toString();
    return ipString;
  }else if(rp.wifi_state == WIFI_SERVOR){
    return "192.168.1.11";
  }
  
  return "null";
}
String get_device_info() {
  // Serialize to string
  String jsonStr;
  // Create static JSON document (allocate memory pool size)
  StaticJsonDocument<1024> doc;

  // Read voltage
  double battery_voltage = rp.get_battery_voltage();
  // Calculate battery level
  double battery_percentage = rp.get_battery_level();
  // Read internal temperature
  double fahrenheit = rp.get_fahrenheit();
  // Convert to Celsius (unit: Fahrenheit)
  double degree_centigrade = rp.get_degree_centigrade();

  // Format to retain two decimal places (rounded to 2 decimal places)
  battery_voltage = round(battery_voltage * 100) / 100;
  battery_percentage = round(battery_percentage) / 1;
  degree_centigrade = round(degree_centigrade * 100) / 100;

  // Add simple values
  doc["type"] = "get_device_info";
  doc["pcb_version"] = rp.pcb_version;
  doc["battery_level"] = rp.battery_level;
  doc["battery_voltage"] = rp.battery_voltage;
  doc["centigrade"] = rp.centigrade;
  doc["name"] = rp.config_json[CONFIG_KEY.NAME];
  doc["charge"] = rp.charge;
  doc["IP"] = get_wifi_ip();
  // rp.get_expression_name(doc);
  doc["cloud_token"] = rp.config_json[CONFIG_KEY.CLOUD_TOKEN];
  doc["openAI_token"] = rp.config_json[CONFIG_KEY.OPENAI_TOKEN];
  // doc["status"] = rp.status;

  // doc["ESP32"] = "2.0.3";
  // doc["main"] = "1.0.0.0";
  // doc["GPS"] = "--";
  doc["mac"] = get_dev_mac();

  serializeJson(doc, jsonStr);
  return jsonStr;
}

// Send all information
void feedback_util_send_message(int send_channel = FEEDBACK_CHANNEL.BLE) {
  // Get device information and send
  String device_info = get_device_info();

  // Serial.print("Send Device Info:");
  // Serial.println(device_info);

  // Send data according to the specified channel
  Serial.print("device info:");
  Serial.println(device_info);
  if (FEEDBACK_CHANNEL.ALL == send_channel) {
    web_sockets_client_send_message(device_info);
    ble_tx_add_string(device_info);
  } else if (FEEDBACK_CHANNEL.BLE == send_channel) {
    ble_tx_add_string(device_info);
  } else if (FEEDBACK_CHANNEL.WEB_SOCKET_CLIENT == send_channel) {
    web_sockets_client_send_message(device_info);
  }
}
