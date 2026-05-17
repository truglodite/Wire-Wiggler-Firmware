/* Wire Wiggler:
Firmware for an antenna rotator using nc controllers with stepper motors.

This code is based on a modified version of satnogs-rotator-firmware by Quartapound:
https://gitlab.com/Quartapound/satnogs-rotator-firmware

******************************
*  Licensed under the GPLv3  *
******************************
*/

#include <../lib/defaults.h>
#include <AccelStepper.h>
#include <../lib/globals.h>
#include <../lib/easycomm.h>
#include <../lib/endstop.h>

// INITIALIZE /////////////////////////////////////////////////// INITIALIZE //
easycomm comm;

// Create AccelSteppers and endstops
// syntax: AccelStepper stepper_x(interface, stepPin, dirPin);
// interface 1=AccelStepper::DRIVER ...aka standard STEP/DIR drivers
AccelStepper stepper_el(1, eleStepPin, eleDirPin);
AccelStepper stepper_az(1, aziStepPin, aziDirPin);

endstop switch_eleMin(eleMinStopPin, DEFAULT_HOME_STATE), switch_aziMin(aziMinStopPin, DEFAULT_HOME_STATE);
#ifdef POLARIZER
    AccelStepper stepper_po(1, polStepPin, polDirPin);
    endstop switch_polMin(polMinStopPin, DEFAULT_HOME_STATE);
#endif

#ifdef WATCHDOG
    wdt_timer wdt;
#endif

#ifndef POLARIZER
    enum _rotator_error homing(long seek_aziMin, long seek_eleMin);
#else
    enum _rotator_error homing(long seek_aziMin, long seek_eleMin, long seek_polMin);
#endif

long deg2step(float deg, float ratio, uint8_t microsteps);
float step2deg(long step, float ratio, uint8_t microsteps);
int readPolPot();

long eleMaxStepRate = 0;
long eleMaxStepAcc = 0;
long aziMaxStepRate = 0;
long aziMaxStepAcc = 0;
#ifdef POLARIZER
    long polMaxStepRate = 0;
    long polMaxStepAcc = 0;
    int polPot = 0;
    int lastPolPot = 0;
    int rawpolpot[POL_POT_SAMPLES]; // holder for rolling average... not needed with 10nF capacitor.
#endif
#if defined(ledUseBuiltin) || defined(ledUseExternal)
    bool ledState = 0;  //logic
    uint32_t ledPeriod = 1000; // msec period for default heartbeat flashing
    uint32_t ledTime = 0; // time holder
#endif

// SETUP //////////////////////////////////////////////////////////// SETUP //
void setup() {
    // Initialize homing switches
    switch_eleMin.init();
    switch_aziMin.init();
    #ifdef POLARIZER
        switch_polMin.init();
        //pinMode(polPotPin, INPUT); // init poti pin, no pullup
    #endif
    #ifdef ledUseBuiltin
        pinMode(ledPinBuiltin, OUTPUT);  // init led pin
    #endif
    #ifdef ledUseExternal
        pinMode(ledPinExternal, OUTPUT); // init led pin
    #endif

    // Serial Communication
    comm.easycomm_init();
    
    // Azimuth motor
    stepper_az.setEnablePin(aziENpin);
    stepper_az.setPinsInverted(false, false, true);    // syntax: setPinsInverted(dir, step, enable), true/false
    stepper_az.enableOutputs();
    aziMaxStepRate = deg2step(AZI_VMAX, AZI_RATIO, AZI_MICROSTEP);
    aziMaxStepAcc = deg2step(AZI_ACC_MAX, AZI_RATIO, AZI_MICROSTEP);
    stepper_az.setMaxSpeed(aziMaxStepRate);
    stepper_az.setAcceleration(aziMaxStepAcc);
    stepper_az.setMinPulseWidth(MIN_PULSE_WIDTH);

    // Elevation motor
    stepper_el.setEnablePin(eleENpin);
    stepper_el.setPinsInverted(false, false, true);
    eleMaxStepRate = deg2step(ELE_VMAX, ELE_RATIO, ELE_MICROSTEP);
    eleMaxStepAcc = deg2step(ELE_ACC_MAX, ELE_RATIO, ELE_MICROSTEP);
    stepper_el.enableOutputs();
    stepper_el.setMaxSpeed(eleMaxStepRate);
    stepper_el.setAcceleration(eleMaxStepAcc);
    stepper_el.setMinPulseWidth(MIN_PULSE_WIDTH);

    // Polarizer motor
    #ifdef POLARIZER        
        stepper_po.setEnablePin(polENpin);
        stepper_po.setPinsInverted(false, false, true);
        polMaxStepRate = deg2step(POL_VMAX, POL_RATIO, POL_MICROSTEP);
        polMaxStepAcc = deg2step(POL_ACC_MAX, POL_RATIO, POL_MICROSTEP);
        stepper_po.enableOutputs();
        stepper_po.setMaxSpeed(polMaxStepRate);
        stepper_po.setAcceleration(polMaxStepAcc);
        stepper_po.setMinPulseWidth(MIN_PULSE_WIDTH);
    #endif
    //  WDT
    #ifdef WATCHDOG
        wdt.watchdog_init();
    #endif
}

