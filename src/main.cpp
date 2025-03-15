#include <Arduino.h>
#include <WiFi.h>

#include <Firebase_Manager.h>

const char *ssid = "Salva Residence 2.4G";
const char *password = "YouareGodAlone_23";

String sampleVal;

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.print("Connected");
    firebaseInit();
    firebaseStreamInit();
}

void loop()
{

    // // firebaseSendData(23, "/int", "23");

    // int intValue;
    // String strval;
    // firebaseReadData(strval, "/currentOperationdff");

    sampleVal = firebaseStreamData(streamSlicingState, "Slicing");
    int sampleInt = parseStreamData<int>(sampleVal, "int");

    Serial.print("Sample Val ");
    Serial.println(sampleVal);
    delay(1500);
}