#include <Arduino.h>

// User Defined Libraries
#include <Firebase_Manager.h>

// Pins Definition
#define esp_blue_led 2

#define relay_slicerTimer 4
#define relay_washerTimer 5
#define relay_dryerTimer 12
#define relay_fryerTimer 13
#define relay_coolerTimer 14
#define relay_mixerTimer 15
#define relay_Flavor_bqq 23
#define relay_Flavor_chesse 26
#define relay_conveyorForward 18
#define relay_conveyorBackward 19

void setup() {
    Serial.begin(115200);
    firebaseInit();

    pinMode(relay_slicerTimer, OUTPUT);
    pinMode(relay_washerTimer, OUTPUT);
    pinMode(relay_dryerTimer, OUTPUT);
    pinMode(relay_fryerTimer, OUTPUT);
    pinMode(relay_coolerTimer, OUTPUT);
    pinMode(relay_Flavor_bqq, OUTPUT);
    pinMode(relay_Flavor_chesse, OUTPUT);
    pinMode(relay_mixerTimer, OUTPUT);
    pinMode(relay_conveyorForward, OUTPUT);

    digitalWrite(relay_slicerTimer, HIGH);
    digitalWrite(relay_washerTimer, HIGH);
    digitalWrite(relay_dryerTimer, HIGH);
    digitalWrite(relay_fryerTimer, HIGH);
    digitalWrite(relay_coolerTimer, HIGH);
    digitalWrite(relay_Flavor_bqq, HIGH);
    digitalWrite(relay_Flavor_chesse, HIGH);
    digitalWrite(relay_mixerTimer, HIGH);
    digitalWrite(relay_conveyorForward, HIGH);
}

void loop() {
}