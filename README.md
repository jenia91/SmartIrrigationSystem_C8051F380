![CI](https://github.com/jenia91/SmartIrrigationSystem_C8051F380/actions/workflows/ci.yml/badge.svg?branch=main)

# Smart Irrigation System – Embedded C (C8051F380)

<img width="900" alt="System Prototype" src="https://github.com/user-attachments/assets/83e4cf8b-73bf-4acf-9e26-734299f84751" />

---

## Overview
A complete embedded irrigation controller implemented on the C8051F380 microcontroller.  
The system measures environmental conditions in real-time and controls water flow automatically based on configurable thresholds and time windows.

---

## Features
- Real-time monitoring: **Soil**, **Rain**, **Light**, **Temperature**
- Automatic irrigation logic based on:
  - Soil moisture level
  - Rain presence
  - Light intensity
  - Temperature limit
  - Allowed time ranges
- **Servo-controlled sprinkler** and **relay-driven water pump**
- **TFT LCD UI** with touch-controlled menu (Check / Setup / Run)
- Communication Interfaces:
  - **I²C** → LM75 (temp), DS1307 (RTC)
  - **SPI** → ILI9341 (display), XPT2046 (touch)
- All code written in **Embedded C**, tested directly on hardware

---

## Build & Flash
- Developed and compiled in **Keil µVision** (C8051F380 toolchain)
- Verified in real-time using **logic analyzer** and **multimeter measurements**

---

## Pin Map
| Function | Pin(s) | Mode / Notes |
|---------|-------|--------------|
| I²C SCL | P1.0 | Push-pull |
| I²C SDA | P1.1 | Open-drain + pull-up |
| ADC Inputs | P2.0–P2.2 | `P2MDIN &= ~0x07` (High-Z analog) |
| Servo PWM | P0.0 | PCA-PWM (600–2400 µs) |
| Relay Pump | P0.2 | Push-pull output |
| System Clock | 48 MHz | `OSCICN=0xC3`, `FLSCL=0x90`, `CLKSEL=0x03` |
| Touch Calibration | – | `TouchSet(427, 3683, 3802, 438)` |

---

## Quick Links
- **Main project logic / UI** → [`src/MainProject_Menu.c`](./src/MainProject_Menu.c)
- **MCU clock / PCA-PWM / I²C / SPI init** → [`src/init380.c`](./src/init380.c)
- **Header files** → [`src/include/`](./src/include/)

---

## Repository Structure


## Repo map
```
src/
 ├─ include/            # header files
 ├─ MainProject_Menu.c  # UI state machine + irrigation logic
 └─ init380.c           # system clock, PCA-PWM, I²C/SPI init
.github/workflows/ci.yml
LICENSE, README.md, .gitignore
```


