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
#define INTERVAL_WIFI_KEEPALIVE     10000

#define PRM_NODENAME                "NODENAME"

#define PRM_WIFI_SSID               "WIFI_SSID"
#define PRM_WIFI_PWD                "WIFI_PWD"

#define PRM_MQTT_HOST               "MQTT_HOST"
#define PRM_MQTT_PORT               "MQTT_PORT"
#define PRM_MQTT_TOPIC_HEARTBEAT    "MQTT_TOPIC_HEARTBEAT"
#define PRM_MQTT_TOPIC_ACTIONS      "MQTT_TOPIC_ACTIONS"
#define PRM_MQTT_TOPIC_DEBUG        "MQTT_TOPIC_DEBUG"
#define PRM_MQTT_TOPIC_EVENTS       "MQTT_TOPIC_EVENTS"
#define PRM_MQTT_TOPIC_EXTRA1       "MQTT_TOPIC_EXTRA1"
#define PRM_MQTT_TOPIC_EXTRA2       "MQTT_TOPIC_EXTRA2"
#define PRM_MQTT_TOPIC_EXTRA3       "MQTT_TOPIC_EXTRA3"


extern void GOR_Setup();
extern void GOR_Loop();
extern void GOR_MessageReceived(String, String);
extern String GOR_HeartbeatPayload();

void MqttKeepAlive(void);
void WifiKeepAlive(void);
void Heartbeat(void);
void onMqttMessage(char*, byte*, unsigned int);




WiFiClient wifi;
PubSubClient mqtt(wifi);



class GOR_Platform {
public:
    GOR_SDCard sd;
    SimpleTimer timer;

    bool debugMode;

    String nodeName;
    String prmWifiSSID;
    String prmWifiPwd;
    String prmMqttHost;
    String prmMqttPort;
    String prmMqttTopicHeartbeat;
    String prmMqttTopicActions;
    String prmMqttTopicDebug;
    String prmMqttTopicEvents;
    String prmMqttTopicExtra1;
    String prmMqttTopicExtra2;
    String prmMqttTopicExtra3;

    void Debug(String s) {
        #ifdef SERIAL_DEBUG
            Serial.println(s);
        #endif

        if(debugMode) {
            sd.log(s);
        }
    }

    String getSetting(String settingName) {
        return sd.getSetting(settingName);
    }

    void begin() {
        Serial.begin(SERIAL_SPEED);
        while(!Serial) {
        }

        Debug(SF("GOR Node Version ") + String(VERSION));
        Debug(SF("--------------------------------"));
        Debug(SF("SD Card init"));
        sd.begin(PIN_CS_SD);

        nodeName = getSetting(PRM_NODENAME);
        prmWifiSSID = getSetting(PRM_WIFI_SSID);
        prmWifiPwd = getSetting(PRM_WIFI_PWD);
        prmMqttHost = getSetting(PRM_MQTT_HOST);
        prmMqttPort = getSetting(PRM_MQTT_PORT);
        prmMqttTopicHeartbeat = getSetting(PRM_MQTT_TOPIC_HEARTBEAT);
        prmMqttTopicActions = getSetting(PRM_MQTT_TOPIC_ACTIONS);
        prmMqttTopicDebug = getSetting(PRM_MQTT_TOPIC_DEBUG);
        prmMqttTopicEvents = getSetting(PRM_MQTT_TOPIC_EVENTS);
        prmMqttTopicExtra1 = getSetting(PRM_MQTT_TOPIC_EXTRA1);
        prmMqttTopicExtra2 = getSetting(PRM_MQTT_TOPIC_EXTRA2);
        prmMqttTopicExtra3 = getSetting(PRM_MQTT_TOPIC_EXTRA3);

        Debug(SF("--------------------------------"));
        Debug(SF("Wifi init"));
        WiFi.begin(prmWifiSSID.c_str(), prmWifiPwd.c_str());

        Debug(SF("--------------------------------"));
        Debug(SF("MQTT init"));
        mqtt.setCallback(onMqttMessage);
        mqtt.setServer(prmMqttHost.c_str(),atoi(prmMqttPort.c_str()));

        Debug(SF("--------------------------------"));
        Debug(SF("Timer init"));
        timer.setInterval(INTERVAL_MQTT_KEEPALIVE, MqttKeepAlive);
        timer.setInterval(INTERVAL_WIFI_KEEPALIVE, WifiKeepAlive);
        timer.setInterval(INTERVAL_HEARTBEAT, Heartbeat);

        Debug(SF("--------------------------------"));
        Debug(SF("Custom init"));
        GOR_Setup();

        Debug(SF("--------------------------------"));
        Debug(SF("Init DONE"));
    }

    void sendEvent(String s) {
        Debug(SF("MQTT - Sent event : ") + s);
        mqtt.publish(prmMqttTopicEvents.c_str(), s.c_str());
    }

    void loop() {
        timer.run();
        mqtt.loop();

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
    if (mqtt.connect(gor.nodeName.c_str())) {
        mqtt.subscribe(gor.prmMqttTopicActions.c_str());

        if(gor.prmMqttTopicExtra1 != "") mqtt.subscribe(gor.prmMqttTopicExtra1.c_str());
        if(gor.prmMqttTopicExtra2 != "") mqtt.subscribe(gor.prmMqttTopicExtra2.c_str());
        if(gor.prmMqttTopicExtra3 != "") mqtt.subscribe(gor.prmMqttTopicExtra3.c_str());

        gor.Debug(SF("MQTT - Connected OK"));
        String event = gor.nodeName + "_Connected";
        gor.sendEvent(event.c_str());
    }
}


void WifiKeepAlive() {
    if(WiFi.isConnected())
        return;
    gor.Debug(SF("Wifi - Re-connecting..."));
    WiFi.reconnect();
}


void Heartbeat() {
    String payload = GOR_HeartbeatPayload();
    gor.Debug(SF("MQTT - Heartbeat - ") + payload);
    mqtt.publish(gor.prmMqttTopicHeartbeat.c_str(), payload.c_str());
}


void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    String topicStr = S(topic);
    String payloadStr = "";
    for(int i =0; i<length; i++) {
        payloadStr = payloadStr + (char)payload[i];
    }

    String outTopic = topicStr;
    if(topicStr == gor.prmMqttTopicActions)
        outTopic = "ACTION";
    else if(topicStr == gor.prmMqttTopicExtra1)
        outTopic = "EXTRA1";
    else if(topicStr == gor.prmMqttTopicExtra2)
        outTopic = "EXTRA2";
    else if(topicStr == gor.prmMqttTopicExtra3)
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
