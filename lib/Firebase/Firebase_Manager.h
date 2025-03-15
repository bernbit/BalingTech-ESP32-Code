#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <Arduino.h>
#include <Firebase_ESP_Client.h>

// Define Firebase objects and status flags
extern FirebaseData fbdo, streamSlicingState, streamWashingState, streamDryingState, streamFryingState, streamCoolingState, streamFlavoringState, streamMixingState;
extern FirebaseAuth auth;
extern FirebaseConfig config;
extern String uid;
extern bool login;

// Firebase Paths
extern String startOperation;
extern String pauseOperation;
extern String operationsDuration;
extern String remainingTime;
extern String currentOperation;
extern String flavorGrams;
extern String flavorType;
extern String manualMode;
extern String forward;
extern String backward;

// Function Declarations
void firebaseInit();
void firebaseStreamInit();

String firebaseStreamData(FirebaseData &stream, String streamName);

// Data Type Convertion Functions
bool parseBool(const String &data);
int parseInt(const String &data);
float parseFloat(const String &data);
String parseString(const String &data);

#include "Firebase_Manager.tpp" // Include template implementation

#endif // FIREBASE_MANAGER_H
