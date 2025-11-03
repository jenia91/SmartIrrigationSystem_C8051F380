// ========================= main.c =========================
// Project: Smart Irrigation System – Final Project
// Target:  Silicon Labs C8051F380 Microcontroller
// Clock:   Internal Oscillator at 24 MHz × 2 = 48 MHz SYSCLK
// Overview:
// Main control program for an automatic irrigation system, handling sensor readings,
// real-time clock management, user interaction via touchscreen, a 5V servo control (PWM),
// and relay activation for a 12V water pump.
// [1] Global Threshold Constants:
//     - Define critical sensor thresholds (soil, rain, light)
// [2] Global System Variables:
//     - Store real-time sensor values, RTC time, servo angle, and operational flags
// [3] Function Prototypes:
//     - Declare UI screens and core logic functions
// [4] Hardware Initialization (main()):
//     - Initialize PCA (PWM), ADC channels, I²C (RTC, temp sensor), SPI (LCD, touch)
// [5] Main Loop Operations:
//     (A) Read sensors (ADC and I²C)
//     (B) Run irrigation logic (if active)
//     (C) Detect touchscreen input and button presses
//     (D) Screen navigation based on user input:
//         • Screen 0 (Main): Displays navigation buttons ("Check", "Setup", "Project")
//         • Screen 1 (Check): Shows real-time sensor data (soil, rain, light, temperature, RTC)
//           - Additional buttons for displaying specific sensor and time values on demand
//         • Screen 2 (Setup): Allows RTC time adjustments and setting the temperature threshold
//           - Buttons to increment/decrement hours, minutes, and temperature threshold
//         • Screen 3 (Project): Activates full irrigation logic, shows all sensor data in real-time
// [6] Screen Drawing Functions:
//     - Create touchscreen buttons and display static UI elements for each screen
// [7] runProject() Logic:
//     - Checks combined conditions:
//         • Soil dryness, no significant rain, low ambient light, temperature below threshold
//         • Allowed irrigation time windows (04:00–08:00 or 19:00–22:00)
//     - If conditions met: activates relay (pump) and gradually sweeps servo position
//     - Displays real-time sensor values during operation
#include "compiler_defs.h"           // Compiler-specific definitions (macros, types, bit-fields)
#include "C8051F380_defs.h"          // Special Function Register (SFR) definitions for C8051F380
#include "initsysSPI.h"              // SPI-based LCD, delay utilities, and touchscreen calibration functions
#include "my_private_header.h"       // User-defined header: low-level declarations (ADC, servo, I²C, etc.)
// --------------------------------------------------------------------
// System Threshold Definitions (calibrated values for decision logic)
// --------------------------------------------------------------------
#define SOIL_THRESHOLD    40   // If soil moisture = 40%, soil is dry -> irrigation may be needed
#define RAIN_THRESHOLD    80   // If rain sensor reading = 80%, no significant rain -> safe to irrigate
#define LIGHT_THRESHOLD   70   // If ambient light < 70%, lighting conditions are suitable for irrigation
// TEMP_THRESHOLD is defined as a variable below (adjustable in runtime)

// --------------------- Global Sensor and System Variables ---------------------

float temp;                     // Temperature reading from LM75 (°C), received via I²C
int TEMP_THRESHOLD = 27;        // Dynamic temperature threshold (°C) for irrigation (default = 27)
int directionUp;                // Servo sweep direction flag: 1 = increasing angle, 0 = decreasing
unsigned int angle = 1500;      // PWM pulse width in µs for servo position (1500µs = center = 90°)
int hour, minute, second;       // RTC time values (hours, minutes, seconds) from DS1307
int rain, soil, light;          // Sensor readings converted to percentages (0–100%)
                                // rain  -> rain sensor (ADC)
                                // soil  -> soil moisture sensor (ADC)
                                // light -> light intensity sensor (ADC)
bit runFlag = 0;                // System state flag (1-bit): 
                                // 0 = idle mode (do nothing), 
                                // 1 = active mode (runProject logic executes)
