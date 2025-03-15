#ifndef FIREBASE_MANAGER_TPP
#define FIREBASE_MANAGER_TPP

#include <Arduino.h>

// Template Function Implementation
template <typename SendType>
void firebaseSendData(SendType data, const String &dataPath, const String &dataVal)
{

    if (Firebase.ready() && login)
    {

        if (Firebase.RTDB.set(&fbdo, dataPath, data))
        {
            Serial.println();
            Serial.print(dataVal);
            Serial.print(" - successfully send data to: ");
            Serial.print(fbdo.dataPath());
            Serial.print(" (");
            Serial.print(fbdo.dataType());
            Serial.println(")");
        }
        else
        {
            Serial.print("Failed to send ");
            Serial.print(dataVal);
            Serial.print(": ");
            Serial.println(fbdo.errorReason().c_str());
        }
    }
}

template <typename ReadType>
void firebaseReadData(ReadType &dataVal, const String &path)
{
    if (Firebase.RTDB.get(&fbdo, path))
    {
        dataVal = fbdo.to<ReadType>();
        Serial.println();
        Serial.print(dataVal);
        Serial.print(" - successfully read data to: ");
        Serial.print(fbdo.dataPath());
        Serial.print(" (");
        Serial.print(fbdo.dataType());
        Serial.println(")");
    }
    else
    {
        Serial.println(fbdo.errorReason());

        Serial.print("Failed to Read Data to - ");
        Serial.print(fbdo.dataPath());
        Serial.print(" ");
        Serial.println(fbdo.errorReason().c_str());
    }
}

#endif // FIREBASE_MANAGER_TPP
