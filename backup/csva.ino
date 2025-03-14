#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Wi-Fi and Firebase configuration
const char* ssid = "PLDTHOMEFIBRbaa40";
const char* password = "PLDTWIFI9xq6z";
#define DATABASE_URL "https://balingtech-450a3-default-rtdb.firebaseio.com/"
#define API_KEY "AIzaSyBcK1n4597MRI6-8SP5KV6EqgEq-AeOlZA"
bool paused = false;  // Flag to control pause/resume
bool triggerRestart = false;
HardwareSerial sim900aSerial(2);
String phoneNumber = "+639151954684";
String message = "Process complete: Your cassava chips have been successfully prepared. The system has completed all stages. Thank you for using BalingTech!";

void sendSMS(String phoneNumber, String message) {
  Serial.println("Sending SMS...");
  sendATCommand("AT+CMGF=1");
  String command = "AT+CMGS=\"" + phoneNumber + "\"";
  sendATCommand(command);
  sendATCommand(message);
  sim900aSerial.write(0x1A);
  delay(1000);
  Serial.println("SMS Sent!");
}

void sendATCommand(String command) {
  sim900aSerial.println(command);

  delay(500);

  while (sim900aSerial.available()) {
    Serial.write(sim900aSerial.read());
  }

  Serial.println();
}

// Firebase objects
FirebaseData fbdo, fbdo_s1, fbdo_s2, fbdo_s3, fbdo_s4, fbdo_s5, fbdo_s6,fbdo_s7;
FirebaseAuth auth;
FirebaseConfig config;
int timers[6] = {0};
// Timer variables
int slicerTimer = 0, washerTimer = 0, dryerTimer = 0, fryerTimer = 0, coolerTimer = 0, flavoring = 0, mixerTimer = 0;
long countdownTime = 0;
int currentProcessIndex = 0;  
bool processRunning = false;
String currentProcess = "";

// Time tracking
unsigned long previousMillis = 0;
unsigned long sendDataPrevMillis = 0;

// Relay control pins
const int relay_slicerTimer = 4;
const int relay_washerTimer = 5;
const int relay_dryerTimer = 12;
const int relay_fryerTimer = 13;
const int relay_coolerTimer = 14;
const int relay_mixerTimer = 15;
const int relay_Flavor_bqq = 23;
const int relay_Flavor_chesse = 26;
// const int relay_Flavor_chesse = 27;

const int relay_conveyorForward = 18;
const int relay_conveyorBackward = 19;
String flavorType = "";  // Global variable to store flavor type
// Forward declarations of functions
void setupFirebase();
void initializeStreams();
void handleProcesses();
void startProcess();
void countdownProcess();
void resetFirebaseValues();
void updateFirebase(String path, int value);
void updateFirebase(String path, String value);
void startSlicing();
void startWashing();
void startDrying();
void startFrying();
void startCooling();
void startMixer();
void startFlavoring();
void getTimersFromDatabase(); 

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

  // Setup serial communication
 
  Serial.printf("Connecting to %s ", ssid);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  // Setup Firebase
  setupFirebase();
getTimersFromDatabase();
  // Initialize Firebase streams
  initializeStreams();
}

void getTimersFromDatabase() {
    if (Firebase.RTDB.getInt(&fbdo, "/operationsDuration/Slicing")) {
        timers[0] = fbdo.to<int>();
    }
    if (Firebase.RTDB.getInt(&fbdo, "/operationsDuration/Washing")) {
        timers[1] = fbdo.to<int>();
    }
    if (Firebase.RTDB.getInt(&fbdo, "/operationsDuration/Drying")) {
        timers[2] = fbdo.to<int>();
    }
    if (Firebase.RTDB.getInt(&fbdo, "/operationsDuration/Frying")) {
        timers[3] = fbdo.to<int>();
    }
    if (Firebase.RTDB.getInt(&fbdo, "/operationsDuration/Cooling")) {
        timers[4] = fbdo.to<int>();
    }
    if (Firebase.RTDB.getInt(&fbdo, "/operationsDuration/Flavoring")) {
        timers[5] = fbdo.to<int>();
    }
    if (Firebase.RTDB.getInt(&fbdo, "/operationsDuration/Mixing")) {
        timers[6] = fbdo.to<int>();
    }
    
    // Fetching the flavor type from Firebase
    if (Firebase.RTDB.getString(&fbdo, "/flavor")) {
        flavorType = fbdo.to<String>();  // Store the flavor type
        Serial.printf("Flavor type: %s\n", flavorType.c_str());
    } else {
        Serial.println("Failed to get flavor type from Firebase");
        flavorType = "Unknown";  // Default value in case of failure
    }
} 

