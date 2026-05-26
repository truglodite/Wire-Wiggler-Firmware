#ifndef LIBRARIES_GLOBALS_H_
#define LIBRARIES_GLOBALS_H_

#include <Arduino.h>

/** Rotator status */
enum _rotator_status {
    idle = 1, moving = 2, pointing = 4, error = 8
};
/** Rotator Errors */
enum _rotator_error {
    no_error = 1, sensor_error = 2, homing_error = 4, motor_error = 8,
    over_temperature = 12, wdt_error = 16
};
/** Rotator Control Modes */
enum _control_mode {
    position = 0, speed = 1
};

struct _control{
    float input;           // Motor Position feedback in deg
    float input_prv;       // T-1 Motor Position feedback in deg
    float speed;           // Motor Rotation speed in deg/s
    float setpoint;        // Position set point in deg
    float setpoint_speed;  // Speed set point in deg/s
    uint16_t load;         // Motor Load in mA
    double u;              // Control signal range 0-255
    double p, i, d;        // Control gains
};

#ifndef POLARIZER
    struct _rotator{
        volatile enum _rotator_status rotator_status; // Rotator status
        volatile enum _rotator_error rotator_error;   // Rotator error
        enum _control_mode control_mode;              // Control mode
        bool homing_flag;                             // Homing flag
        bool parking_flag;                            // Parking flag
        int8_t inside_temperature;                    // Inside Temperature
        double park_az, park_el;                      // Park position for both axis
        uint8_t fault_az, fault_el;                   // Motor drivers fault flag
        bool switch_eleMin, switch_aziMin;            // End-stop values
    };
#else
    struct _rotator{
        volatile enum _rotator_status rotator_status;      // Rotator status
        volatile enum _rotator_error rotator_error;        // Rotator error
        enum _control_mode control_mode;                   // Control mode
        bool homing_flag;                                  // Homing flag
        bool parking_flag;                                 // Parking flag
        int8_t inside_temperature;                         // Inside Temperature
        float park_az, park_el, park_po;                   // Park position for both axis
        uint8_t fault_az, fault_el, fault_po;              // Motor drivers fault flag
        bool switch_eleMin, switch_aziMin, switch_polMin;  // End-stop values
    };
#endif

_control control_az = { .input = 0.0, .input_prv = 0.0, .speed=0.0, .setpoint = 0.0,
                        .setpoint_speed = 0.0, .load = 0, .u = 0.0, .p = 8.0,
                        .i = 0.0, .d = 0.5 };
_control control_el = { .input = 0.0, .input_prv = 0.0, .speed=0.0, .setpoint = 0.0,
                        .setpoint_speed = 0.0, .load = 0, .u = 0.0, .p = 10.0,
                        .i = 0.0, .d = 0.3 };
#ifdef POLARIZER
    _control control_po = { .input = 0.0, .input_prv = 0.0, .speed=0.0, .setpoint = 0.0,
                        .setpoint_speed = 0.0, .load = 0, .u = 0.0, .p = 10.0,
                        .i = 0.0, .d = 0.3 };

    _rotator rotator = { .rotator_status = idle, .rotator_error = no_error,
                     .control_mode = position, .homing_flag = false, .parking_flag = false,
                     .inside_temperature = 0, .park_az = 0.0, .park_el = 0.0, .park_po = 0.0,
                     .fault_az = LOW, .fault_el = LOW , .fault_po = LOW , .switch_eleMin = false,
                     .switch_aziMin = false , .switch_polMin = false};
#else
    _rotator rotator = { .rotator_status = idle, .rotator_error = no_error,
                     .control_mode = position, .homing_flag = false, .parking_flag = false,
                     .inside_temperature = 0, .park_az = 0.0, .park_el = 0.0,
                     .fault_az = LOW, .fault_el = LOW , .switch_eleMin = false,
                     .switch_aziMin = false};
#endif

#endif /* LIBRARIES_GLOBALS_H_ */
