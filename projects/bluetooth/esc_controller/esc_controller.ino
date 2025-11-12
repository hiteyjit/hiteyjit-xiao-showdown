// MG24 ESC NODE (BLE CLIENT + MOTOR CONTROLLER)

//initializing libraries
#include <Arduino.h>
#include <Servo.h>
#include "sl_bt_api.h"


//setting constants
const uint8_t ESC_PIN = D1; //ESC pin connected to XIAO
const int ESC_NEUTRAL_US = 1500; //setting "stop" signal value for ESC. Also necessary to arm the ESC.
const int ESC_MIN_US = 1450; //setting minimum PWM value signal to send to ESC 
const int ESC_MAX_US = 1900; //setting maximum PWM value signal to send to ESC
const uint32_t ARM_DELAY_MS = 5000; //delay to allow the ESC to recognize the stopped signal & arm itself.


//Setting up BT-reciever constants
static const uuid_128 THROTTLE_SVC = { .data = {0xde,0xad,0xbe,0xef,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x80,0x5f,0x9b,0x34,0xfb} };
static const uuid_128 THROTTLE_CHR = { .data = {0xde,0xad,0xbe,0xef,0x00,0x00,0x10,0x01,0x80,0x00,0x00,0x80,0x5f,0x9b,0x34,0xfb} };

Servo esc;
volatile uint16_t incomingRaw = 0;
uint16_t chr_handle = 0;
uint8_t conn = 0xFF;
uint32_t lastPktMs = 0;


//setting up conversion of raw signal from BT to be mapped to PWM for ESC control.
static int rawToUs(uint16_t raw){
  int pwm = map((int)raw, 2850, 3300, 1500, 1800);
  if (raw < 2850) { 
    pwm = 1500; 
   
    if (raw < 2550 && raw > 0) {
      pwm = ESC_MIN_US; 
      
      }
    
    }

  return constrain(pwm, 1000, 2000);
}

//Bluetooth transmitter searching (transmitter is another MG24 board on the glove)
static void start_scan(){ sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_observation); }

//Bluetooth signal handling
extern "C" void sl_bt_on_event(sl_bt_msg_t *evt){
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id: start_scan(); break;
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      sl_bt_scanner_stop();
      sl_bt_connection_open(evt->data.evt_scanner_legacy_advertisement_report.address,
                            evt->data.evt_scanner_legacy_advertisement_report.address_type,
                            sl_bt_gap_phy_1m, NULL);
      break;
    case sl_bt_evt_connection_opened_id:
      conn = evt->data.evt_connection_opened.connection;
      sl_bt_gatt_discover_primary_services_by_uuid(conn, sizeof(THROTTLE_SVC), THROTTLE_SVC.data);
      break;
    case sl_bt_evt_gatt_service_id: {
      uint32_t svc = evt->data.evt_gatt_service.service;
      sl_bt_gatt_discover_characteristics_by_uuid(conn, svc, sizeof(THROTTLE_CHR), THROTTLE_CHR.data);
      break;
    }
    case sl_bt_evt_gatt_characteristic_id:
      chr_handle = evt->data.evt_gatt_characteristic.characteristic; break;
    case sl_bt_evt_gatt_procedure_completed_id:
      if (chr_handle) sl_bt_gatt_set_characteristic_notification(conn, chr_handle, sl_bt_gatt_notification);
      break;
    case sl_bt_evt_gatt_characteristic_value_id: {
      auto &d = evt->data.evt_gatt_characteristic_value;
      if (d.characteristic == chr_handle && d.value.len >= 2) {
        incomingRaw = (uint16_t)d.value.data[0] | ((uint16_t)d.value.data[1] << 8);
        lastPktMs = millis();
      }
      break;
    }
    case sl_bt_evt_connection_closed_id:
      chr_handle = 0; conn = 0xFF; start_scan(); break;
  }
}


//Actual motor control
void setup(){
  Serial.begin(115200);
  esc.attach(ESC_PIN);
  esc.writeMicroseconds(ESC_NEUTRAL_US);
  delay(ARM_DELAY_MS);
}

void loop(){
  const uint32_t now = millis();
  if (now - lastPktMs > 250) {
    esc.writeMicroseconds(ESC_NEUTRAL_US);
  } else {
    static int lastUs = ESC_NEUTRAL_US;
    int target = rawToUs(incomingRaw);
    int blended = lastUs + (target - lastUs)/3;
    esc.writeMicroseconds(blended);
    lastUs = blended;
  }
  delay(10);
}
