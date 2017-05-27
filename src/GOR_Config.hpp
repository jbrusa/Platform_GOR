//
// Created by Jordi Brusa on 28.05.17.
//

#ifndef PLATFORM_GOR_GOR_CONFIG_HPP
#define PLATFORM_GOR_GOR_CONFIG_HPP

#include <vector>
#include <SD.h>

#define CONFIG_FILE         "CONFIG.TXT"
#define CONFIG_SD_CD_PIN    2

using namespace std;

enum GOR_Config_Errors {
    CARD_MOUNT_FAILED = 1,
    CARD_NOT_PLUGGED,
    FILE_NOT_FOUND
};

struct GOR_Setting {
    String name;
    String value;
};

class GOR_Config {
private:

    vector<GOR_Setting> m_settings;
    uint8_t m_cardType;

public:

    GOR_Config() {
    }


    int begin() {
        if(!SD.begin(CONFIG_SD_CD_PIN)) {
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

        return 0;
    }


    String operator [](const String name) { return get(name); }

    String get(const String name) {
        for(uint16_t i=0; i<m_settings.size(); i++) {
            if(m_settings[i].name == name) {
                return m_settings[i].value;
            }
        }

        return "";
    }


    uint64_t getCardSize() {
        return SD.cardSize();
    }


    unsigned int getSettingCount() {
        return m_settings.size();
    }

};

GOR_Config GOR_Config;


#endif //PLATFORM_GOR_GOR_CONFIG_HPP