void loop() {
  // Continuously handle the process lifecycle
   checkForRestart();
  handleProcesses();
}


// Firebase setup function
void setupFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase SignUp OK!");
  } else {
    Serial.printf("Firebase sign up error: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// Initialize Firebase streams
void initializeStreams() {
  if (!Firebase.RTDB.beginStream(&fbdo_s1, "operationsDuration/Slicing"))
    Serial.printf("Stream 1 error: %s\n", fbdo_s1.errorReason().c_str());

  if (!Firebase.RTDB.beginStream(&fbdo_s2, "operationsDuration/Washing"))
    Serial.printf("Stream 2 error: %s\n", fbdo_s2.errorReason().c_str());

  if (!Firebase.RTDB.beginStream(&fbdo_s3, "operationsDuration/Drying"))
    Serial.printf("Stream 3 error: %s\n", fbdo_s3.errorReason().c_str());

  if (!Firebase.RTDB.beginStream(&fbdo_s4, "operationsDuration/Frying"))
    Serial.printf("Stream 4 error: %s\n", fbdo_s4.errorReason().c_str());

  if (!Firebase.RTDB.beginStream(&fbdo_s5, "operationsDuration/Cooling"))
    Serial.printf("Stream 5 error: %s\n", fbdo_s5.errorReason().c_str());

 if (!Firebase.RTDB.beginStream(&fbdo_s6, "operationsDuration/Flavoring"))
    Serial.printf("Stream 6 error: %s\n", fbdo_s6.errorReason().c_str());
    
 if (!Firebase.RTDB.beginStream(&fbdo_s7, "operationsDuration/Mixing"))
    Serial.printf("Stream 7 error: %s\n", fbdo_s7.errorReason().c_str());
}

// Main function to handle process flow
void handleProcesses() {
  if (!processRunning) {
    startProcess();
  } else {
    countdownProcess();
  }
}

// Start process based on current index
void startProcess() {
  switch (currentProcessIndex) {
    case 0: startSlicing(); break;
    case 1: startWashing(); break;
    case 2: startDrying(); break;
    case 3: startFrying(); break;
    case 4: startCooling(); break;
    case 5: startFlavoring(); break;
    case 6: startMixer(); break; 
    default:
      Serial.println("All processes completed.");
      
  digitalWrite(relay_slicerTimer, HIGH);
  digitalWrite(relay_washerTimer, HIGH);
  digitalWrite(relay_dryerTimer, HIGH);
  digitalWrite(relay_fryerTimer, HIGH);
  digitalWrite(relay_coolerTimer, HIGH);
  digitalWrite(relay_Flavor_bqq, HIGH);
  digitalWrite(relay_Flavor_chesse, HIGH);
  digitalWrite(relay_mixerTimer, HIGH);
  digitalWrite(relay_conveyorForward, HIGH);
      resetFirebaseValues();
      break;
  }
}

// Countdown function to manage the active process
void countdownProcess() {
  if (!paused && countdownTime > 0) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {  // 1-second interval
      previousMillis = currentMillis;
      countdownTime -= 1000;

      int remainingMinutes = countdownTime / 60000;
      int remainingSeconds = (countdownTime % 60000) / 1000;
      Serial.printf("Time left: %d minutes %d seconds\n", remainingMinutes, remainingSeconds);

      // Update remaining time in Firebase every second
      if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();
        String timerMessage = "Time left: " + String(remainingMinutes) + " minutes " + String(remainingSeconds) + " seconds";
        updateFirebase("Timer/RemainingTime", timerMessage);
      }

      if (countdownTime <= 0) {
        processRunning = false;
        currentProcessIndex++;
        Serial.println("Process finished, moving to the next...");
      }
    }
  }
}

