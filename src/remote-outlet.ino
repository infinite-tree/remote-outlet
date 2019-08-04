// Built-in includes
#include <Arduino.h>
#include "Config.h"
#include <WiFi.h>

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// // DNS doesn't seem to work without these
// #include "lwip/inet.h"
// #include "lwip/dns.h"


// Downloaded from https://github.com/teebr/Influx-Arduino
#include <InfluxArduino.hpp>
#include "RootCert.hpp"


// Sensor Pins
#define OUTLET_SENSE_PIN     2
#define OUTLET_CONTROL_PIN   4

// How often to send data in seconds
#define DATA_DELAY_SEC              60

// How often to send data in milliseconds
#define DATA_DELAY                  DATA_DELAY_SEC * 1000

InfluxArduino influx;

// Wifi Settings
#define CONNECTION_RETRY        10

// Influx Settings

const char OUTLET_MEASUREMENT[] = "outlet_on";

String Tags;
uint8_t OutletState = 0;
unsigned long LastDataSent = 0;

Config config;

// void printDNSServers()
// {
//     Serial.print("DNS #1, #2 IP: ");
//     WiFi.dnsIP().printTo(Serial);
//     Serial.print(", ");
//     WiFi.dnsIP(1).printTo(Serial);
//     Serial.println();
// }

void connectToWifi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Wifi already connceted");
        return;
    }

    WiFi.mode(WIFI_STA);
    while (true)
    {
        Serial.print("Connecting to Wifi network ");
        Serial.print(config.WifiSSID);
        Serial.print(" ");
        WiFi.scanNetworks();
        WiFi.begin(config.WifiSSID.c_str(), config.WifiPassword.c_str());
        for (int x = 0; x < CONNECTION_RETRY; x++)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("Connected");
                return;
            }
            Serial.print(".");
            delay(500);
        }
    }
}


bool sendDatapoint(const char *measurement, String tags,String fields) {
    // Make sure there is a connection
    if (WiFi.status() != WL_CONNECTED) {
        connectToWifi();
    }

    // Send the data point
    if (!influx.write(measurement, tags.c_str(), fields.c_str()))
    {
        Serial.print("ERROR sending ");
        Serial.print(measurement);
        Serial.print(": ");
        Serial.println(influx.getResponse());
        return false;
    }

    Serial.print("MEASUREMENT: ");
    Serial.print(measurement);
    Serial.print(", tags: ");
    Serial.print(tags);
    Serial.print(", fields: ");
    Serial.println(fields);

    return true;
}


void reconfigureCheck() {
    if (Serial.available()) {
        char code = Serial.read();
        if (code == 'i' || code == 'I') {
            printConfig(config);
            return;
        } else if (code == 'c' || code == 'C') {
            // Reconfigure the sensor
            askForSettings(config);
            Serial.println("Please reboot now");
            while (1)
            {
            };
        } else if (code == 's' || code == 'S') {
            askForSiteSettings(config);
            saveConfig(config);
            Serial.println("Please reboot now");
            while (1)
            {
            };
        } else if (code == 'p' || code == 'P') {
            askForPreferences(config);
            saveConfig(config);
            Serial.println("Please reboot now");
            while (1)
            {
            };
        }
    }
}


void setupInflux() {
    influx.configure(config.InfluxDatabase.c_str(), config.InfluxHostname.c_str());
    influx.authorize(config.InfluxUser.c_str(), config.InfluxPassword.c_str());
    influx.addCertificate(ROOT_CERT);
}


void setupOTA() {
    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("End Failed");
        });

    ArduinoOTA.begin();
}

void setup() {
    Serial.begin(115200);
    Serial.println("####### ESP32 OUTLET INIT #######");

    loadConfig(config);
    if (config.Magic != CONFIG_MAGIC) {
        askForSettings(config);
    }

    // wait to see if user wants to update settings
    delay(1000);
    reconfigureCheck();
    Tags = "location=" + config.Location + ",sensor=" + config.Sensor;
    LastDataSent = millis();

    pinMode(OUTLET_SENSE_PIN, INPUT);
    OutletState = digitalRead(OUTLET_SENSE_PIN);

    pinMode(OUTLET_CONTROL_PIN, OUTPUT);
    digitalWrite(OUTLET_CONTROL_PIN, LOW);

    connectToWifi();
    setupInflux();

    Serial.println("Configuring OTA...");
    ArduinoOTA.setHostname("remote-outlet-esp");
    setupOTA();

    Serial.println("Setup Complete. Entering run loop");
}


void loop() {
    OutletState = digitalRead(OUTLET_SENSE_PIN);

    unsigned long now = millis();
    if ((now - LastDataSent) > DATA_DELAY) {
        LastDataSent = now;

        Serial.print("Outlet State: ");
        Serial.println(OutletState);
        String fields = "value=" + String(OutletState) + ".0";
        sendDatapoint(OUTLET_MEASUREMENT, Tags, fields);
    }

    ArduinoOTA.handle();
    delay(50);
}