// ---------------- Function Prototypes ----------------
// Screen drawing functions (for user interface)  
void screen0(void);  // Startup screen  
void screen1(void);  // "Check" screen: displays sensor data and time  
void screen2(void);  // "Setup" screen: allows RTC adjustments  and TEMP_THRESHOLD
void screen3(void);  // "Project" screen: real-time operation  

// Main project logic function – executed in PROJECT mode  
void runProject(void);  
// ---------------- Main Function ----------------  
void main(void)  
{  
    S16 x = 0, y = 0;             // Variables for touchscreen X and Y coordinates  
    S16 ButtonNum = 0;            // Variable for detected button number from touch input  
    U8 screen = 0;                // Current screen indicator: 0 = startup, 1 = Check, 2 = Setup, 3 = Project  

    // ---------- Hardware Initialization ----------  
    Init_Device();                // Initialize hardware: PCA for PWM, ADC channels, I2C pins, oscillator, Enable crossbar + route CEX0 (PWM) to P0.0.  
    initSysSpi();                 // Initialize LCD, delays and touch functions  
	 
    TouchSet(427, 3683, 3802, 438);  // Calibrate the touchscreen with raw min/max X/Y values
	  // Touch calibration (RAW ADC ranges -> pixel map, 240x320 portrait)
    // TouchSet expects: (Xmin, Xmax, Ymax, Ymin)
    LCD_fillScreen(BLACK);           // Clear the entire LCD screen by filling it with black color
    // Display the startup screen (screen0)  
    screen0();  

    // ---------- MAIN LOOP ----------  
    while(1)  
    {  
        delay_ms(20);  // Base loop delay (20 ms)  

        // --- (A) Read Sensors Values ---
        // Read temperature from LM75 (I²C address = 0x48)
        // I²C format requires 7-bit address + 1-bit R/W flag:
        // (0x48 << 1) = 0x90 -> shifts address left to make room for R/W bit
        // R/W bit = 1 (Read mode), so full byte sent = 0x91
        temp = readTemp((0x48 << 1) | 1);  // Reads and converts 9-bit digital value to float (°C)
       	// 48 = binnary 1001000
			  // if master write =1 (0x48 << 1) | 0 -> 10010000 = 0x90
			  // if master read = 0  (0x48 << 1) | 1 -> 10010001 = 0x91
			
			  // Read current time from DS1307 RTC (values are in BCD format)
        hour   = readDS1307(0x02);   // Read hour register (address 0x02)
        minute = readDS1307(0x01);   // Read minute register (address 0x01)
        second = readDS1307(0x00);   // Read second register (address 0x00), CH (Clock Halt) bit masked
  
        // Read ADC sensor values from the defined channels and convert to percentage (0..100)  
        light = (ADC_IN_CHANNEL(0x00) * 10) / 102; // Light sensor reading from ADC channel 0 (P2.0) scaled to percentage  
        soil  = (ADC_IN_CHANNEL(0x01) * 10) / 102; // Soil sensor reading from ADC channel 1 (P2.1) scaled to percentage  
        rain  = (ADC_IN_CHANNEL(0x02) * 10) / 102; // Rain sensor reading from ADC channel 2 (P2.2) scaled to percentage  

        // --- (B) Execute PROJECT Mode Logic if Active ---  
        if(runFlag)                    // Check if irrigation flag is active (runFlag == 1)
        runProject();              // Execute irrigation logic (runProject) only when flag is set
        // --- (C) Read Touchscreen Input ---  
        x = ReadTouchX();                // Read raw X coordinate from touchscreen controller (after touch detected)
        y = ReadTouchY();                // Read raw Y coordinate from touchscreen controller (after touch detected)

         ButtonNum = ButtonTouch(x, y);  // Determine which button was pressed based on (X, Y) position
                                         // Returns button index if touch overlaps a defined button area
 
    // --- (D) Menu Navigation Based on Touchscreen Input ---
    if(ButtonNum != 0)  // A button press was detected (ButtonNum = 0): user requested a screen change
    {
    // If "Check" screen button (Button 1) is pressed  
    if(ButtonNum == 1)                
    {  
        screen = 1;                   // Set screen index to 1 (Check screen)
        runFlag = 0;                  // Disable irrigation logic
        Relay_Off();                  // Ensure pump is off before entering Check mode
        screen1();                    // Display the Check screen (show sensor reading buttons)
    }  
    // If "Setup" screen button (Button 2) is pressed  
    else if(ButtonNum == 2)          
    {  
        screen = 2;                   // Set screen index to 2 (Setup screen)
        runFlag = 0;                  // Disable automatic mode to allow manual RTC edits
        Relay_Off();                  // Turn off pump while adjusting settings
        screen2();                    // Display the Setup screen (RTC and threshold adjustments)
    }  
    // If "Project" screen button (Button 3) is pressed  
    else if(ButtonNum == 3)          
    {  
        screen = 3;                   // Set screen index to 3 (Project mode)
        runFlag = 1;                  // Enable automatic irrigation logic (runProject active)
        screen3();                    // Display the Project screen
    }  

// --- (D.1) Sub-menu: CHECK Screen Options ---
if(screen == 1) {                         // Only handle sub-menu actions when current screen is CHECK (1)
    if(ButtonNum == 4) {                  // "Time" button pressed
        LCD_fillRect(10,200,300,40,BLUE); // Clear result area (x=10,y=200,w=300,h=40) with blue background
        LCD_setCursor(15,215);            // Position text cursor inside the result area
        printf("Time: %02d:%02d:%02d", hour, minute, second); // Print time HH:MM:SS with zero padding
    } else if(ButtonNum == 5) {           // "Tempr" button pressed (temperature)
        LCD_fillRect(10,200,300,40,BLUE); // Refresh result area background
        LCD_setCursor(15,215);            // Set cursor for text output
        printf("Temp: %.2f C", temp);     // Show LM75 temperature with 2 decimals
    } else if(ButtonNum == 6) {           // "Soil" button pressed
        LCD_fillRect(10,200,300,40,BLUE); // Refresh result area
        LCD_setCursor(15,215);            // Set cursor position
        printf("Soil: %u%%", soil);       // Print soil moisture percentage (0..100)
    } else if(ButtonNum == 7) {           // "Rain" button pressed
        LCD_fillRect(10,200,300,40,BLUE); // Refresh result area
        LCD_setCursor(15,215);            // Set cursor position
        printf("Rain: %u%%", rain);       // Print rain sensor percentage (0..100)
    } else if(ButtonNum == 8) {           // "Light" button pressed
        LCD_fillRect(10,200,300,40,BLUE); // Refresh result area
        LCD_setCursor(15,215);            // Set cursor position
        printf("Light: %u%%", light);     // Print light intensity percentage (0..100)
    } else if(ButtonNum == 9) {           // "Pump" button pressed
        LCD_fillRect(10,200,300,40,BLUE); // Refresh result area
        LCD_setCursor(15,215);            // Set cursor position
        if(Relay)                         // Read Relay sbit (P0.2): 1=coil energized (pump ON), 0=OFF
            printf("Pump: ON");           // Report pump state ON
        else
            printf("Pump: OFF");          // Report pump state OFF
    } else if(ButtonNum == 10) {          // "Servo" button pressed
        LCD_fillRect(10,200,300,40,BLUE); // Refresh result area
        LCD_setCursor(15,215);            // Set cursor position
        printf("Servo: %u deg", (unsigned int)((angle - 600) / 10));

    }
}
 
 // --- (D.2) Sub-menu: Setup Screen for RTC Adjustment  and TEMP_THRESHOLD---
if(screen == 2) {                             // Handle buttons only when 'Setup' screen is active
    if(ButtonNum == 13) {                     // Increase hour (+)
        hour++;                               // Increment hour value
        if(hour >= 24) hour = 0;              // Wrap 23 -> 0 (24h format)
        writeDS1307(0x02, hour);              // Write HOURS register (0x02); function converts to BCD
        LCD_fillRect(185,75,80,30,GREEN);     // Clear/redraw the Hour display field
        LCD_setCursor(200,80);                // Position cursor inside the Hour field
        printf("%d", hour);                   // Show updated hour
    } else if(ButtonNum == 14) {              // Decrease hour (-)
        if(hour == 0) hour = 23; else hour--; // Wrap 0 -> 23, otherwise decrement
        writeDS1307(0x02, hour);              // Update HOURS in RTC (BCD handled inside)
        LCD_fillRect(185,75,80,30,GREEN);     // Refresh Hour field background
        LCD_setCursor(200,80);                // Cursor for printing
        printf("%d", hour);                   // Print hour
    } else if(ButtonNum == 15) {              // Increase minute (+)
        minute++;                             // Increment minute value
        if(minute >= 60) minute = 0;          // Wrap 59 -> 0
        writeDS1307(0x01, minute);            // Write MINUTES register (0x01) in BCD
        LCD_fillRect(185,115,50,30,GREEN);    // Clear/redraw the Minute field
        LCD_setCursor(200,120);               // Position cursor inside Minute field
        printf("%d", minute);                 // Show updated minute
    } else if(ButtonNum == 16) {              // Decrease minute (-)
        if(minute == 0) minute = 59; else minute--; // Wrap 0 -> 59, otherwise decrement
        writeDS1307(0x01, minute);            // Update MINUTES in RTC (BCD handled inside)
        LCD_fillRect(185,115,50,30,GREEN);    // Refresh Minute field background
        LCD_setCursor(200,120);               // Cursor for printing
        printf("%d", minute);                 // Print minute
    } else if(ButtonNum == 17) {              // Adjust temperature threshold (+/- cycles 20..30°C)
        TEMP_THRESHOLD++;                     // Increment threshold
        if(TEMP_THRESHOLD > 30) TEMP_THRESHOLD = 20; // Wrap back to 20°C after 30°C
        writeDS1307(0x06, TEMP_THRESHOLD);    // (Optional) store threshold in DS1307 register 0x06
        LCD_fillRect(185,155,50,30,GREEN);    // Clear/redraw the Threshold field
        LCD_setCursor(200,160);               // Position cursor inside Threshold field
        printf("%d", TEMP_THRESHOLD);         // Show updated threshold
    }
}
 
    }   // End of Menu Navigation block

// --- (E) Loop Delay ---  
// delay_ms(20);  // Optional delay between iterations for UI responsiveness 
    } //End of the MAIN LOOP  
} //End of the MAIN FUNCTION  