// Function to toggle pause/resume
void togglePause() {
  paused = !paused;
  Serial.println(paused ? "Timer Paused" : "Timer Resumed");
}
// Functions to handle each specific process
void startSlicing() {
    countdownTime = timers[0] * 60000;
    processRunning = true;
    currentProcess = "Slicing";

  digitalWrite(relay_slicerTimer, LOW);
  digitalWrite(relay_washerTimer, HIGH);
  digitalWrite(relay_dryerTimer, HIGH);
  digitalWrite(relay_fryerTimer, HIGH);
  digitalWrite(relay_coolerTimer, HIGH);
  digitalWrite(relay_Flavor_bqq, HIGH);
  digitalWrite(relay_Flavor_chesse, HIGH);
  digitalWrite(relay_mixerTimer, HIGH);
  digitalWrite(relay_conveyorForward, HIGH);

    Serial.printf("Starting Slicing for %d minutes\n", timers[0]);
    updateFirebase("Timer/CurrentProcess", currentProcess);
}

void startWashing() {
   digitalWrite(relay_conveyorForward, LOW);
   delay(3000);
   digitalWrite(relay_conveyorForward, HIGH);
    countdownTime = timers[1] * 60000;
    processRunning = true;
    currentProcess = "Washing";
    digitalWrite(relay_slicerTimer, HIGH);
    digitalWrite(relay_washerTimer, LOW);
    digitalWrite(relay_dryerTimer, HIGH);
    digitalWrite(relay_fryerTimer, HIGH);
    digitalWrite(relay_coolerTimer, HIGH);
  digitalWrite(relay_Flavor_bqq, HIGH);
  digitalWrite(relay_Flavor_chesse, HIGH);
    digitalWrite(relay_mixerTimer, HIGH);

    Serial.printf("Starting Washing for %d minutes\n", timers[1]);
    updateFirebase("Timer/CurrentProcess", currentProcess);
}

void startDrying() {
  
   digitalWrite(relay_conveyorForward, LOW);
   delay(3000);
   digitalWrite(relay_conveyorForward, HIGH);

    countdownTime = timers[2] * 60000;
    processRunning = true;
    currentProcess = "Drying";

    digitalWrite(relay_slicerTimer, HIGH);
    digitalWrite(relay_washerTimer, HIGH);
    digitalWrite(relay_dryerTimer, LOW);
    digitalWrite(relay_fryerTimer, HIGH);
    digitalWrite(relay_coolerTimer, HIGH);
  digitalWrite(relay_Flavor_bqq, HIGH);
  digitalWrite(relay_Flavor_chesse, HIGH);
    digitalWrite(relay_mixerTimer, HIGH);

    Serial.printf("Starting Drying for %d minutes\n", timers[2]);
    updateFirebase("Timer/CurrentProcess", currentProcess);
}

void startFrying() {
  
   digitalWrite(relay_conveyorForward, LOW);
   delay(3000);
   digitalWrite(relay_conveyorForward, HIGH);

    countdownTime = timers[3] * 60000;
    processRunning = true;
    currentProcess = "Frying";

    digitalWrite(relay_slicerTimer, HIGH);
    digitalWrite(relay_washerTimer, HIGH);
    digitalWrite(relay_dryerTimer, HIGH);
    digitalWrite(relay_fryerTimer, LOW);
    digitalWrite(relay_coolerTimer, HIGH);
  digitalWrite(relay_Flavor_bqq, HIGH);
  digitalWrite(relay_Flavor_chesse, HIGH);
    digitalWrite(relay_mixerTimer, HIGH);

    Serial.printf("Starting Frying for %d minutes\n", timers[3]);
    updateFirebase("Timer/CurrentProcess", currentProcess);
}

