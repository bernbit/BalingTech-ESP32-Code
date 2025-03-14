#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <WiFi.h>

#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"

const char* ssid = "PLDTHOMEFIBRbaa40";
const char* password = "PLDTWIFI9xq6z";

#define DATABASE_URL "https://balingtech-450a3-default-rtdb.firebaseio.com/"
#define API_KEY "AIzaSyBcK1n4597MRI6-8SP5KV6EqgEq-AeOlZA"
#define USER_EMAIL "balingtech@esp.com";
#define USER_PASSWORD "balingtech_2025";

FirebaseData fbdo, fbdo_s8, fbdo_s9, fbdo_s10, fbdo_s11, fbdo_s12, fbdo_s13, fbdo_s14, fbdo_s15, fbdo_s16;
FirebaseAuth auth;
FirebaseConfig config;
String uid;
bool login = false;

unsigned long previousMillis = 0;
unsigned long sendDataPrevMillis = 0;

int processTimers[7] = {0, 0, 0, 0, 0, 0, 0};  // Array to store timers [Slicing, Washing, Drying, Frying, Cooling]
long countdownTime = 0;
int currentProcessIndex = 0;
bool processRunning = false;
String currentProcess = "";
bool startOperation = false;

bool isPaused = false;  // Global pause state
// Relay control pins
const int relay_slicerTimer = 4;
const int relay_washerTimer = 5;
const int relay_dryerTimer = 12;
const int relay_fryerTimer = 13;
const int relay_coolerTimer = 14;
const int relay_mixerTimer = 15;
const int relay_Flavor_bqq = 23;
const int relay_Flavor_chesse = 26;
const int relay_conveyorForward = 18;
const int relay_conveyorBackward = 19;
String flavorType = "";  // Global variable to store flavor type
bool signupOK = false;
bool manualMode = false;

bool slicerStatus = false;
bool washStatus = false;
bool dryerStatus = false;
bool cookStatus = false;
bool fryerStatus = false;
bool coolStatus = false;
bool mixerStatus = false;
bool Flavor_bqqStatus = false;
bool Flavor_chesseStatus = false;
bool conveyorForward = false;
bool conveyorBackward = false;
bool Flavoring = false;

// Function prototypes
void setupFirebase();
void fetchTimers();
void handleProcesses();
void startProcess();
void updateFirebase(String path, String value);
void resetFirebaseValues();
void startSlicing();
void startWashing();
void startDrying();
void startFrying();
void startCooling();
void countdownProcess();

void startProcess();
void startSlicing();
void startWashing();
void startDrying();
void startFrying();
void startCooling();
void startFlavoring();
void startMixer();
void countdownProcess();
void deactivateRelays();
void updateFirebaseInt(String path, int value);
void checkForRestart();
void manual();

// Paths
String startOperationPath = "/startOperation";
String pauseOperationPath = "/pauseOperation";
String operationsDurationPath = "/operationsDuration";
String remainingTimePath = "/remainingTime";
String currentOperationPath = "/currentOperation";
String flavorGramsPath = "/flavorGrams";
String flavorTypPath = "/flavorType";
String manualModePath = "/manualMode";
String forwardPath = "/forward";
String backwardPath = "/backward";

void setup() {
    Serial.begin(115200);
    // Setup relay control pins
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

       Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
    resetFirebaseValues();
    setupFirebase();
    fetchTimers();  // Fetch values from Firebase and store them in the array
}

