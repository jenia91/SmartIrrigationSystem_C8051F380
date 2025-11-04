![CI](https://github.com/jenia91/SmartIrrigationSystem_C8051F380/actions/workflows/ci.yml/badge.svg?branch=main)
# Smart Irrigation – C8051F380

<img width="1267" height="858" alt="PROTEUS" src="https://github.com/user-attachments/assets/8a1a4430-9546-4d39-900c-5c27c7462f74" />



Embedded irrigation controller on **C8051F380 (48 MHz)**.

**Features**
- I²C (bit-bang): **LM75** temperature, **DS1307** RTC  
- ADC (10-bit): **Light=P2.0**, **Soil=P2.1**, **Rain=P2.2**  
- PCA-PWM (16-bit): **Servo on P0.0**, 600–2400 µs  
- Relay pump: **P0.2** (push-pull)  
- TFT+Touch via SPI , touch calib: `TouchSet(427, 3683, 3802, 438)`

**Pin map**
- I²C: **SCL=P1.0 (push-pull)**, **SDA=P1.1 (open-drain + pull-up)**  
- ADC pins set to **analog (High-Z)** via `P2MDIN &= ~0x07`  
- SYSCLK = **48 MHz** (`OSCICN=0xC3`, `FLSCL=0x90`, `CLKSEL=0x03`)

**Build (compile-only)**
```bash
sdcc -mmcs51 -c *.c
