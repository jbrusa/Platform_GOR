//
// Created by Jordi Brusa on 28.05.17.
//

#ifndef PLATFORM_GOR_GOR_PLATFORM_HPP
#define PLATFORM_GOR_GOR_PLATFORM_HPP

#include <WiFi.h>
#include <WiFiClient.h>

#include "PubSubClient.h"
#include "SimpleTimer.h"

#include "GOR_SDCard.hpp"
#include "GOR_Config.hpp"
#include "GOR_WebServer.hpp"
#include "GOR_Relay.h"
#include "GOR_RGBLed.hpp"

#define S(x)                        String(x)
#define SF(x)                       S(F(x))
#define IP_STR(x)                   S(x[0]) + S(".") + S(x[1]) + S(".") + S(x[2]) + S(".") + S(x[3])

// Commenter ici pour désactiver le débug via le port série
#define SERIAL_DEBUG                1

#define VERSION                     0.1

#define PIN_CS_SD                   2
#define PIN_RELAY                   15

#define RGBLED_R_PIN                13
#define RGBLED_G_PIN                12
#define RGBLED_B_PIN                14

#define SERIAL_SPEED                115200

#define INTERVAL_MQTT_KEEPALIVE     5000
#define INTERVAL_HEARTBEAT          5000
#define INTERVAL_WIFI_KEEPALIVE     60000
#define INTERVAL_RGBLED             200


extern void GOR_Setup();
extern void GOR_Loop();
extern void GOR_MessageReceived(String, String);
extern String GOR_HeartbeatPayload();

void MqttKeepAlive(void);
void WifiEvent(WiFiEvent_t);
void WifiKeepAlive(void);
void Heartbeat(void);
void onMqttMessage(char*, byte*, unsigned int);

WiFiClient wifi;
PubSubClient mqtt(wifi);



class GOR_Platform {
private:
    bool debugMode;


public:
    GOR_SDCard sd;
    GOR_Config config;
    GOR_WebServer webserver;
    GOR_RGBLed led;
    GOR_Relay relay;
    SimpleTimer timer;


    void setDebugMode(bool debugMode)   { this->debugMode = debugMode; }
    bool getDebugMode()                 { return debugMode; }

    void Debug(String s) {
        #ifdef SERIAL_DEBUG
            Serial.println(s);
        #endif

        if(debugMode) {
            sd.log(s);
        }
    }

    void begin() {
        Serial.begin(SERIAL_SPEED);
        while(!Serial) {
        }

        Debug(SF("GOR Node Version ") + String(VERSION));
        Debug(SF("--------------------------------"));
        Debug(SF("SD Card init"));
        sd.begin(PIN_CS_SD);

        Debug(SF("--------------------------------"));
        Debug(SF("Config init"));
        config.begin();

        Debug(SF("--------------------------------"));
        delay(1000);

        Debug(SF("Wifi init"));
        Debug(SF("Connecting to ") + config.wifiSSID + " using " + config.wifiPwd);
        WiFi.onEvent(WifiEvent);
        WiFi.enableSTA(true);
        WiFi.enableAP(false);


        Debug(SF("--------------------------------"));
        Debug(SF("MQTT init"));
        mqtt.setCallback(onMqttMessage);
        mqtt.setServer(config.mqttHost.c_str(),atoi(config.mqttPort.c_str()));

        Debug(SF("--------------------------------"));
        Debug(SF("Timer init"));
        timer.setInterval(INTERVAL_MQTT_KEEPALIVE, MqttKeepAlive);
        timer.setInterval(INTERVAL_WIFI_KEEPALIVE, WifiKeepAlive);
        timer.setInterval(INTERVAL_HEARTBEAT, Heartbeat);

        Debug(SF("--------------------------------"));
        Debug(SF("Relay init"));
        relay.begin(PIN_RELAY);

        Debug(SF("--------------------------------"));
        Debug(SF("RGB LED init"));
        led.begin(RGBLED_R_PIN, RGBLED_G_PIN, RGBLED_B_PIN);

        Debug(SF("--------------------------------"));
        Debug(SF("Webserver init"));
        webserver.begin();

        Debug(SF("--------------------------------"));
        Debug(SF("Custom init"));
        GOR_Setup();

        Debug(SF("--------------------------------"));
        Debug(SF("Init DONE"));
    }

    void sendEvent(String s) {
        Debug(SF("MQTT - Sent event : ") + s);
        mqtt.publish(config.mqttTopicEvents.c_str(), s.c_str());
    }


    void loop() {
        timer.run();
        mqtt.loop();
        webserver.loop();

        // Custom loop
        GOR_Loop();
    }
};

GOR_Platform gor;