void loop() {
    handleProcesses();
    checkForRestart();

    if (Firebase.RTDB.getBool(&fbdo, pauseOperationPath)) {
        isPaused = fbdo.boolData();
        Serial.printf("Pause State: %s\n", isPaused ? "Paused" : "pauseOperation");
    }

    if (manualMode) {
        isPaused = true;
        manual();
    }
}
void manual() {
    if (Firebase.ready() && signupOK) {
        if (!Firebase.RTDB.readStream(&fbdo_s8))
            Serial.printf("Stream 8 read Error, %s\n\n", fbdo_s8.errorReason().c_str());

        if (fbdo_s8.streamAvailable()) {
            if (fbdo_s8.dataType() == "boolean") {
                slicerStatus = fbdo_s8.boolData();
                digitalWrite(relay_slicerTimer, !slicerStatus);
            }
        }

        if (!Firebase.RTDB.readStream(&fbdo_s9))
            Serial.printf("Stream 9 read Error, %s\n\n", fbdo_s9.errorReason().c_str());

        if (fbdo_s9.streamAvailable()) {
            if (fbdo_s9.dataType() == "boolean") {
                washStatus = fbdo_s9.boolData();
                digitalWrite(relay_washerTimer, !washStatus);
            }
        }
        if (!Firebase.RTDB.readStream(&fbdo_s10))
            Serial.printf("Stream 10 read Error, %s\n\n", fbdo_s10.errorReason().c_str());

        if (fbdo_s10.streamAvailable()) {
            if (fbdo_s10.dataType() == "boolean") {
                dryerStatus = fbdo_s10.boolData();
                digitalWrite(relay_dryerTimer, !dryerStatus);
            }
        }
        if (!Firebase.RTDB.readStream(&fbdo_s11))
            Serial.printf("Stream 11 read Error, %s\n\n", fbdo_s11.errorReason().c_str());

        if (fbdo_s11.streamAvailable()) {
            if (fbdo_s11.dataType() == "boolean") {
                fryerStatus = fbdo_s11.boolData();
                digitalWrite(relay_fryerTimer, !fryerStatus);
            }
        }
        if (!Firebase.RTDB.readStream(&fbdo_s12))
            Serial.printf("Stream 12 read Error, %s\n\n", fbdo_s12.errorReason().c_str());

        if (fbdo_s12.streamAvailable()) {
            if (fbdo_s12.dataType() == "boolean") {
                coolStatus = fbdo_s12.boolData();
                digitalWrite(relay_coolerTimer, !coolStatus);
            }
        }
        if (!Firebase.RTDB.readStream(&fbdo_s13))
            Serial.printf("Stream 13 read Error, %s\n\n", fbdo_s13.errorReason().c_str());

        if (fbdo_s13.streamAvailable()) {
            if (fbdo_s13.dataType() == "boolean") {
                mixerStatus = fbdo_s13.boolData();
                digitalWrite(relay_mixerTimer, !mixerStatus);
            }
        }
        if (!Firebase.RTDB.readStream(&fbdo_s14))
            Serial.printf("Stream 14 read Error, %s\n\n", fbdo_s14.errorReason().c_str());

        if (fbdo_s14.streamAvailable()) {
            if (fbdo_s14.dataType() == "boolean") {
                Flavoring = fbdo_s14.boolData();
                digitalWrite(relay_Flavor_bqq, !Flavoring);
            }
        }
        //   if (!Firebase.RTDB.readStream(&fbdo_s14))
        //   Serial.printf("Stream 14 read Error, %s\n\n", fbdo_s14.errorReason().c_str());

        // if (fbdo_s14.streamAvailable()) {
        //   if (fbdo_s14.dataType() == "boolean") {
        //     Flavoring = fbdo_s14.boolData();
        //     digitalWrite(relay_Flavor_bqq, Flavoring);
        //   }
        // }
        if (!Firebase.RTDB.readStream(&fbdo_s15))
            Serial.printf("Stream 15 read Error, %s\n\n", fbdo_s15.errorReason().c_str());

        if (fbdo_s15.streamAvailable()) {
            if (fbdo_s15.dataType() == "boolean") {
                conveyorForward = fbdo_s15.boolData();
                digitalWrite(relay_conveyorForward, !conveyorForward);
            }
        }
        if (!Firebase.RTDB.readStream(&fbdo_s16))
            Serial.printf("Stream 16 read Error, %s\n\n", fbdo_s11.errorReason().c_str());

        if (fbdo_s16.streamAvailable()) {
            if (fbdo_s16.dataType() == "boolean") {
                conveyorBackward = fbdo_s16.boolData();
                digitalWrite(relay_conveyorBackward, !conveyorBackward);
            }
        }
    }
}
void setupFirebase() {
    // Assign Value to Firebase Objects
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback;
    // Assign the maximum retry of token generation
    config.max_token_generation_retry = 5;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    Serial.println("Getting User UID");
    while ((auth.token.uid) == "") {
        Serial.print('.');
        delay(1000);
    }
    // Print user UID
    uid = auth.token.uid.c_str();
    Serial.print("User UID: ");
    Serial.println(uid);

    if (auth.token.uid != "") {
        login = true;
    }

    Serial.print("Authetication Status: ");
    Serial.print(login);
}

