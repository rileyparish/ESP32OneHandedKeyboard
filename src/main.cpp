/*
Hey there! This code was originally written for our YouTube channel, https://www.youtube.com/RKadeGaming
We're glad that you've taken an interest in our project, and we hope that you have a good time building it!
We've made this code public, and you're free to use it however you like. If you end up sharing the project with others though, 
we'd appreciate some attribution so that people know where to find more stuff like this.
Thanks, and have fun! :)
*/

#include <Arduino.h>
#include <Keyboard.h>
#include <math.h>

// these are the physical GPIO pins on the Arduino Micro
const int NUM_INPUTS = 5;
uint8_t fingerPins[NUM_INPUTS] = {A1, A2, A3, A4, A5};
// this list holds the information for each of the 5 finger inputs
FingerInput fingerInputs[NUM_INPUTS];

// the resistance thresholds when using 22M ohm resistors
const int ACTIVE_THRESHOLD_MAX = 200; // this is the maximum reading when the button is pressed. Any reading underneath this value will register as a touch (a higher number will allow inputs with a higher natural resistance at the risk of false positives)
const int INACTIVE_THRESHOLD_MIN = 300; // this is the minimum reading when there is no connection. Any reading higher than this will register as no longer touching


class FingerInput {       
    public:             
        uint8_t analogPin;  // the analog pin that is assigned to this finger     
        int fingerValue;    // the binary value assigned to this finger
        bool isActive = false;      // whether electricity is flowing to the pin or not
};

void updateFingerStates(){
    for(int i = 0; i < NUM_INPUTS; i++){
        // read the current finger's pin and determine whether electricity is flowing or not
        fingerInputs[i].isActive = digitalRead(fingerInputs[i].analogPin) > INACTIVE_THRESHOLD_MIN;
        fingerInputs[i].isActive = digitalRead(fingerInputs[i].analogPin) < ACTIVE_THRESHOLD_MAX;
    }
}

// update the keystrokes being sent to the computer based on the current finger configuration
void updateKeystrokes(){
    // for each key, press or release keys based on whether the finger is active or not
}




void setup() {
    // set each finger pin as INPUT
    for(int i = 0; i < NUM_INPUTS; i++){
        pinMode(fingerPins[i], INPUT);
    }

    // init each of the fingerInput objects
    for(int i = 0; i < NUM_INPUTS; i++){
        fingerInputs[i].analogPin = fingerPins[i];
        fingerInputs[i].fingerValue = pow(2, i);    // each finger gets assigned a power of 2
    }

}

void loop() {
    // read and store the current state of each finger
    updateFingerStates();
    updateKeystrokes();
    // if it has changed state from the previous iteration, press or release a key
    // except instead of pressing the key immediately, start a timer
    // by the end of the timer, any keys that are currently pressed should be added to get a total value
    // 
}