// LOOP //////////////////////////////////////////////////////////// LOOP //
void loop() {
    // Debug LED on... move this where ever to help with debugging
    #ifdef DEBUG
        #ifdef ledUseBuiltin
            digitalWrite(ledPinBuiltin,HIGH); //turn on led for debugging
        #endif
        #ifdef ledUseExternal
            digitalWrite(ledPinExternal,HIGH); //turn on led for debugging
        #endif
    #endif
    // Update WDT
    #ifdef WATCHDOG
        wdt.watchdog_reset();
    #endif
    // Run easycomm subroutine
    comm.easycomm_proc();
    // Get position of axis
    control_az.input = step2deg(stepper_az.currentPosition(), AZI_RATIO, AZI_MICROSTEP);
    control_el.input = step2deg(stepper_el.currentPosition(), ELE_RATIO, ELE_MICROSTEP);
    #ifdef POLARIZER
        control_po.input = step2deg(stepper_po.currentPosition(), POL_RATIO, POL_MICROSTEP);
    #endif
    // No error flags
    if (rotator.rotator_status != error) {
        // Not yet homed
        if (rotator.homing_flag == false) {
            rotator.control_mode = position;
            // Run homing subroutine
            #ifndef POLARIZER
                rotator.rotator_error = homing(deg2step(AZI_MAX_ANGLE, AZI_RATIO, AZI_MICROSTEP),
                                           deg2step(ELE_MAX_ANGLE, ELE_RATIO, ELE_MICROSTEP));
            #else
                rotator.rotator_error = homing(-deg2step(AZI_MAX_ANGLE, AZI_RATIO, AZI_MICROSTEP),
                                           -deg2step(ELE_MAX_ANGLE, ELE_RATIO, ELE_MICROSTEP),
                                           -deg2step(POL_MAX_ANGLE, POL_RATIO, POL_MICROSTEP));
            #endif
            // Homing subroutine done, set flag and idle mode, or set error flags
            if (rotator.rotator_error == no_error) {
                rotator.rotator_status = idle;
                rotator.homing_flag = true;
            } 
            else {
                rotator.rotator_status = error;
                rotator.rotator_error = homing_error;
            }
        } 
        // Homing completed... start rotator routine
        else {
            // Rotate without polarizer
            #ifndef POLARIZER
                stepper_az.moveTo(deg2step(control_az.setpoint, AZI_RATIO, AZI_MICROSTEP));
                stepper_el.moveTo(deg2step(control_el.setpoint, ELE_RATIO, ELE_MICROSTEP));
                rotator.rotator_status = pointing;
                // Move azimuth and elevation motors
                stepper_az.run();
                stepper_el.run();
                // Done moving, change to idle mode
                if (stepper_az.distanceToGo() == 0 && stepper_el.distanceToGo() == 0) {
                    rotator.rotator_status = idle;
                }
            #else
            // Rotate w/ polarizer
                // polPot = analogRead(polPotPin); // read poti
                polPot = readPolPot();
                // Poti has moved enough to respond to
                if((polPot - lastPolPot > POL_POT_HYSTERESIS) || (lastPolPot - polPot > POL_POT_HYSTERESIS)) {
                    lastPolPot = polPot;
                    control_po.setpoint = ((float)polPot / float(1023.0)) * float(POL_MAX_ANGLE);   // use 10bit reading to change polarize setpoint angle
                }
                // Move motors
                stepper_az.moveTo(deg2step(control_az.setpoint, AZI_RATIO, AZI_MICROSTEP));
                stepper_el.moveTo(deg2step(control_el.setpoint, ELE_RATIO, ELE_MICROSTEP));
                stepper_po.moveTo(deg2step(control_po.setpoint, POL_RATIO, POL_MICROSTEP));
                rotator.rotator_status = pointing;
                stepper_az.run();
                stepper_el.run();
                stepper_po.run();
                // Idle rotator
                if (stepper_az.distanceToGo() == 0 && stepper_el.distanceToGo() == 0 && stepper_po.distanceToGo() == 0) {
                    rotator.rotator_status = idle;
                }
            #endif

            // LED heartbeat: slow blink while rotator routine is running
            #ifndef DEBUG
                #ifdef ledUseBuiltin
                    if(millis() - ledTime > ledPeriod)   {
                        if(ledState)    {
                            digitalWrite(ledPinBuiltin,LOW);
                            ledState = 0;
                            ledTime = millis();
                        }
                        else{
                            digitalWrite(ledPinBuiltin,HIGH);
                            ledState = 1;
                            ledTime = millis();
                        }
                    }
                #endif
                #ifdef ledUseExternal
                    if(millis() - ledTime > ledPeriod)   {
                        if(ledState)    {
                            digitalWrite(ledPinExternal,LOW);
                            ledState = 0;
                            ledTime = millis();
                        }
                        else{
                            digitalWrite(ledPinExternal,HIGH);
                            ledState = 1;
                            ledTime = millis();
                        }
                    }
                #endif
            #endif
        }
    } 
    // Error: stop motors and disable the motor drivers
    else {  
        stepper_az.stop();
        stepper_az.disableOutputs();
        stepper_el.stop();
        stepper_el.disableOutputs();
        #ifdef POLARIZER
            stepper_po.stop();
            stepper_po.disableOutputs();           
        #endif
        if (rotator.rotator_error != homing_error) {
            // Reset error according to error value
            rotator.rotator_error = no_error;
            rotator.rotator_status = idle;
        }
        #ifndef DEBUG
            #ifdef ledUseBuiltin
                // LED heartbeat: remain on if an error occured
                if(!ledState)    {
                    digitalWrite(ledPinBuiltin,HIGH);
                    ledState = 1;
                }
            #endif
            #ifdef ledUseExternal
                // LED heartbeat: remain on if an error occured
                if(!ledState)    {
                    digitalWrite(ledPinExternal,HIGH);
                    ledState = 1;
                }
            #endif
        #endif
    }
}

