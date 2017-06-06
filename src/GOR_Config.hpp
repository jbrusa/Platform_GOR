//
// Created by Jordi Brusa on 30.05.17.
//

#ifndef PLATFORM_GOR_GOR_CONFIG_HPP
#define PLATFORM_GOR_GOR_CONFIG_HPP

#include <map>
#include <SD.h>

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

#define CONFIG_FILE                 "/CONFIG.TXT"
#define CONFIG_FILE_BAK             "/CONFIG.BAK"

#define READ_NAME                   1
#define READ_VAL                    2
#define MAX_NAME_LENGTH             128
#define MAX_VALUE_LENGTH            512

using namespace std;

enum GOR_Config_Errors {
    FILE_NOT_FOUND = 1
};

class GOR_Config {
public:
    std::map<String, String> settings;

    String nodeName;
    String wifiSSID;
    String wifiPwd;
    String mqttHost;
    String mqttPort;
    String mqttTopicHeartbeat;
    String mqttTopicActions;
    String mqttTopicDebug;
    String mqttTopicEvents;
    String mqttTopicExtra1;
    String mqttTopicExtra2;
    String mqttTopicExtra3;


    void setSetting(String name, String value) {
        settings[name] = value;
    }

    void saveSettings() {
        std::map<String, String>::iterator it;

        SD.remove(CONFIG_FILE);
        File file = SD.open(CONFIG_FILE, FILE_WRITE);

        for(it = settings.begin() ; it != settings.end() ; ++it) {
            file.println(it->first + "=" + it->second);
        }

        file.close();
    }

    int begin() {
        File file = SD.open(CONFIG_FILE);
        if(!file){
            return GOR_Config_Errors::FILE_NOT_FOUND;
        }

        // Lecture du fichier de config, population du vecteur
        int reading = READ_NAME;
        String settingName = "";
        String settingValue = "";
        while(file.available()) {
            char ch = file.read();

            if(ch == '\r') {
                // Caractère 'Carriage return', fin de ligne DOS/Windows, ignoré
                continue;
            } else if(ch == '\n') {
                // Caractère 'Line Feed', fin de ligne, indique la fin d'un paramètre
                settings[settingName] =  settingValue;

                reading = READ_NAME;
                settingName = "";
                settingValue = "";
            } else if(ch == '#' && settingName == "") {
                // Un '#' trouvé en début de ligne, ignorer les caractères jusqu'à la fin de la ligne
                while(file.available()) {
                    char ch2 = file.read();
                    if(ch2 == '\n') break;
                }
            } else if(ch == '=') {
                // Caractère '=', on passe du nom à la valeur
                reading = READ_VAL;
                settingName.trim();
            } else {
                // Autres caractères, ajoutés au nom ou a la valeur
                if(reading == READ_NAME && settingName.length() < MAX_NAME_LENGTH) {
                    settingName = settingName + ch;
                }

                if(reading == READ_VAL && settingValue.length() < MAX_VALUE_LENGTH) {
                    settingValue = settingValue + ch;
                }
            }
        }

        file.close();

        nodeName = settings[PRM_NODENAME];
        wifiSSID = settings[PRM_WIFI_SSID];
        wifiPwd = settings[PRM_WIFI_PWD];
        mqttHost = settings[PRM_MQTT_HOST];
        mqttPort = settings[PRM_MQTT_PORT];
        mqttTopicHeartbeat = settings[PRM_MQTT_TOPIC_HEARTBEAT];
        mqttTopicActions = settings[PRM_MQTT_TOPIC_ACTIONS];
        mqttTopicDebug = settings[PRM_MQTT_TOPIC_DEBUG];
        mqttTopicEvents = settings[PRM_MQTT_TOPIC_EVENTS];
        mqttTopicExtra1 = settings[PRM_MQTT_TOPIC_EXTRA1];
        mqttTopicExtra2 = settings[PRM_MQTT_TOPIC_EXTRA2];
        mqttTopicExtra3 = settings[PRM_MQTT_TOPIC_EXTRA3];

    }
};


#endif //PLATFORM_GOR_GOR_CONFIG_HPP
