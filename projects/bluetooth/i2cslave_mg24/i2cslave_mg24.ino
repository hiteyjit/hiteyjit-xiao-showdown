//XIAO MG24 SENSOR NODE (I2C SLAVE)

//Setting up libraries
#include <Arduino.h>
#include <Wire.h>

//Setting up I2C + sensor pins
#define I2C_ADDR   0x55
#define SENSOR_PIN1 D0 // Thumb
#define SENSOR_PIN2 D1 // Index
#define SENSOR_PIN3 D2 // Middle
#define SENSOR_PIN4 D3 // Ring
#define SENSOR_PIN5 D6 // Pinky


volatile uint16_t lastValue = 0;


//Computing join-bending average from each sensor for one unified signal
static inline uint16_t readAverage() {
  uint32_t s = analogRead(SENSOR_PIN1);
  s += analogRead(SENSOR_PIN2);
  s += analogRead(SENSOR_PIN3);
  s += analogRead(SENSOR_PIN4);
  s += analogRead(SENSOR_PIN5);
  return (uint16_t)(s / 5); // MG24 ~12-bit ADC (0..4095)
}


//Handling I2C request from Master MG24 board
void onRequest() {
  lastValue = readAverage();
  uint8_t p[2] = { (uint8_t)(lastValue & 0xFF), (uint8_t)(lastValue >> 8) };
  Wire.write(p, 2);

  Serial.println(((uint8_t *)&sensorValue)[0], BIN);
  Serial.println(((uint8_t *)&sensorValue)[1], BIN);
  Serial.println(sensorValue,BIN);
  // Wire.write(sensorValue);
  Serial.printf("onRequest -> Sent sensor value: %d\n", sensorValue);
}

void onReceive(int len) {
  // handle commands from master
  while (Wire.available()) (void)Wire.read();
}


void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN1, INPUT);
  pinMode(SENSOR_PIN2, INPUT);
  pinMode(SENSOR_PIN3, INPUT);

  Wire.onRequest(onRequest);
  Wire.onReceive(onReceive);
  Wire.begin((uint8_t)I2C_ADDR);   // SLAVE mode
}

void loop() {
  // nothing; responds on request
}
