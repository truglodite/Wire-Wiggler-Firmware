// USER CONFIGURATION: Edit this file to suit your specific build. 
// Most times this is the only file that needs to be touch.

// Uncomment only one board from the list below... see boards file and /images for more info.
// If using a custom board file, leave all these commented, and use the custom board 
// setting below, and change filename to match your board file (which should be in boards).
// Note some microprocessors may not yet be defined in platform.ini. If there is no build option
// for your board, a section may be added to platform.ini to accomodate.
// When adding a new board, consider adding a schematic to /images for user reference.

//#define UNO_CNC_SHIELD_V_3
#define NANO_CNC_V_4
//#define RAMPS_V_1_4_MEGA2560
//#define TEENSY_UNO_CNC_SHIELD_V_3
//#define ANET_A8_ATMEGA1280_PRINTER_BOARD

// Define heartbeat LED... choose only one option
// #define ledUseBuiltin            // Uncomment to enable heartbeat on built in LED
#define ledUseExternal              // comment to disable heartbeat on external LED

//#include <../boards/nameofcustomboard.h> // edit and uncomment for a custom board file.

// Uncomment to enable antenna polarity rotator feature (manual control with poti from 0V to Vcc)
#define POLARIZER
//#define POL_POT_HYSTERESIS  1     // Increase to eliminate polarize axis shaking while poti is not moved
//#define POL_POT_SAMPLES   4       // Size of array -1 to store rolling average samples

// Uncomment to enable 5883 compass feedback feature
//#define COMPASS

// Uncomment to enable mpu6050 IMU feedback feature
//#define IMU_FEEDBACK

// Uncomment to enable parking feature... not yet implemented!
//#define PARKING

// Uncomment and move debug led to troubled spot
//#define DEBUG                   

// Disable motors if WDT is triggered (not used for Teensy MCU's)
#define WATCHDOG

// The following lines can be uncommented and values changed to overide default settings (default settings shown for reference)

// #define BAUDRATE           57600 // Set the Baudrate of easycomm 3 protocol (57600)

// #define AZI_RATIO          68.4  // Azimuth mechanical gear ratio (19:1 planetary * 72:20 belt drive = 68.4:1)
// #define AZI_MICROSTEP      1     // Azimuth stepper driver microstepping
// #define AZI_VMAX           10.0  // Antenna deg/s
// #define AZI_ACC_MAX        20.0  // Antenna deg/s^2
// #define AZI_MIN_ANGLE      0.0   // Minimum antenna azimuth angle
// #define AZI_MAX_ANGLE      360.0 // Maximum antenna azimuth angle
// #define AZI_PARK_ANGLE     90.0  // Antenna azimuth parking position

// #define ELE_RATIO          68.4  // Elevation mechanical gear ratio
// #define ELE_MICROSTEP      1     // Elevation stepper driver microsteppinh
// #define ELE_VMAX           5.0   // Antenna deg/s
// #define ELE_ACC_MAX        20.0  // Antenna deg/s^2
// #define ELE_MIN_ANGLE      0.0   // Minimum antenna elevation angle
// #define ELE_MAX_ANGLE      180.0 // Maximum antenna elevation angle
// #define ELE_PARK_ANGLE     -5.0  // Antenna elevation parking position

// #define POL_RATIO          3.6   // Polarization mechanical gear ratio (72:20 belt drive = 3.6:1)
// #define POL_MICROSTEP      1     // Polarization stepper driver microstepping
// #define POL_VMAX           90.0  // Antenna deg/s
// #define POL_ACC_MAX        80.0  // Antenna deg/s^2
// #define POL_MIN_ANGLE      0.0   // Minimum antenna polarization angle
// #define POL_MAX_ANGLE      180.0 // Maximum antenna polarization angle
// #define POL_PARK_ANGLE     0.0   // Antenna polarizer parking position

// #define SPR                200   // Motor steps per revolution (200.0 for standard 1.8degree motors)
// #define MIN_PULSE_WIDTH    20    // uSec (for AccelStepper)
// #define DEFAULT_HOME_STATE LOW   // Low for N.O. switches
// #define HOME_DELAY         1000  // millis to keep moving after hitting endstop (time to settle before resetting zeros)


