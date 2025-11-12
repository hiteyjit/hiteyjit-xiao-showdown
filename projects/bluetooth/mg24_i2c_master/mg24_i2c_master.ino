//MG24 RELAY NODE (I2C MASTER + BLE SERVER)

//Setting up libraries
#include <Arduino.h>
#include <Wire.h>
#include "sl_bt_api.h"

//Defining I2C pins that will recieve information from another MG24 wired to finer sensors
#define I2C_ADDR 0x55
#define SDA_PIN  D4   // SDA0
#define SCL_PIN  D5   // SCL0

//Setting up BLE UUIDs (same as receiver connected to ESC)
static const uuid_128 THROTTLE_SVC = { .data = {0xde,0xad,0xbe,0xef,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x80,0x5f,0x9b,0x34,0xfb} };
static const uuid_128 THROTTLE_CHR = { .data = {0xde,0xad,0xbe,0xef,0x00,0x00,0x10,0x01,0x80,0x00,0x00,0x80,0x5f,0x9b,0x34,0xfb} };

static uint16_t gdb, svc, chr;
static uint32_t tPrev = 0;

//Recieving I2C transmissions
uint16_t readSlave16() {
  uint16_t v = 0;
  Wire.requestFrom(I2C_ADDR, 2, true);
  if (Wire.available() >= 2) {
    uint8_t lo = Wire.read();
    uint8_t hi = Wire.read();
    v = (uint16_t)lo | ((uint16_t)hi << 8);
  }
  return v;
}

//BT Signal handling
extern "C" void sl_bt_on_event(sl_bt_msg_t *evt) {
  if (SL_BT_MSG_ID(evt->header) == sl_bt_evt_system_boot_id) {
    // Build GATT: service + char(notify)
    sl_bt_gattdb_new_session(&gdb);
    sl_bt_gattdb_add_service(gdb, sl_bt_gattdb_primary_service, SL_BT_GATTDB_ADVERTISED_SERVICE, sizeof(THROTTLE_SVC), THROTTLE_SVC.data, &svc);
    sl_bt_gattdb_add_uuid128_characteristic(gdb, svc, SL_BT_GATTDB_CHARACTERISTIC_READ | SL_BT_GATTDB_CHARACTERISTIC_NOTIFY, 0, 0, THROTTLE_CHR, sl_bt_gattdb_variable_length_value, 2, 2, nullptr, &chr);
    uint16_t ccc; const sl_bt_uuid_16_t ccc_uuid = { .data={0x02,0x29} };
    sl_bt_gattdb_add_uuid16_descriptor(gdb, chr, 0, 0, ccc_uuid, sl_bt_gattdb_fixed_length_value, 2, 2, (const uint8_t*)"\x00\x00", &ccc);
    uint16_t id; sl_bt_gattdb_commit(gdb, &id);

    sl_bt_advertiser_create_set(nullptr);
    sl_bt_legacy_advertiser_start(0, sl_bt_advertiser_connectable_scannable);
    Serial.println("Relay advertising (BLE Server)...");
  }
}

//Extracting I2C Data + relaying
void setup() {
  Serial.begin(115200);
  Wire.begin();               // MASTER mode
  //Wire.setPins(SDA_PIN, SCL_PIN);

  // (Optionally) set clock: Wire.setClock(400000);
}

void loop() {
  if (millis() - tPrev >= 50) {
    tPrev = millis();
    uint16_t raw = readSlave16();
    uint8_t p[2] = { (uint8_t)(raw & 0xFF), (uint8_t)(raw >> 8) };
    sl_bt_gatt_server_notify_all(chr, 2, p);
  }
}
