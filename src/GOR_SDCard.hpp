//
// Created by Jordi Brusa on 28.05.17.
//

#ifndef PLATFORM_GOR_GOR_SDCARD_HPP
#define PLATFORM_GOR_GOR_SDCARD_HPP

#include <SD.h>

#define LOG_FILE            "/LOG.TXT"

enum GOR_SD_Errors {
    CARD_MOUNT_FAILED = 1,
    CARD_NOT_PLUGGED
};


class GOR_SDCard {
private:
    uint8_t m_cardType;

public:

    GOR_SDCard() {
    }


    int begin(uint8_t pin) {
        if(!SD.begin(pin)) {
            return GOR_SD_Errors::CARD_MOUNT_FAILED;
        };

        m_cardType = SD.cardType();
        if(m_cardType == CARD_NONE){
            return GOR_SD_Errors::CARD_NOT_PLUGGED;
        }

        return 0;
    }


    void log(String s) {
        File file = SD.open(LOG_FILE, FILE_APPEND);
        file.println(s.c_str());
        file.close();
    }

    uint64_t getCardSize() {
        return SD.cardSize();
    }
};



#endif //PLATFORM_GOR_GOR_CONFIG_HPP
