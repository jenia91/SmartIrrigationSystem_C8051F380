![CI](https://github.com/jenia91/SmartIrrigationSystem_C8051F380/actions/workflows/ci.yml/badge.svg?branch=main)

# Smart Irrigation System – Embedded C (C8051F380)
<img width="1267" height="858" alt="PROTEUS" src="https://github.com/user-attachments/assets/83e4cf8b-73bf-4acf-9e26-734299f84751" />

Features:
• Real-time environmental monitoring (Soil, Rain, Light, Temperature)
• Automatic irrigation logic based on thresholds and time windows
• Servo-controlled sprinkler + Relay-based pump control
• TFT LCD UI with touch menu (Check, Setup, Run)
• I²C: LM75 + DS1307   |  SPI: ILI9341 + XPT2046   |  ADC
All code written in embedded C, tested on hardware.

Build & Flash:
• Developed and compiled in Keil µVision (C8051F380 toolchain)
• Tested on hardware in real-time using logic analyzer & multimeter verification

## Pin map
- I²C: **SCL=P1.0 (push-pull)**, **SDA=P1.1 (open-drain + pull-up)**  
- ADC pins set to **analog (High-Z)** via `P2MDIN &= ~0x07`
- PCA-PWM (16-bit): **Servo on P0.0**, 600–2400 µs
- Relay pump: **P0.2** (push-pull)  
- SYSCLK = **48 MHz** (`OSCICN=0xC3`, `FLSCL=0x90`, `CLKSEL=0x03`)
- TFT + Touch via SPI, touch calib: `TouchSet(427, 3683, 3802, 438)`

## Quick links
* **Main code:** [`src/MainProject_Menu.c`](./src/MainProject_Menu.c)
* **MCU init / clocks / PCA / I²C / SPI:** [`src/init380.c`](./src/init380.c)
* **Headers:** [`src/include/`](./src/include/)

## Repo map
```
src/
 ├─ include/            # header files
 ├─ MainProject_Menu.c  # UI state machine + irrigation logic
 └─ init380.c           # system clock, PCA-PWM, I²C/SPI init
.github/workflows/ci.yml
LICENSE, README.md, .gitignore
```


