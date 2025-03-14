#include <Firebase_Manager.h>

// Arduino
#include <Arduino.h>

// Firebase ESP32 Main Library
#include <Firebase_ESP_Client.h>

#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"

// Firebase Objects
FirebaseData fbdo, streamSlicing, streamWashing, streamDrying, streamFrying, streamCooling, streamFlavoring, streamMixing;
FirebaseJson json;
FirebaseAuth auth;
FirebaseConfig config;
String uid;
bool login = false;

// Firebase Credentials
String DATABASE_URL = "https://balingtech-450a3-default-rtdb.firebaseio.com/";
String API_KEY = "AIzaSyBcK1n4597MRI6-8SP5KV6EqgEq-AeOlZA";
String USER_EMAIL = "balingtech@esp.com";
String USER_PASSWORD = "balingtech_2025";

// Firebase Paths
String startOperation = "/startOperation";
String pauseOperation = "/pauseOperation";
String operationsDuration = "/operationsDuration";
String remainingTime = "/remainingTime";
String currentOperation = "/currentOperation";
String flavorGrams = "/flavorGrams";
String flavorType = "/flavorType";
String manualMode = "/manualMode";
String forward = "/forward";
String backward = "/backward";

// Firebase Setup Function
void firebaseInit() {
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

    Serial.print("Sign-Up Status: ");
    Serial.print(login);
}

// Initialize Firebase Streams
void firebaseStreamInit() {
    beginStream(streamSlicing, (std::string(operationsDuration.c_str()) + "Slicing").c_str());
    beginStream(streamWashing, (std::string(operationsDuration.c_str()) + "Washing").c_str());
    beginStream(streamDrying, (std::string(operationsDuration.c_str()) + "Drying").c_str());
    beginStream(streamFrying, (std::string(operationsDuration.c_str()) + "Frying").c_str());
    beginStream(streamCooling, (std::string(operationsDuration.c_str()) + "Cooling").c_str());
    beginStream(streamFlavoring, (std::string(operationsDuration.c_str()) + "Flavoring").c_str());
    beginStream(streamMixing, (std::string(operationsDuration.c_str()) + "Mixing").c_str());
}

// Firebase Sending Function
template <typename SendType>
void firebaseSendData(SendType data, const std::string& dataPath) {
    if (Firebase.RTDB.set(&fbdo, dataPath, data)) {
        Serial.println();
        Serial.print(String(data));
        Serial.print("- successfully saved to: ");
        Serial.print(fbdo.dataPath());
        Serial.print(" (");
        Serial.print(fbdo.dataType());
        Serial.println(")");

    } else {
        Serial.print("Failed: ");
        Serial.println(fbdo.errorReason().c_str());
    }
}

// Firebase Reading Function
template <typename ReadType>
void firebaseReadData(ReadType data, String path) {
    if (Firebase.RTDB.get(&fbdo, path)) {
        data = fbdo.to<ReadType>();
        Serial.print("Successfully get: ");
        Serial.println(intValue);
    } else {
        Serial.println(fbdo.errorReason());
    }
}

// Firebase Stream Function
String firebaseStreamData(FirebaseData& stream, String streamName) {
    String result = "";  // Default empty

    if (Firebase.ready() && login) {
        if (!Firebase.RTDB.readStream(&stream)) {
            Serial.printf("Reading %s Stream Failed, %s\n\n", streamName.c_str(), stream.errorReason().c_str());
            return result;
        }

        if (stream.streamAvailable()) {
            String type = stream.dataType();

            if (type == "boolean") {
                result = stream.boolData() ? "true" : "false";
            } else if (type == "int" || type == "integer") {
                result = String(stream.intData());
            } else if (type == "float" || type == "double") {
                result = String(stream.floatData(), 3);
            } else if (type == "string") {
                result = stream.stringData();
            } else if (type == "json") {
                FirebaseJson& json = stream.jsonObject();
                String jsonStr;
                json.toString(jsonStr, true);
                result = jsonStr;
            } else if (type == "blob") {
                result = "Blob Data Received";
            } else {
                result = "Unknown Data Type";
            }
        }
    }

    return result;
}

// Helper Functions
void beginStream(FirebaseData& stream, String path) {
    if (!Firebase.RTDB.beginStream(&stream, path.c_str()))
        Serial.printf("Unable to Begin Stream for %s, %s\n\n", path.c_str(), stream.errorReason().c_str());
}
