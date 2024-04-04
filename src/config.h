// USER CONFIGURATION

// Uncomment one board from the list... or add a pins file if needed.
//#define UNO_CNC_SHIELD_V_3
//#define BRUSHES_N_POTS
#define ANET_A8_ATMEGA1280_PRINTER_BOARD
//#define RAMPS_V_1_4_MEGA2560

#define AZI_RATIO          108.0 // Azimuth mechanical gear ratio
#define AZI_MICROSTEP      16.0  // Azimuth driver microsteps
#define AZI_VMAX           300   // motor deg/s
#define AZI_ACC_MAX        120   // motor deg/s^2
#define MIN_AZI_ANGLE      0     // Minimum angle of azimuth
#define MAX_AZI_ANGLE      360   // Maximum angle of azimuth

#define ELE_RATIO          108.0 // Elevation mechanical gear ratio
#define ELE_MICROSTEP      16.0  // Elevation driver microsteps
#define ELE_VMAX           300   // motor deg/s
#define ELE_ACC_MAX        120   // motor deg/s^2
#define MIN_ELE_ANGLE      0     // Minimum angle of elevation
#define MAX_ELE_ANGLE      180   // Maximum angle of elevation

#define SPR                200.0 // Motor steps per revolution (200.0 for standard 1.8degree motors)
#define MIN_PULSE_WIDTH    20    // uSec (for AccelStepper)
#define DEFAULT_HOME_STATE LOW   // Change to LOW according to Home sensor
#define HOME_DELAY         500   // millis to keep moving after hitting endstop??! WTF lol
#define SAMPLE_TIME        0.1   // Control loop in sec

#define WATCHDOG                 // Disable motors if WDT is triggered
//#define DEBUG                  // Uncomment and move debug led to troubled spot 