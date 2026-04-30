#include "eeprom_util.h"
#include "EEPROM.h"
#include "robot.h"

EepromUtil eeprom_util;

bool init_completed = false;


void EepromUtil::init(void) {
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.printf("ERROR! EEPROM INIT FALSE \r\n");
  }
}

String EepromUtil::read(EepromObject *param) {
  if (!param) return String();
  return EEPROM.readString(param->index);
}

size_t EepromUtil::read(EepromObject *param, char *value) {
  if (!param || !value || param->size < 1) return 0;
  return EEPROM.readString(param->index, value, param->size);
}

uint16_t EepromUtil::readToUint16T(EepromObject *param) {
  if (!param) return 0;
  return EEPROM.readUShort(param->index);
}

size_t EepromUtil::write(EepromObject *param, String value) {
  if (!param) return 0;
  size_t result = EEPROM.writeString(param->index, value);
  EEPROM.commit();  // save
  return result;
}

size_t EepromUtil::writeUint16T(EepromObject *param, uint16_t value) {
  if (!param) return 0;
  size_t result = EEPROM.writeUShort(param->index, value);
  EEPROM.commit();  // save
  return result;
}