void startCooling() {
  
   digitalWrite(relay_conveyorForward, LOW);
   delay(3000);
   digitalWrite(relay_conveyorForward, HIGH);

    countdownTime = timers[4] * 60000;
    processRunning = true;
    currentProcess = "Cooling";

    digitalWrite(relay_slicerTimer, HIGH);
    digitalWrite(relay_washerTimer, HIGH);
    digitalWrite(relay_dryerTimer, HIGH);
    digitalWrite(relay_fryerTimer, HIGH);
    digitalWrite(relay_coolerTimer, LOW);
  digitalWrite(relay_Flavor_bqq, HIGH);
  digitalWrite(relay_Flavor_chesse, HIGH);
    digitalWrite(relay_mixerTimer, HIGH);

    Serial.printf("Starting Cooling for %d minutes\n", timers[4]);
    updateFirebase("Timer/CurrentProcess", currentProcess);
}
void startFlavoring() {  


    // Start the conveyor
    digitalWrite(relay_conveyorForward, LOW);
    delay(3000);
    digitalWrite(relay_conveyorForward, HIGH);
    // Activate the relay based on the fetched global `flavorType`
  
    // Set process timer
    countdownTime = timers[5] * 60000;
    processRunning = true;
    currentProcess = "Mixing";
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
    // Activate necessary timers
    digitalWrite(relay_slicerTimer, HIGH);
    digitalWrite(relay_washerTimer, HIGH);
    digitalWrite(relay_dryerTimer, HIGH);
    digitalWrite(relay_fryerTimer, HIGH);
    digitalWrite(relay_coolerTimer, HIGH);
    digitalWrite(relay_mixerTimer, HIGH);

    Serial.printf("Starting Mixing for %d minutes\n", timers[5]);
    updateFirebase("Timer/CurrentProcess", currentProcess);
}

void startMixer() {
  
   digitalWrite(relay_conveyorForward, LOW);
   delay(3000);
   digitalWrite(relay_conveyorForward, HIGH);

    countdownTime = timers[6] * 60000;
    processRunning = true;
    currentProcess = "Mixing";  // Fixed incorrect process name

    digitalWrite(relay_slicerTimer, HIGH);
    digitalWrite(relay_washerTimer, HIGH);
    digitalWrite(relay_dryerTimer, HIGH);
    digitalWrite(relay_fryerTimer, HIGH);
    digitalWrite(relay_coolerTimer, HIGH);
    digitalWrite(relay_Flavor_bqq, HIGH);
    digitalWrite(relay_Flavor_chesse, HIGH);
    digitalWrite(relay_mixerTimer, LOW);

    Serial.printf("Starting Mixing for %d minutes\n", timers[5]);
    updateFirebase("Timer/CurrentProcess", currentProcess);
}
// Update Firebase integer value
void updateFirebase(String path, int value) {
  if (!Firebase.RTDB.setInt(&fbdo, path.c_str(), value)) {
    Serial.println("Error updating Firebase: " + fbdo.errorReason());
    processRunning = false;
  }
}

// Update Firebase string value
void updateFirebase(String path, String value) {
  if (!Firebase.RTDB.setString(&fbdo, path.c_str(), value)) {
    Serial.println("Error updating Firebase: " + fbdo.errorReason());
  }
}
// Reset all timer values in Firebase back to 0 and update status nodes
void resetFirebaseValues() {
const char* timers = ""; 
for (int i = 0; i < strlen(timers); i++) {
    String path = "operationsDuration/" + String(timers[i]) + "/duration";
    updateFirebase(path, 0);
  }
  updateFirebase("Timer/remainingTime", "Set a Timer");
  updateFirebase("Timer/currentProcess", "None");
}

// Check for new timer data. If new non-zero timer is detected, load the data and restart the process sequence.
void checkForRestart() {
  // For example, we check a separate node "Timer/newData" to know when new values are entered.
  // If newData is non-zero, assume that new timer data has been entered.
  if (Firebase.RTDB.getInt(&fbdo, "Timer/newData")) {
    int newDataFlag = fbdo.intData();
    if (newDataFlag > 0) {
      Serial.println("New timer data detected. Reloading process data...");
      getTimersFromDatabase();
      // Reset the newData flag in Firebase to 0
      updateFirebase("Timer/newData", 0);
      // Reset process index to start from the beginning
      currentProcessIndex = 0;
      processRunning = false;
    }
  }
}