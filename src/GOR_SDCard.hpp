//
// Created by Jordi Brusa on 28.05.17.
//

#ifndef PLATFORM_GOR_GOR_CONFIG_HPP
#define PLATFORM_GOR_GOR_CONFIG_HPP

#include <vector>
#include <SD.h>

#define CONFIG_FILE         "CONFIG.TXT"
#define LOG_FILE            "LOG.TXT"

#define READ_NAME           1
#define READ_VAL            2
#define MAX_NAME_LENGTH     128
#define MAX_VALUE_LENGTH    512

using namespace std;

enum GOR_Config_Errors {
    CARD_MOUNT_FAILED = 1,
    CARD_NOT_PLUGGED,
    FILE_NOT_FOUND
};

struct GOR_Setting {
public:
    String name;
    String value;

    GOR_Setting(String n, String v) : name(n), value(v) {};
};

class GOR_SDCard {
private:

    vector<GOR_Setting> m_settings;
    uint8_t m_cardType;

public:

    GOR_SDCard() {
    }


    int begin(uint8_t pin) {
        if(!SD.begin(pin)) {
            return GOR_Config_Errors::CARD_MOUNT_FAILED;
        };

        m_cardType = SD.cardType();
        if(m_cardType == CARD_NONE){
            return GOR_Config_Errors::CARD_NOT_PLUGGED;
        }

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
                GOR_Setting s(settingName, settingValue);
                m_settings.push_back(s);

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

        return 0;
    }


    String operator [](const String name) { return getSetting(name); }

    String getSetting(const String name) {
        for(uint16_t i=0; i<m_settings.size(); i++) {
            if(m_settings[i].name == name) {
                return m_settings[i].value;
            }
        }

        return "";
    }

    void setSetting(const String name, const String value) {
        bool found = false;

        // Mise à jour du setting, s'il existe
        for(uint16_t i=0; i<m_settings.size(); i++) {
            if(m_settings[i].name == name) {
                m_settings[i].value = value;
                found = true;
            }
        }

        // Le setting n'existe pas, ajout
        if(!found) {
            GOR_Setting s(name, value);
            m_settings.push_back(s);
        }

        // Re-création du fichier de config
        SD.remove(CONFIG_FILE);
        File file = SD.open(CONFIG_FILE, FILE_WRITE);

        for(uint16_t i=0; i<m_settings.size(); i++) {
            file.println(m_settings[i].name + "=" + m_settings[i].value);
        }

        file.close();
    }

    void log(String s) {
        File file = SD.open(LOG_FILE, FILE_APPEND);
        file.println(s.c_str());
        file.close();
    }

    uint64_t getCardSize() {
        return SD.cardSize();
    }


    unsigned int getSettingCount() {
        return m_settings.size();
    }

};



#endif //PLATFORM_GOR_GOR_CONFIG_HPP