// ------------------- Screen Drawing Functions -------------------  
// screen0(): Draws the startup screen with main menu buttons.  
void screen0(void)  
{  
    LCD_fillScreen(BLACK);                       // Clear the LCD screen by filling with black  

    // Draw the main navigation buttons  
    LCD_drawButton(1,  20, 20,  70, 40, 5, BLUE, WHITE, "Check",   2);  // Draw "Check" button at (20,20) with size 70×40, radius 5, colors blue/white, text size 2  
    LCD_drawButton(2,  95, 20,  70, 40, 5, BLUE, WHITE, "Setup",   2);  // Draw "Setup" button at (95,20)  
    LCD_drawButton(3, 170, 20, 100, 40, 5, BLUE, WHITE, "Project", 2);  // Draw "Project" button at (170,20)  

    // Project Title (Top Center)  
    LCD_setCursor(10, 80);                        // Set cursor coordinates to (10,80) for text  
    LCD_setTextSize(2);                           // Set text size to 2 (double height)  
    LCD_setText1Color(YELLOW);                    // Set text color to yellow for title  
    printf("Automatic Irrigation Sys");            // Print main title on screen  

    // Student names  
    LCD_setCursor(10, 110);                       // Set cursor to (10,110) for first name  
    LCD_setText1Color(WHITE);                     // Set text color to white for names  
    printf("Ivgeni-Goriatchev");                  // Print first student name  

   
}  

