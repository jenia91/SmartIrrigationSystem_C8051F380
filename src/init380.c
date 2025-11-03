//==================== init380.c ====================
// Project: Smart Irrigation System – Final Project
// Target:  C8051F380 Microcontroller
// Purpose:
// Initializes all hardware peripherals before entering main().
// -> Disables Watchdog Timer
// -> Sets up PCA for PWM control (Servo on P0.0)
// -> PCA operates at SYSCLK / 12 = 4 MHz -> 1 tick = 0.25 µs
// -> Configures Crossbar and SPI routing
// -> SPI can operate up to SYSCLK / 2 -> limited to ~12.5 MHz max (hardware limit)
// -> Initializes I²C lines (Open-Drain + Pull-Ups) for LM75 and DS1307
// -> I²C standard-mode supports up to 400 kHz = **400,000 bits per second**
// -> This is the bit-rate (not samples!) for serial communication on SDA/SCL
// -> Sets Relay control pin (P0.2) as Push-Pull output
// -> Prepares ADC input pins (P2.0–P2.2) for analog sensors
// -> ADC0 is a 10-bit SAR ADC, supports up to 500 kSPS (Samples Per Second)
// -> ADC = Analog-to-Digital Converter (used for Rain, Soil, Light sensors)
// -> Enables Internal Oscillator and Clock Multiplier (SYSCLK = 48 MHz)
#include "compiler_defs.h"
#include "C8051F380_defs.h"

void Init_Device(void)
{
    // 1) Disable Watchdog Timer and configure PCA clock source
    PCA0MD &= ~0x40;  // Clear WDTE bit -> disables Watchdog Timer (prevents unwanted resets)
    PCA0MD = 0x00;    // Sets PCA clock source to SYSCLK / 12 ->
                      // If SYSCLK = 48MHz -> PCA runs at 4MHz -> 1 tick = 0.25µs
       // 2) Configure PCA Module 0 for 16-bit PWM (Servo on P0.0)
    PCA0CN = 0x40;     // Enable PCA counter
                       // -> Starts internal 16-bit up-counter used for PWM generation (based on SYSCLK/12)
    PCA0CPM0 = 0xC2;   // Enable ECOM and PWM mode (16-bit)
    // -> ECOM = Enable Comparator (needed for match detection) PCA output is low until counter matches value in PCA0CPL0/PCA0CPH0
    // -> PWM = enable 16-bit  Pulse Width Modulation output on CEX0 (P0.0)
    // -> Used to generate accurate PWM pulses for servo control
	
    // 3) Crossbar Configuration
    XBR1 = 0x41;         // Enable Crossbar (bit 0 = 1) and route PCA Channel 0 (CEX0) to P0.0 (bit 6 = 0)
                         // -> Connects PWM output from PCA (used for servo) to the physical pin P0.0
    P0MDOUT |= 0x01;     // Set P0.0 as Push-Pull output (for strong HIGH and LOW levels)
                         // -> Required for generating a clean, sharp PWM signal for servo control
    // 4) I²C Configuration (for LM75 + DS1307 via P1.0 = SCL, P1.1 = SDA)
    // Set P1.0 (SCL) as Push-Pull output for fast clock edges (Master drives SCL directly)
    P1MDOUT |= 0x01;
    // Set P1.1 (SDA) as Open-Drain for proper bidirectional communication (I²C standard)
    P1MDOUT &= ~0x02;
    
    P1 |= 0x02;
		
    // 5) Relay Output Setup (Relay control via P0.2)
    P0MDOUT |= 0x04;         // Set P0.2 as Push-Pull output (strong 0/1 control)
    P0 &= ~0x04;             // Start with relay OFF (logic LOW = open circuit)

    // 6) ADC Inputs Setup (P2.0 = Light, P2.1 = Soil, P2.2 = Rain sensors)
    P2MDIN &= ~0x07;// Configure P2.0–P2.2 as analog inputs (Light, Soil, Rain)
                    // -> Clears digital input mode -> sets pins to High-Z (high impedance)
                    // -> Prevents internal circuitry from affecting voltage levels
                    // -> Ensures accurate analog voltage measurement by the ADC
    ADC0CN = 0x80;  // Enable ADC0 module (bit 7 = ADEN = 1)
                    // -> Activates the internal ADC for single-ended analog input conversion
    AMX0N = 0x1F;   // Set negative input (ADC-) to GND (single-ended mode)
                    // -> All ADC readings will be measured relative to ground (not differential)
    REF0CN = 0x08;  // Enable internal voltage reference (VREF = 1.65V ±2%) for ADC measurements
                    // -> Ensures consistent and accurate analog-to-digital conversions

    // 7) Clock Configuration – Set SYSCLK = 48MHz

    OSCICN = 0xC3;     // Enable internal oscillator in precision mode (24MHz)
                   // -> This oscillator serves as the input to the clock multiplier
    FLSCL = 0x90;      // Enable clock multiplier (bit 7 = 1) and configure flash timing
                   // -> Multiplies the 24MHz oscillator ×2 to generate 48MHz
    CLKSEL = 0x03;     // Select clock multiplier output as SYSCLK source (SYSCLK = 48MHz)
                   // -> Provides high-speed clock for system modules (PCA, SPI, etc.)
                   // -> Note: Not all peripherals run at SYSCLK directly — each uses its own divider
                   //    • PCA: SYSCLK / 12 -> 4MHz (0.25µs per tick)
                   //    • SPI: SYSCLK / [2 × (SPI0CKR + 1)] -> configurable
                   //    • I²C and ADC operate at their own controlled speeds

    // 8) SPI  Serial Peripheral Interface Configuration is performed in initSysSpi() (called later in main)
    // Example (not here): SPI0CKR = 2 -> SPI Clock = SYSCLK / [2 × (2 + 1)] = 8MHz
}
