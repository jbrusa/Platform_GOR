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


#define S(x)                        String(x)
#define SF(x)                       S(F(x))
#define IP_STR(x)                   S(x[0]) + S(".") + S(x[1]) + S(".") + S(x[2]) + S(".") + S(x[3])

// Commenter ici pour désactiver le débug via le port série
#define SERIAL_DEBUG                1

#define VERSION                     0.1

#define PIN_CS_SD                   2
#define PIN_RELAY                   15

#define SERIAL_SPEED                115200

#define INTERVAL_MQTT_KEEPALIVE     5000
#define INTERVAL_HEARTBEAT          5000
#define INTERVAL_WIFI_KEEPALIVE     60000


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
public:
    GOR_SDCard sd;
    GOR_Config config;
    GOR_WebServer webserver;
    SimpleTimer timer;

    bool debugMode;



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
        WiFi.begin(config.wifiSSID.c_str(), config.wifiPwd.c_str());

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
    if(event == SYSTEM_EVENT_STA_CONNECTED) {
        gor.Debug(S("Wifi - Connected to ") + gor.config.wifiSSID);
        gor.Debug(S("Wifi - Getting IP..."));
    }

    if(event == SYSTEM_EVENT_STA_GOT_IP) {
        gor.Debug(S("Wifi - Got IP ") + IP_STR(WiFi.localIP()));
    }

    if(event == SYSTEM_EVENT_STA_DISCONNECTED) {
        gor.Debug(S("Wifi - Disconnected"));
        WiFi.begin();
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


void setup() {
    gor.begin();
}

void loop() {
    gor.loop();
}

#endif //PLATFORM_GOR_GOR_PLATFORM_HPP