void MqttKeepAlive() {
    if(!WiFi.isConnected())
        return;

    if(mqtt.connected())
        return;

    gor.Debug(SF("MQTT - Connecting..."));
    if (mqtt.connect(gor.config.nodeName.c_str())) {
        gor.led.setColor(COLOR_BLUE);
        mqtt.subscribe(gor.config.mqttTopicActions.c_str());

        if(gor.config.mqttTopicExtra1 != "") mqtt.subscribe(gor.config.mqttTopicExtra1.c_str());
        if(gor.config.mqttTopicExtra2 != "") mqtt.subscribe(gor.config.mqttTopicExtra2.c_str());
        if(gor.config.mqttTopicExtra3 != "") mqtt.subscribe(gor.config.mqttTopicExtra3.c_str());

        gor.Debug(SF("MQTT - Connected OK"));
        String event = gor.config.nodeName + "_Connected";
        gor.sendEvent(event.c_str());
    }
}


void WifiEvent(WiFiEvent_t event) {
    switch(event) {
        case SYSTEM_EVENT_STA_START:
            gor.Debug(S("Wifi - Started "));
            gor.led.setColor(COLOR_RED);
            WiFi.begin(gor.config.wifiSSID.c_str(), gor.config.wifiPwd.c_str());
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
            gor.Debug(S("Wifi - Connected to ") + gor.config.wifiSSID);
            gor.Debug(S("Wifi - Getting IP..."));
            gor.led.setColor(COLOR_ORANGE);
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            gor.Debug(S("Wifi - Got IP ") + IP_STR(WiFi.localIP()));
            gor.led.setColor(COLOR_GREEN);
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            gor.Debug(S("Wifi - Disconnected"));
            gor.led.setColor(COLOR_RED);
            WiFi.begin();
            break;
    }

}

void WifiKeepAlive() {
    if(WiFi.isConnected())
        return;
    gor.Debug(SF("Wifi - Re-connecting..."));
    WiFi.reconnect();
}


void Heartbeat() {
    if(!mqtt.connected())
        return;

    String payload = GOR_HeartbeatPayload();
    gor.Debug(SF("MQTT - Heartbeat - ") + payload);
    mqtt.publish(gor.config.mqttTopicHeartbeat.c_str(), payload.c_str());
}


void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    String topicStr = S(topic);
    String payloadStr = "";
    for(int i =0; i<length; i++) {
        payloadStr = payloadStr + (char)payload[i];
    }

    String outTopic = topicStr;
    if(topicStr == gor.config.mqttTopicActions)
        outTopic = "ACTION";
    else if(topicStr == gor.config.mqttTopicExtra1)
        outTopic = "EXTRA1";
    else if(topicStr == gor.config.mqttTopicExtra2)
        outTopic = "EXTRA2";
    else if(topicStr == gor.config.mqttTopicExtra3)
        outTopic = "EXTRA3";

    gor.Debug(SF("MQTT - Message received - ") + outTopic + SF(" - ") + payloadStr);

    GOR_MessageReceived(outTopic, payloadStr);
}

String GetInfoESP () {
    String response = "";
    response += S("\"NodeName\":\"") + gor.config.nodeName + S("\",");
    response += S("\"Free memory \":\"") + ESP.getFreeHeap() + S(" bytes\",");
    response += S("\"Chip revision \":\"") + ESP.getChipRevision() + S("\",");
    response += S("\"CPU Freq \":\"") + ESP.getCpuFreqMHz() + S(" MHz\",");
    response += S("\"Flash size \":\"") + ESP.getFlashChipSize() + S(" bytes\"");
    return response;
}


String GetInfoWifi () {
    String response = "";
    response += S("\"SSID \":\"") + WiFi.SSID() + S("\",");
    response += S("\"Signal Strength \":\"") + WiFi.RSSI() + S("\",");
    response += S("\"MAC Address \":\"") + WiFi.macAddress() + S("\",");
    response += S("\"IP\":\"") + IP_STR(WiFi.localIP()) + S("\",");
    response += S("\"Subnet Mask\":\"") + IP_STR(WiFi.subnetMask()) + S("\",");
    response += S("\"Gateway \":\"") + IP_STR(WiFi.gatewayIP()) + S("\",");
    response += S("\"DNS \":\"") + IP_STR(WiFi.dnsIP()) + S("\",");
    response += S("\"MQTT Connected \":\"") + (mqtt.connected() ? S("Yes") : S("No")) + S("\",");
    response += S("\"MQTT Host \":\"") + gor.config.mqttHost + ":" +  gor.config.mqttPort + S("\"");

    return response;
}

void setup() {
    gor.begin();
}

void loop() {
    gor.loop();
}

#endif //PLATFORM_GOR_GOR_PLATFORM_HPP
