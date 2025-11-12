# Silicon Labs XIAO Showdown Submission

**Welcome!** For the XIAO Showdown, I have created a glove-controlled electric skateboard!

The project in its final form consists of three Seeed Studio **XIAO MG24** boards communicating over I²C and BLE to form a complete wireless control system for an electric skateboard.

- **Sensor Node (MG24 #1)** – Reads flex/stretch sensors embedded in a wearable glove and acts as an **I²C slave** that streams a processed throttle value.  
- **Relay Node (MG24 #2)** – Acts as an **I²C master** that polls the sensor board and a **BLE server** that advertises the throttle value to remote clients.  
- **ESC Node (MG24 #3)** – Functions as a **BLE client**, subscribing to throttle notifications and translating them into PWM signals that control the skateboard’s **electronic speed controller (ESC)**.  

This three-tiered design mirrors the original ESP32-based glove project while replacing Wi-Fi / ESP-NOW with the **low-power Bluetooth Low Energy (BLE)** stack of the XIAO MG24, providing lower latency and power consumption optimized for wearables.

The system was first developed and tested using **ESP32 boards** with ESP-NOW communication. After validation, it was fully ported to the XIAO MG24 platform—maintaining identical pin assignments and sensor interfaces but switching to BLE notifications for wireless data transmission. Part of the reason development occured in this way was since it took time for the Xiao MG24 boards to be delivered to me, and I wanted to begin testing and iterating as soon as possible.

---

## Arduino Sketches

| File | Description |
|------|--------------|
| `src/glove_sensor_node.ino` | Runs on **MG24 #1 (Sensor)**. Reads analog stretch sensors, averages signals, and serves data over I²C (address `0x55`). |
| `src/glove_relay_node.ino` | Runs on **MG24 #2 (Relay)**. Polls the I²C sensor node and broadcasts the throttle value over BLE as a custom GATT characteristic. |
| `src/skateboard_receiver_ESC.ino` | Runs on **MG24 #3 (Receiver)**. Connects to the BLE server, receives throttle updates, and converts them to PWM signals for the ESC. |

All sketches are written for the **Seeed XIAO MG24 Arduino core** using the **Silicon Labs BLE stack**  
(`Tools → Protocol Stack → BLE (Silabs)`).

---

## Hardware Overview

- **Microcontrollers:** 3 × Seeed Studio XIAO MG24 Sense boards  
- **Sensors:** 3–5 flex/stretch sensors on glove fingers (analog input 0–3.3 V)  
- **Interconnects:** I²C (SDA = D4, SCL = D5) between Sensor and Relay nodes; BLE 5.3 between Relay and ESC nodes  
- **ESC Control:** PWM signal on D1 (1–2 ms range) with common GND  
- **Power:** Li-Po battery 3.7 V → 5 V boost or regulated 3.3 V rail  

All three boards share the same **XIAO footprint**, allowing rapid swaps between MG24, ESP32, or nRF52 variants if alternate wireless protocols (e.g., Wi-Fi, ESP-NOW, Thread, Zigbee) are desired.

---

## Demonstration Video

🎥 [**Watch the demo**](https://youtu.be/oGmYVVamqsc)  
The video showcases the full system—the flex-sensor glove, the wireless communication chain, and the electric skateboard responding in real time to hand gestures. Hope you enjoy!

---


---

## Dependencies

Install the following Arduino libraries:

- **Servo** (by Michael Margolis et al.) – for ESC PWM control  
- **Wire** (standard) – for I²C communication  
- **Seeed XIAO MG24 Arduino Core** (latest) – includes `sl_bt_api.h` and Silicon Labs BLE stack  

---

## Notes

- BLE characteristic updates are sent at ~20 Hz (50 ms interval).  
- The ESC node includes a **failsafe timeout** (250 ms) returning the motor to neutral if no packets are received.  
- The modular 3-board setup allows for future integration with additional wearables (e.g., IMU, EMG sensors) or multi-axis motor control.  

---

**Author:** Hiteyjit Singh Gujral
**License:** MSLA (see `LICENSE` file for details)


