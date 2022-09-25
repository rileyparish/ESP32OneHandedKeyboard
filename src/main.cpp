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

class FingerInput {       
    public:             
        uint8_t analogPin;  // the analog pin that is assigned to this finger     
        int fingerValue;    // the binary value assigned to this finger
        bool isActive = false;      // whether electricity is flowing to the pin or not
        uint8_t letter;
        bool stateNeedsUpdate = false;   // stores whether the state of this input has changed from the previous iteration
};

// since this script takes control of the keyboard, I want to be able to disable control. The start pin must read LOW before the Arduino can control the keyboard.
const int startPin = 13;

// these are the physical GPIO pins on the Arduino Micro
const int NUM_INPUTS = 5;
uint8_t fingerPins[NUM_INPUTS] = {A1, A2, A3, A4, A5};

uint8_t letters[NUM_INPUTS] = {'a', 'b', 'c', 'd', 'e'};

// the resistance thresholds when using 22M ohm resistors
const int ACTIVE_THRESHOLD_MAX = 200; // this is the maximum reading when the button is pressed. Any reading underneath this value will register as a touch (a higher number will allow inputs with a higher natural resistance at the risk of false positives)
const int INACTIVE_THRESHOLD_MIN = 300; // this is the minimum reading when there is no connection. Any reading higher than this will register as no longer touching


// this list holds the information for each of the 5 finger inputs
FingerInput fingerInputs[NUM_INPUTS];

// read from all of the pins and update the finger objects with the proper information
void updateFingerStates(){
    for(int i = 0; i < NUM_INPUTS; i++){
        // if the current reading of the pin does not match the current "active" state in storage, then mark this input as needing an update
        if(fingerInputs[i].isActive && analogRead(fingerInputs[i].analogPin) > INACTIVE_THRESHOLD_MIN ){
            // this means that a key is being pressed, but the pin has stopped receiving electricity. The input and output are out of sync and need an update.
            fingerInputs[i].isActive = false;
            fingerInputs[i].stateNeedsUpdate = true;
        }
        if(!fingerInputs[i].isActive && analogRead(fingerInputs[i].analogPin) < ACTIVE_THRESHOLD_MAX){
            // this means that no key is currently being pressed, but the pin has started receiving electricity. The input and output are out of sync and need an update.
            fingerInputs[i].isActive = true;
            fingerInputs[i].stateNeedsUpdate = true;
        }
    }
}

// update the keystrokes being sent to the computer based on the current finger configuration
void updateKeystrokes(){
    // for each key, press or release keys based on whether the finger is active or not
    for(int i = 0; i < NUM_INPUTS; i++){
        // if the state of this input hasn't changed from the previous iteration, it doesn't need an update.
        if(fingerInputs[i].stateNeedsUpdate){
            if(fingerInputs[i].isActive){
                Keyboard.press(fingerInputs[i].letter);
            }else{
                Keyboard.release(fingerInputs[i].letter);
            }
            // now the state of the pin and the state of the keyboard output are correctly synced. No updates are needed for this input
            fingerInputs[i].stateNeedsUpdate = false;
        }
        
    }
}

void setup() {
    Serial.begin(9600);
    // do nothing until I've confirmed that I want to give the board control of the keyboard
    pinMode(startPin, INPUT_PULLUP);
    while(digitalRead(startPin) == HIGH){
        Serial.println("Waiting for startPin before continuing...");
        delay(500);
    }

    // set each finger pin as INPUT
    for(int i = 0; i < NUM_INPUTS; i++){
        pinMode(fingerPins[i], INPUT);
    }

    // init each of the fingerInput objects
    for(int i = 0; i < NUM_INPUTS; i++){
        fingerInputs[i].analogPin = fingerPins[i];
        fingerInputs[i].fingerValue = pow(2, i);    // each finger gets assigned an increasing power of 2
        fingerInputs[i].letter = letters[i];
    }

}

void loop() {
    // read and store the current state of each finger
    updateFingerStates();
    updateKeystrokes();
    // if it has changed state from the previous iteration, press or release a key
    // except instead of pressing the key immediately, start a timer
    // by the end of the timer, any keys that are currently pressed should be added to get a total value
}