// FUNCTIONS //////////////////////////////////////////////////////////// FUNCTIONS //

#ifndef POLARIZER
// Homing function without polarizer feature enabled
    enum _rotator_error homing(long seek_aziMin, long seek_eleMin) {
        // Declare and init flags
        bool isHome_az = false;
        bool isHome_el = false;
        bool switches_closed = false;
        unsigned long time = 0;

        // If azi endstop is closed, prep for back off
        if(switch_aziMin.get_state())   {
            isHome_az = true;
            switches_closed = true;
            stepper_az.moveTo(-seek_aziMin);
        }
        // If ele endstop is closed, prep for back off
        if(switch_eleMin.get_state())   {
            isHome_el = true;
            switches_closed = true;
            stepper_el.moveTo(-seek_eleMin);
        }
        // Back off any closed switches
        while(isHome_az || isHome_el || isHome_po)  {
            if (switch_aziMin.get_state() == false && isHome_az) {
                // Switch opened, set flag and stop motor
                stepper_az.stop();
                isHome_az = false;
            }
            if (switch_eleMin.get_state() == false && isHome_el) {
                // Switch opened, set flag and stop motor
                stepper_el.stop();
                isHome_el = false;
            }

            // Move motors
            stepper_az.run();
            stepper_el.run();

            // Traveled beyond step limit... mechanical failure. Exit loop with error
            if ((stepper_az.distanceToGo() == 0 && isHome_az) ||
                (stepper_el.distanceToGo() == 0 && isHome_el)) {
                return homing_error;
            }
                                
            #ifdef WATCHDOG
                wdt.watchdog_reset();
            #endif

            #ifndef DEBUG
                #ifdef ledUseBuiltin
                    // LED heartbeat: fast blink while homing
                    if(millis() - ledTime > ledPeriod/6)   {
                        if(ledState)    {
                            digitalWrite(ledPinBuiltin,LOW);
                            ledState = 0;
                            ledTime = millis();
                        }
                        else{
                            digitalWrite(ledPinBuiltin,HIGH);
                            ledState = 1;
                            ledTime = millis();
                        }
                    }
                #endif
                #ifdef ledUseExternal
                    // LED heartbeat: fast blink while homing
                    if(millis() - ledTime > ledPeriod/6)   {
                        if(ledState)    {
                            digitalWrite(ledPinExternal,LOW);
                            ledState = 0;
                            ledTime = millis();
                        }
                        else{
                            digitalWrite(ledPinExternal,HIGH);
                            ledState = 1;
                            ledTime = millis();
                        }
                    }
                #endif
            #endif
        }

        if(switches_closed)  {
            // Delay to allow switch back off moves to complete
            time = millis();
            while (millis() - time < HOME_DELAY) {
                #ifdef WATCHDOG
                    wdt.watchdog_reset();
                #endif
                stepper_az.run();
                stepper_el.run();
            }
            switches_closed = false;
        }

        // Endstop switches all open, move motors to "seek" position
        stepper_az.moveTo(seek_aziMin);
        stepper_el.moveTo(seek_eleMin);

        // Homing loop
        while (isHome_az == false || isHome_el == false) {
            if (switch_aziMin.get_state() == true && !isHome_az) {
                // Found azimuth home, set flag and return to trigger position
                stepper_az.moveTo(stepper_az.currentPosition());
                isHome_az = true;
            }
            if (switch_eleMin.get_state() == true && !isHome_el) {
                // Found elevation home, set flag and return to trigger position
                stepper_el.moveTo(stepper_el.currentPosition());
                isHome_el = true;
            }
            // Traveled beyond step limit... mechanical failure. Exit loop with error
            if ((stepper_az.distanceToGo() == 0 && !isHome_az) ||
                (stepper_el.distanceToGo() == 0 && !isHome_el)) {
                return homing_error;
            }
            // Move motors
            stepper_az.run();
            stepper_el.run();
                
            #ifdef WATCHDOG
                wdt.watchdog_reset();
            #endif
            
            #ifndef DEBUG
                #ifdef ledUseBuiltin
                    // LED heartbeat: fast blink while homing
                    if(millis() - ledTime > ledPeriod/6)   {
                        if(ledState)    {
                            digitalWrite(ledPinBuiltin,LOW);
                            ledState = 0;
                            ledTime = millis();
                        }
                        else{
                            digitalWrite(ledPinBuiltin,HIGH);
                            ledState = 1;
                            ledTime = millis();
                        }
                    }
                #endif
                #ifdef ledUseExternal
                    // LED heartbeat: fast blink while homing
                    if(millis() - ledTime > ledPeriod/6)   {
                        if(ledState)    {
                            digitalWrite(ledPinExternal,LOW);
                            ledState = 0;
                            ledTime = millis();
                        }
                        else{
                            digitalWrite(ledPinExternal,HIGH);
                            ledState = 1;
                            ledTime = millis();
                        }
                    }
                #endif
            #endif
        }
        // Delay to allow homing movement to complete
        time = millis();
        while (millis() - time < HOME_DELAY) {
            #ifdef WATCHDOG
                wdt.watchdog_reset();
            #endif
            stepper_az.run();
            stepper_el.run();
        }
        // Save home positions and rezero setpoints
        stepper_az.setCurrentPosition(0.0);
        stepper_el.setCurrentPosition(0.0);
        control_az.setpoint = 0.0;
        control_el.setpoint = 0.0;
        return no_error;
    }// homing without pol
