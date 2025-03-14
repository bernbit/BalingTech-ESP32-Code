#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <Arduino.h>
#include <Firebase_ESP_Client.h>

// Firebase Path Declarations
extern String DATABASE_URL;
extern String API_KEY;
extern String USER_EMAIL;
extern String USER_PASSWORD;

// Firebase Path Declarations
extern String startOperationPath;
extern String pauseOperationPath;
extern String operationsDurationPath;
extern String remainingTimePath;
extern String currentOperationPath;
extern String flavorGramsPath;
extern String flavorTypePath;
extern String manualModePath;
extern String forwardPath;
extern String backwardPath;

// Function declarations
void firebaseInit();        
void firebaseStreamInit();  

// Firebase Sending Function
template <typename SendType>
void firebaseSendData(SendType data, const std::string& dataPath);

// Firebase Reading Function
template <typename ReadType>
void firebaseReadData(ReadType data, String path);

// Firebase Stream Function
String firebaseStreamData(FirebaseData& stream, String streamName);

#endif