// screen1(): Draws the "Check" screen which displays sensor data sub-menu.  
void screen1(void)  
{  
    LCD_fillScreen(BLACK);                       // Clear the screen (fill with black)  
    LCD_drawButton(1,20,20,70,40,5,BLUE,WHITE,"Check",2);   // Redraw top menu buttons  
    LCD_drawButton(2,95,20,70,40,5,BLUE,WHITE,"Setup",2);   // Draw "Setup" button  
    LCD_drawButton(3,170,20,100,40,5,BLUE,WHITE,"Project",2); // Draw "Project" button  
    LCD_drawButton(4,20,65,70,40,5,BLUE,WHITE,"Time",2);      // Draw sub-menu "Time" button  
    LCD_drawButton(5,95,65,70,40,5,BLUE,WHITE,"Tempr",2);     // Draw sub-menu "Temperature" button  
    LCD_drawButton(6,20,110,70,40,5,BLUE,WHITE,"Soil",2);      // Draw sub-menu "Soil" button  
    LCD_drawButton(7,95,110,70,40,5,BLUE,WHITE,"Rain",2);     // Draw sub-menu "Rain" button  
    LCD_drawButton(8,170,110,100,40,5,BLUE,WHITE,"Light",2);  // Draw sub-menu "Light" button  
    LCD_drawButton(9, 20,155,70,40,5,BLUE,WHITE,"Pump",2);     // Draw "Pump" button at (20,155)
    LCD_drawButton(10,95,155,70,40,5,BLUE,WHITE,"Servo",2);    // Draw "Servo" button at (95,155)
    LCD_fillRect(10,200,300,40,BLUE);             // Draw a blue rectangle for result display area at (10,200) size 300×40  
    LCD_setCursor(15,215);                        // Position cursor at (15,215) inside result area  
    printf("Result:");                            // Print "Result:" label  
}  

