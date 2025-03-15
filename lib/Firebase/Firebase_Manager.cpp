#include <Firebase_Manager.h>

// Arduino
#include <Arduino.h>
// Firebase ESP32 Main Library
#include <Firebase_ESP_Client.h>
// Firebase ESP32 Helper
#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"

// Firebase Objects
FirebaseData fbdo, streamSlicingState, streamWashingState, streamDryingState, streamFryingState, streamCoolingState, streamFlavoringState, streamMixingState;
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

//* Helper Functions
void beginStream(FirebaseData &stream, String path)
{
    if (!Firebase.RTDB.beginStream(&stream, path.c_str()))
        Serial.printf("Unable to Begin Stream for %s, %s\n\n", path.c_str(), stream.errorReason().c_str());
}

// Data Type Convertion Functions
bool parseBool(const String &data)
{
    return data == "true";
}

int parseInt(const String &data)
{
    return data.toInt();
}

float parseFloat(const String &data)
{
    return data.toFloat();
}

String parseString(const String &data)
{
    return data;
}

//! ==================================== Section Divider ===================================================

//* Main Function
// Firebase Setup Function
void firebaseInit()
{
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
    while ((auth.token.uid) == "")
    {
        Serial.print('.');
        delay(1000);
    }
    // Print user UID
    uid = auth.token.uid.c_str();
    Serial.print("User UID: ");
    Serial.println(uid);

    if (auth.token.uid != "")
    {
        login = true;
    }

    Serial.print("Sign-Up Status: ");
    Serial.print(login);
}

// Firebase Initialize Streams
void firebaseStreamInit()
{
    beginStream(streamSlicingState, (operationsDuration + "/Slicing").c_str());
    beginStream(streamWashingState, (operationsDuration + "/Washing").c_str());
    beginStream(streamDryingState, (operationsDuration + "/Drying").c_str());
    beginStream(streamFryingState, (operationsDuration + "/Frying").c_str());
    beginStream(streamCoolingState, (operationsDuration + "/Cooling").c_str());
    beginStream(streamFlavoringState, (operationsDuration + "/Flavoring").c_str());
    beginStream(streamMixingState, (operationsDuration + "/Mixing").c_str());
}

// Firebase Stream Function
String lastStreamData = ""; // global variable to keep last known value
String firebaseStreamData(FirebaseData &stream, String streamName)
{
    if (Firebase.ready() && login)
    {
        if (!Firebase.RTDB.readStream(&stream))
        {
            Serial.printf("Reading %s Stream Failed, %s\n\n", streamName.c_str(), stream.errorReason().c_str());
            return lastStreamData; // return last known value if failed
        }

        if (stream.streamAvailable())
        {

            String type = stream.dataType();
            String result; // Temporary result for this update

            if (type == "boolean")
            {
                result = stream.boolData() ? "true" : "false";
            }
            else if (type == "int" || type == "integer")
            {
                result = String(stream.intData());
            }
            else if (type == "float" || type == "double")
            {
                result = String(stream.floatData(), 3);
            }
            else if (type == "string")
            {
                result = stream.stringData();
            }
            else
            {
                result = "Unknown Data Type";
            }

            lastStreamData = result; // Update last known value
        }
    }

    return lastStreamData; // Always return last known value
}
