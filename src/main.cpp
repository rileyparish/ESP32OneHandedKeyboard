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
};

// since this script takes control of the keyboard, I want to be able to disable it. The start pin must read LOW before the Arduino can control the keyboard.
const int startPin = 13;

// these are the physical GPIO pins on the Arduino Micro
const int NUM_INPUTS = 5;
uint8_t fingerPins[NUM_INPUTS] = {A5, A4, A3, A2, A1};

// the resistance thresholds when using 22M ohm resistors
const int ACTIVE_THRESHOLD_MAX = 200; // this is the maximum reading when the button is pressed. Any reading underneath this value will register as a touch (a higher number will allow inputs with a higher natural resistance at the risk of false positives)
const int INACTIVE_THRESHOLD_MIN = 300; // this is the minimum reading when there is no connection. Any reading higher than this will register as no longer touching

// the amount of time to wait before sending/stopping keyboard signals
// a longer value will reduce responsiveness but be more forgiving of mistakes
const unsigned long IO_DELAY = 100;
// this stores what the last keypress was so I know when the input changes
int previousKeySum = 0;
// this doesn't need to be global, but this way I don't have to initialize a new variable every iteration
int currentKeySum = 0;

char baseLetters[32] = {'0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x','y', 'z', '-', '-', '-', (char)0xB2 /* BKSP */, (char)0x20 /* SPACE */};

// this list holds the information for each of the 5 finger inputs
FingerInput fingerInputs[NUM_INPUTS];

// sums the finger values in a given moment
int getCurrentKeySum(){
    currentKeySum = 0;
    for(int i = 0; i < NUM_INPUTS; i++){
        if(analogRead(fingerInputs[i].analogPin) < ACTIVE_THRESHOLD_MAX){
            currentKeySum += fingerInputs[i].fingerValue;
        }
    }
    return currentKeySum;
}

// converts a summed value into the corresponding character
char convertSumToChar(int sum){
    if(sum > 32 || sum < 0){
        Serial.println("ERROR: provided sum out of range; can't convert.");
        return '*';
    }
    return baseLetters[sum];
}

void setup() {
    Serial.begin(9600);
    // do nothing until I've confirmed that I want to give arduino control of the keyboard
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
        fingerInputs[i].fingerValue = round(pow(2, i));    // each finger gets assigned an increasing power of 2. round() because pow() works with doubles
    }
}

void loop() {
    // continuously check each of the finger pins to detect when a change in configuration occurs
    if(getCurrentKeySum() != previousKeySum){
        // then an event has started to occur. Wait for other inputs to settle (because human actions are very slow for computers)
        delay(IO_DELAY);    // TODO: this causes problems if a key is pressed and released before 100 ms has elapsed
        if(previousKeySum == 0){
            // then no keys are being pressed and we want to press a key
            currentKeySum = getCurrentKeySum();
            Keyboard.press(convertSumToChar(currentKeySum));
            previousKeySum = currentKeySum;
        }else{
            // then we want to release a key
            // this will probably need to change when the keyboard gets more complex
            Keyboard.releaseAll();
            // no keys are being pressed, so reset previousKeySum
            previousKeySum = 0;
        }
    }
}