// screen2(): Draws the "Setup" screen for RTC adjustments.  
void screen2(void)  
{  
    LCD_fillScreen(BLACK);                       // Clear the screen (fill with black)  
    LCD_drawButton(1,20,20,70,40,5,GREEN,WHITE,"Check",2);    // Draw "Check" button in green/white  
    LCD_drawButton(2,95,20,70,40,5,GREEN,WHITE,"Setup",2);    // Draw "Setup" button in green/white  
    LCD_drawButton(3,170,20,100,40,5,GREEN,WHITE,"Project",2); // Draw "Project" button in green/white  
    LCD_print2C(10,80,"Hour",2,GREEN,BLACK);     // Print centered "Hour" label at (10,80) in green on black  
    LCD_drawButton(13,65,75,50,30,5,GREEN,WHITE,"+",2);  // Draw "+" button to increase hour at (65,75) with size 50×30  
    LCD_drawButton(14,125,75,50,30,5,GREEN,WHITE,"-",2); // Draw "-" button to decrease hour at (125,75)  
    LCD_fillRect(185,75,80,30,GREEN);             // Clear hour display area by drawing a green rectangle at (185,75) size 80×30  
    LCD_print2C(10,120,"Min",2,GREEN,BLACK);      // Print centered "Min" label at (10,120) in green on black  
    LCD_drawButton(15,65,115,50,30,5,GREEN,WHITE,"+",2); // Draw "+" button to increase minute at (65,115)  
    LCD_drawButton(16,125,115,50,30,5,GREEN,WHITE,"-",2); // Draw "-" button to decrease minute at (125,115)  
    LCD_fillRect(185,115,80,30,GREEN);            // Clear minute display area by drawing a green rectangle at (185,115) size 80×30  
	  
    LCD_print2C(10,160,"Temp", 2, GREEN, BLACK); // Print centered "Temp" label at (10,160)  
    LCD_drawButton(17,65,155,110,30,5,GREEN,WHITE,"+/-",3); // Draw "+/-" button to adjust TEMP_THRESHOLD at (65,155) size 110×30  
    LCD_fillRect(185,155,80,30,GREEN);            // Clear temperature threshold display area by drawing a green rectangle  
}  

