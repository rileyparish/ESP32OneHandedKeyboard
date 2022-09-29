/*
Hey there! This code was originally written for our YouTube channel, https://www.youtube.com/RKadeGaming
We're glad that you've taken an interest in our project, and we hope that you have a good time building it!
We've made this code public, and you're free to use it however you like. If you end up sharing the project with others though, 
we'd appreciate some attribution so that people know where to find more stuff like this.
Thanks, and have fun! :)
*/

#include <Arduino.h>
#include <BleKeyboard.h>
#include <math.h>

class FingerInput {       
    public:             
        uint8_t capacitivePin;  // the GPIO pin that is assigned to this finger     
        int fingerValue;    // the binary value assigned to this finger
};
// this is the bluetooth keyboard
BleKeyboard bleKeyboard;

// since this script takes control of the keyboard, I want to be able to disable it. The start pin must read LOW before the ESP32 can control the keyboard.
const int START_PIN = 26;       // D26 as labeled on the board

// these are the physical GPIO pins on the ESP32
const int NUM_INPUTS = 5;
uint8_t fingerPins[NUM_INPUTS] = {T5, T6, T7, T8, T9};  // corresponds to pins D12, D14, D27, D32, and D33 respectively

const int ACTIVE_THRESHOLD = 25; // this is the maximum reading when the button is pressed. Any reading below this value will register as a touch.

// the amount of time to wait before sending/stopping keyboard signals
// a longer value will reduce responsiveness but will be more forgiving of imperfectly synced actions
const unsigned long IO_DELAY = 100;
// this stores what the last keypress was so I know when the input changes
int previousKeySum = 0;
// this doesn't need to be global, but this way I don't have to initialize a new variable every iteration
int currentKeySum = 0;

char baseLetters[32] = {'*', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x','y', 'z', 
    '-' /*Alt letter mode*/, 
    '-' /* Special keys mode */, 
    '-', 
    (char)0xB2 /* BKSP */, 
    (char)0x20 /* SPACE */
};
char altSymbols[32] = {'*', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '{', '}', ';', ':', '.', '?', 
    '-' /*Alt letter mode*/, 
    '-' /* Special keys mode */, 
    '-', 
    (char)0xB2 /* BKSP */, 
    (char)0x20 /* SPACE */
};   // still missing <>/\[]"'-=_+|
bool altSymbolMode = false;     // whether alt mode is enabled or not
const int ALT_SYMBOL_MODE = 27;

// this list holds the information for each of the 5 finger inputs
FingerInput fingerInputs[NUM_INPUTS];

// sums the finger values in a given moment
int getCurrentKeySum(){
    currentKeySum = 0;
    for(int i = 0; i < NUM_INPUTS; i++){
        if(touchRead(fingerInputs[i].capacitivePin) < ACTIVE_THRESHOLD){
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
    // if alt symbol mode is enabled, choose from the altSymbols array
    if(altSymbolMode){
        return altSymbols[sum];
    }
    return baseLetters[sum];
}

void setup() {
    Serial.begin(115200);
    bleKeyboard.begin();
    // don't proceed until I've confirmed that I want to give ESP32 control of the keyboard
    pinMode(START_PIN, INPUT_PULLUP);
    while(digitalRead(START_PIN) == HIGH){
        Serial.println("Waiting for START_PIN before continuing...");
        delay(500);
    }

    // set each finger pin as INPUT
    for(int i = 0; i < NUM_INPUTS; i++){
        pinMode(fingerPins[i], INPUT);
    }

    // init each of the fingerInput objects
    for(int i = 0; i < NUM_INPUTS; i++){
        fingerInputs[i].capacitivePin = fingerPins[i];
        fingerInputs[i].fingerValue = round(pow(2, i));    // each finger gets assigned an increasing power of 2. round() because pow() works with doubles
    }
}

void loop() {
    // no need to do anything if the keyboard is not currently connected to a device
    if(bleKeyboard.isConnected()){
        // continuously check each of the finger pins to detect when a change in configuration occurs
        if(getCurrentKeySum() != previousKeySum){
            // an event has started to occur. Wait for other inputs to settle (because human actions are very slow for computers)
            delay(IO_DELAY);    // TODO: no touch will register if a key is pressed and released within IO_DELAY. Could use millis() instead.
            currentKeySum = getCurrentKeySum();
            // first determine if the combination is activating a different keyboard mode
            if(currentKeySum == ALT_SYMBOL_MODE){
                // toggle alt symbol mode
                altSymbolMode = !altSymbolMode;
            }else{
                // if a special mode was not provided, process the input as a keystroke
                if(previousKeySum == 0 && currentKeySum != 0){
                    // then no keys are currently being pressed and we want to press a key
                    bleKeyboard.press(convertSumToChar(currentKeySum));
                    previousKeySum = currentKeySum;
                }else{
                    // if previousKeySum is set, then we want to release whatever was being pressed
                    // this will probably need to change when the keyboard gets more complex
                    bleKeyboard.releaseAll();
                    // no keys are being pressed anymore, so reset previousKeySum
                    previousKeySum = 0;
                }
            }
        }
    }
}