// Fetch timers from Firebase once and store them in the array
void fetchTimers() {
    if (Firebase.RTDB.getInt(&fbdo, "operationsDuration/Slicing")) {
        processTimers[0] = fbdo.intData();
    }
    if (Firebase.RTDB.getInt(&fbdo, "operationsDuration/Washing")) {
        processTimers[1] = fbdo.intData();
    }
    if (Firebase.RTDB.getInt(&fbdo, "operationsDuration/Drying")) {
        processTimers[2] = fbdo.intData();
    }
    if (Firebase.RTDB.getInt(&fbdo, "operationsDuration/Frying")) {
        processTimers[3] = fbdo.intData();
    }
    if (Firebase.RTDB.getInt(&fbdo, "operationsDuration/Cooling")) {
        processTimers[4] = fbdo.intData();
    }
    if (Firebase.RTDB.getInt(&fbdo, "operationsDuration/Flavoring")) {
        processTimers[5] = fbdo.intData();
    }
    if (Firebase.RTDB.getInt(&fbdo, "operationsDuration/Mixing")) {
        processTimers[6] = fbdo.intData();
    }

    // Fetching the flavor type from Firebase
    if (Firebase.RTDB.getString(&fbdo, "/flavorType")) {
        flavorType = fbdo.to<String>();  // Store the flavor type
        Serial.printf("Flavor type: %s\n", flavorType.c_str());
    } else {
        Serial.println("Failed to get flavor type from Firebase");
        flavorType = "Unknown";  // Default value in case of failure
    }

    Serial.println("Timers loaded into array:");
    for (int i = 0; i < 5; i++) {
        Serial.printf("Process %d Timer: %d minutes\n", i, processTimers[i]);
    }
}

void handleProcesses() {
    if (!processRunning) {
        startProcess();
    } else {
        countdownProcess();
    }
}

void startProcess() {
    switch (currentProcessIndex) {
        case 0:
            startSlicing();
            break;
        case 1:
            startWashing();
            break;
        case 2:
            startDrying();
            break;
        case 3:
            startFrying();
            break;
        case 4:
            startCooling();
            break;
        case 5:
            startFlavoring();
            break;
        case 6:
            startMixer();
            break;
        default:
            Serial.println("All processes completed.");
            break;
    }
}

void startSlicing() {
    countdownTime = processTimers[0];
    if (countdownTime > 0) {
        processRunning = true;
        currentProcess = "Slicing";

        digitalWrite(relay_slicerTimer, LOW);
        Serial.printf("Starting Slicing for %d minutes\n", processTimers[0]);
        updateFirebase(currentOperationPath, currentProcess);
    }
}

void startWashing() {
    countdownTime = processTimers[1];
    if (countdownTime > 0) {
        processRunning = true;
        currentProcess = "Washing";
        digitalWrite(relay_washerTimer, LOW);
        Serial.printf("Starting Washing for %d minutes\n", processTimers[1]);
        updateFirebase(currentOperationPath, currentProcess);
    }
}

void startDrying() {
    digitalWrite(relay_conveyorForward, LOW);
    delay(3000);
    deactivateRelays();
    countdownTime = processTimers[2];
    if (countdownTime > 0) {
        processRunning = true;
        currentProcess = "Drying";
        digitalWrite(relay_dryerTimer, LOW);
        Serial.printf("Starting Drying for %d minutes\n", processTimers[2]);
        updateFirebase(currentOperationPath, currentProcess);
    }
}

void startFrying() {
    digitalWrite(relay_conveyorForward, LOW);
    delay(3000);
    deactivateRelays();
    countdownTime = processTimers[3];
    if (countdownTime > 0) {
        processRunning = true;
        currentProcess = "Frying";
        digitalWrite(relay_fryerTimer, LOW);
        Serial.printf("Starting Frying for %d minutes\n", processTimers[3]);
        updateFirebase(currentOperationPath, currentProcess);
    }
}

