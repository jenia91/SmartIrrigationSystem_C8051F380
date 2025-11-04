
![CI](https://github.com/jenia91/SmartIrrigationSystem_C8051F380/actions/workflows/ci.yml/badge.svg?branch=main)

# Smart Irrigation – C8051F380
<img width="1267" height="858" alt="PROTEUS" src="https://github.com/user-attachments/assets/83e4cf8b-73bf-4acf-9e26-734299f84751" />

Embedded irrigation controller on **C8051F380 (48 MHz)**.

## Features
- I²C (bit-bang): **LM75** temperature, **DS1307** RTC  
- ADC (10-bit): **Light=P2.0**, **Soil=P2.1**, **Rain=P2.2**  
- PCA-PWM (16-bit): **Servo on P0.0**, 600–2400 µs  
- Relay pump: **P0.2** (push-pull)  
- TFT + Touch via SPI, touch calib: `TouchSet(427, 3683, 3802, 438)`

## Pin map
- I²C: **SCL=P1.0 (push-pull)**, **SDA=P1.1 (open-drain + pull-up)**  
- ADC pins set to **analog (High-Z)** via `P2MDIN &= ~0x07`  
- SYSCLK = **48 MHz** (`OSCICN=0xC3`, `FLSCL=0x90`, `CLKSEL=0x03`)

## Build (compile-only)
bash
sdcc -mmcs51 -c *.c


## Quick links

* **Main code:** [`src/MainProject_Menu.c`](./src/MainProject_Menu.c)
* **MCU init / clocks / PCA / I²C / SPI:** [`src/init380.c`](./src/init380.c)
* **Headers:** [`src/include/`](./src/include/)
* **Vendor (local-only, git-ignored):** `vendor/` → `initsysSPI.h`, `LcdSpi20.LIB`

## Repo map

```
src/
 ├─ include/            # header files
 ├─ MainProject_Menu.c  # UI state machine + irrigation logic
 └─ init380.c           # system clock, PCA-PWM, I²C/SPI init
vendor/                 # proprietary deps (not committed)
.github/workflows/ci.yml
LICENSE, README.md, .gitignore
```

```
::contentReference[oaicite:0]{index=0}
```