// screen3(): Draws the "Project" screen for real-time operation.  
void screen3(void)  
{  
    LCD_fillScreen(BLACK);                       // Clear the screen (fill with black)  
    LCD_drawButton(1,20,20,70,40,5,RED,WHITE,"Check",2);    // Draw "Check" button in red/white  
    LCD_drawButton(2,95,20,70,40,5,RED,WHITE,"Setup",2);    // Draw "Setup" button in red/white  
    LCD_drawButton(3,170,20,100,40,5,RED,WHITE,"Project",2); // Draw "Project" button in red/white  
} 
// --------------------------------------------------------------------  
// runProject(): Implements the real-time project logic (irrigation control)  
// --------------------------------------------------------------------  
void runProject(void)  
{  
    // Set LCD text color (foreground WHITE on background BLACK)  
    LCD_setText2Color(WHITE, BLACK);  
    // Display current time label and value  
    LCD_setCursor(20,70);                // Position cursor for "Time:" label  
    printf("Time:");                     // Print "Time:"  
    printTime(hour, minute, second);     // Print formatted time (calls helper to format HH:MM:SS)     
    // Display temperature  
    LCD_setCursor(20,100);               // Position cursor for temperature line  
    printf("Temp=%.2f C  (Th=%d)", temp, TEMP_THRESHOLD); // Print temperature and threshold  
    // Display sensor readings for rain, soil, and light  
    LCD_setCursor(20,130);               // Position cursor for rain  
    printf("Rain=%d%%", rain);           // Print rain sensor percentage  
    LCD_setCursor(20,160);               // Position cursor for soil  
    printf("Soil=%d%%", soil);           // Print soil moisture percentage  
    LCD_setCursor(20,190);               // Position cursor for light  
    printf("Light=%d%%", light);         // Print light sensor percentage  
    // ---------------- Combined Irrigation Conditions ----------------  
    // Conditions to activate irrigation:  
    //   1. Soil sensor reading must be at least SOIL_THRESHOLD (i.e., soil is dry).  
    //   2. Current time must be within the allowed windows: 04:00-08:00 or 19:00-22:00.  
    //   3. Ambient light sensor reading must be below LIGHT_THRESHOLD (i.e., not too bright).  
    //   4. Rain sensor reading must be at least RAIN_THRESHOLD (i.e., no significant rain).  
    //   5. Temperature must be below TEMP_THRESHOLD.  
// Check if soil is dry enough
if (soil < SOIL_THRESHOLD) {
    Relay_Off();           // Soil is still moist – turn off the pump
    return;                // Exit function early – no need to evaluate further conditions
}
if (!(((hour >= 4) && (hour < 8)) || ((hour >= 19) && (hour < 22)))) {   // Current time must be within the allowed windows: 04:00-08:00 or 19:00-22:00.
    Relay_Off();           // Time is outside allowed irrigation window – disable pump
    return;                // Skip irrigation logic
}
if (light >= LIGHT_THRESHOLD) {
    Relay_Off();           // Ambient light is too strong – cancel irrigation
    return;
}
if (rain < RAIN_THRESHOLD) {
    Relay_Off();           // Rain has been detected – skip watering
    return;
}
if (temp >= TEMP_THRESHOLD) {
    Relay_Off();           // Temperature too high – skip irrigation
    return;
}

// All conditions met – activate irrigation
Relay_On();                // Enable pump via relay control

// Gradually sweep the servo angle
// Note: 30 µs step = 30×4 = 120 PCA ticks (~1.67% of the 600–2400 µs span; ~0.18% duty change per 16.384 ms frame)
if (directionUp) {                    // State flag: 1 = sweep up, 0 = sweep down
    angle += 30;                      // Step +30 µs (smooth increment)
    if (angle >= 2400) {              // Upper bound check (max 2.4 ms)
        angle = 2400;                 // Clamp to max safe width
        directionUp = 0;              // Flip direction at the top end
    }
} else {                              // directionUp == 0 -> sweep down
    angle -= 30;                      // Step -30 µs (smooth decrement)
    if (angle <= 600) {               // Lower bound check (min 0.6 ms)
        angle = 600;                  // Clamp to min safe width
        directionUp = 1;              // Flip direction at the bottom end
    }
}
pulse(angle);      // Load PCA compare with new pulse width (µs) Update PWM HIGH time: writes 16-bit compare (CPL0/CPH0); PCA ends HIGH at match
delay_ms(20);      // ~20 ms between steps -> natural servo motion
}  
 