void startCooling() {
    digitalWrite(relay_conveyorForward, LOW);
    delay(3000);
    deactivateRelays();
    countdownTime = processTimers[4];
    if (countdownTime > 0) {
        processRunning = true;
        currentProcess = "Cooling";
        digitalWrite(relay_coolerTimer, LOW);
        Serial.printf("Starting Cooling for %d minutes\n", processTimers[4]);
        updateFirebase(currentOperationPath, currentProcess);
    }
}

void startFlavoring() {
    // Start the conveyor
    digitalWrite(relay_conveyorForward, LOW);
    delay(3000);
    deactivateRelays();
    countdownTime = processTimers[5];
    if (countdownTime > 0) {
        processRunning = true;
        currentProcess = "Flavoring";

        if (flavorType == "bqq") {
            digitalWrite(relay_Flavor_bqq, LOW);
            digitalWrite(relay_Flavor_chesse, HIGH);
        } else if (flavorType == "chesse") {
            digitalWrite(relay_Flavor_bqq, HIGH);
            digitalWrite(relay_Flavor_chesse, LOW);
        } else {
            // Default: Turn off both if `flavorType` is unknown
            digitalWrite(relay_Flavor_bqq, HIGH);
            digitalWrite(relay_Flavor_chesse, HIGH);
        }
        Serial.printf("Starting Flavoring for %d minutes\n", processTimers[5]);
        updateFirebase(currentOperationPath, currentProcess);
    }
}

void startMixer() {
    digitalWrite(relay_conveyorForward, LOW);
    delay(3000);
    deactivateRelays();
    countdownTime = processTimers[6];
    if (countdownTime > 0) {
        processRunning = true;
        currentProcess = "Mixing";
        digitalWrite(relay_mixerTimer, LOW);
        Serial.printf("Starting Cooling for %d minutes\n", processTimers[6]);
        updateFirebase(currentOperationPath, currentProcess);
    }
}

void countdownProcess() {
    if (isPaused) {
        Serial.println("Timer paused....Manual is activated!");

        return;
    }
    if (countdownTime > 0) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= 1000) {
            previousMillis = currentMillis;
            countdownTime -= 1000;

            int remainingMinutes = countdownTime / 60000;
            int remainingSeconds = (countdownTime % 60000) / 1000;
            Serial.printf("Time left: %d minutes %d seconds\n", remainingMinutes, remainingSeconds);

            if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
                sendDataPrevMillis = millis();
                String timerMessage = "Time left: " + String(remainingMinutes) + " minutes " + String(remainingSeconds) + " seconds";

                String operationDurationPath = "operationsDuration/" + currentProcess;
                updateFirebaseInt(operationDurationPath, countdownTime);
            }

            if (countdownTime <= 0) {
                processRunning = false;
                currentProcessIndex++;
                deactivateRelays();
                Serial.println("Process finished, moving to the next...");
            }
        }
    }
}
void deactivateRelays() {
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
void updateFirebase(String path, String value) {
    if (!Firebase.RTDB.setString(&fbdo, path.c_str(), value)) {
        Serial.println("Error updating Firebase: " + fbdo.errorReason());
    }
}

void updateFirebaseInt(String path, int value) {
    if (!Firebase.RTDB.setInt(&fbdo, path.c_str(), value)) {
        Serial.println("Error updating Firebase: " + fbdo.errorReason());
    }
}

void resetFirebaseValues() {
    for (int i = 0; i < 5; i++) {
        processTimers[i] = 0;
    }

    updateFirebaseInt("operationsDuration/Slicing", 0);
    updateFirebaseInt("operationsDuration/Washing", 0);
    updateFirebaseInt("operationsDuration/Drying", 0);
    updateFirebaseInt("operationsDuration/Frying", 0);
    updateFirebaseInt("operationsDuration/Cooling", 0);
    updateFirebase("operationsDuration/RemainingTime", "Set a Timer");
    updateFirebase("operationsDuration/CurrentProcess", "No process running");
}

void checkForRestart() {
    if (Firebase.RTDB.getInt(&fbdo, "/startOperation")) {
        startOperation = fbdo.intData();
        if (startOperation) {
            Serial.println("Restart signal received. Resetting process...");
            currentProcessIndex = 0;
            processRunning = false;
            fetchTimers();
            Firebase.RTDB.setInt(&fbdo, "/startOperation", false);
        }
    }
}