#else 
// Homing with Polarizer enabled
    enum _rotator_error homing(long seek_aziMin, long seek_eleMin, long seek_polMin) {
        // Declare and init flags
        bool isHome_az = false;
        bool isHome_el = false;
        bool isHome_po = false;
        bool switches_closed = false;
        unsigned long time = 0;

        // If azi endstop is closed, prep for back off
        if(switch_aziMin.get_state())   {
            isHome_az = true;
            switches_closed = true;
            stepper_az.moveTo(-seek_aziMin);
        }
        // If ele endstop is closed, prep for back off
        if(switch_eleMin.get_state())   {
            isHome_el = true;
            switches_closed = true;
            stepper_el.moveTo(-seek_eleMin);
        }
        // If pol endstop is closed, prep for back off
        if(switch_polMin.get_state())   {
            isHome_po = true;
            switches_closed = true;
            stepper_po.moveTo(-seek_polMin);
        }
        // Back off any closed switches
        while(isHome_az || isHome_el || isHome_po)  {
            if (switch_aziMin.get_state() == false && isHome_az) {
                // Switch opened, set flag and stop motor
                stepper_az.stop();
                isHome_az = false;
            }
            if (switch_eleMin.get_state() == false && isHome_el) {
                // Switch opened, set flag and stop motor
                stepper_el.stop();
                isHome_el = false;
            }
            if (switch_polMin.get_state() == false && isHome_po) {
                // Switch opened, set flag and stop motor
                stepper_po.stop();
                isHome_po = false;
            }

            // Move motors
            stepper_az.run();
            stepper_el.run();
            stepper_po.run();

            // Traveled beyond step limit... mechanical failure. Exit loop with error
            if ((stepper_az.distanceToGo() == 0 && isHome_az) ||
                (stepper_el.distanceToGo() == 0 && isHome_el) ||
                (stepper_po.distanceToGo() == 0 && isHome_po)) {
                return homing_error;
            }
                                
            #ifdef WATCHDOG
                wdt.watchdog_reset();
            #endif

            #ifndef DEBUG
                #ifdef ledUseBuiltin
                    // LED heartbeat: fast blink while homing
                    if(millis() - ledTime > ledPeriod/6)   {
                        if(ledState)    {
                            digitalWrite(ledPinBuiltin,LOW);
                            ledState = 0;
                            ledTime = millis();
                        }
                        else{
                            digitalWrite(ledPinBuiltin,HIGH);
                            ledState = 1;
                            ledTime = millis();
                        }
                    }
                #endif
                #ifdef ledUseExternal
                    // LED heartbeat: fast blink while homing
                    if(millis() - ledTime > ledPeriod/6)   {
                        if(ledState)    {
                            digitalWrite(ledPinExternal,LOW);
                            ledState = 0;
                            ledTime = millis();
                        }
                        else{
                            digitalWrite(ledPinExternal,HIGH);
                            ledState = 1;
                            ledTime = millis();
                        }
                    }
                #endif
            #endif
        }

        if(switches_closed)  {
            // Delay to allow switch back off moves to complete
            time = millis();
            while (millis() - time < HOME_DELAY) {
                #ifdef WATCHDOG
                    wdt.watchdog_reset();
                #endif
                stepper_az.run();
                stepper_el.run();
                stepper_po.run();
            }
            switches_closed = false;
        }

        // Endstop switches all open, move motors to "seek" position
        stepper_az.moveTo(seek_aziMin);
        stepper_el.moveTo(seek_eleMin);
        stepper_po.moveTo(seek_polMin);

        // Homing loop
        while (isHome_az == false || isHome_el == false || isHome_po == false) {
            if (switch_aziMin.get_state() == true && !isHome_az) {
                // Found azimuth home, set flag and return to trigger position
                stepper_az.moveTo(stepper_az.currentPosition());
                isHome_az = true;
            }
            if (switch_eleMin.get_state() == true && !isHome_el) {
                // Found elevation home, set flag and return to trigger position
                stepper_el.moveTo(stepper_el.currentPosition());
                isHome_el = true;
            }
            if (switch_polMin.get_state() == true && !isHome_po) {
                // Found elevation home, set flag and return to trigger position
                stepper_po.moveTo(stepper_po.currentPosition());
                isHome_po = true;
            }
            // Traveled beyond step limit... mechanical failure. Exit loop with error
            if ((stepper_az.distanceToGo() == 0 && !isHome_az) ||
                (stepper_el.distanceToGo() == 0 && !isHome_el) ||
                (stepper_po.distanceToGo() == 0 && !isHome_po)) {
                return homing_error;
            }
            // Move motors
            stepper_az.run();
            stepper_el.run();
            stepper_po.run();
                
            #ifdef WATCHDOG
                wdt.watchdog_reset();
            #endif
            
            #ifndef DEBUG
                #ifdef ledUseBuiltin
                    // LED heartbeat: fast blink while homing
                    if(millis() - ledTime > ledPeriod/6)   {
                        if(ledState)    {
                            digitalWrite(ledPinBuiltin,LOW);
                            ledState = 0;
                            ledTime = millis();
                        }
                        else{
                            digitalWrite(ledPinBuiltin,HIGH);
                            ledState = 1;
                            ledTime = millis();
                        }
                    }
                #endif
                #ifdef ledUseExternal
                    // LED heartbeat: fast blink while homing
                    if(millis() - ledTime > ledPeriod/6)   {
                        if(ledState)    {
                            digitalWrite(ledPinExternal,LOW);
                            ledState = 0;
                            ledTime = millis();
                        }
                        else{
                            digitalWrite(ledPinExternal,HIGH);
                            ledState = 1;
                            ledTime = millis();
                        }
                    }
                #endif
            #endif
        }
        // Delay to allow homing movement to complete
        time = millis();
        while (millis() - time < HOME_DELAY) {
            #ifdef WATCHDOG
                wdt.watchdog_reset();
            #endif
            stepper_az.run();
            stepper_el.run();
            stepper_po.run();
        }
        // Save home positions and rezero setpoints
        stepper_az.setCurrentPosition(0.0);
        stepper_el.setCurrentPosition(0.0);
        stepper_po.setCurrentPosition(0.0);
        control_az.setpoint = 0.0;
        control_el.setpoint = 0.0;
        control_po.setpoint = 0.0;
        return no_error;
    }
#endif // homing with pol

// Convert degrees to steps
long deg2step(float deg, float ratio, uint8_t microsteps) {
    long steps = (ratio * SPR * float(microsteps) * deg) / float(360.0);
    return steps;
}

// Convert steps to degrees
float step2deg(long step, float ratio, uint8_t microsteps) {
    float deg = (360.0 * float(step)) / (SPR * ratio * float(microsteps));
    return deg;
}

// SAVED //////////////////////////////////////////////////////////// SAVED //
// Polarizer poti rolling average... not needed with 10nF capacitor.
int readPolPot() {
    int polpotavg = 0;
    for(byte i = 0; i < POL_POT_SAMPLES; i++) {             // this runs POL_POT_SAMPLES-1 loops
        polpotavg += rawpolpot[i];                          // add to average
        if(i < POL_POT_SAMPLES - 1) {
            rawpolpot[i] = rawpolpot[i + 1];                // increment indexes over for next run (rolling array)
        }
    }
    rawpolpot[POL_POT_SAMPLES - 1] = analogRead(polPotPin); // last reading stored
    polpotavg += rawpolpot[POL_POT_SAMPLES - 1];            // ...and added
    return (polpotavg / (POL_POT_SAMPLES + 1));             // return average
}
