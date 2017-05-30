//
// Created by Jordi Brusa on 27.05.17.
//

#include "GOR_Platform.hpp"

String GOR_HeartbeatPayload() {
    return "{\"nodename\":\"" + gor.config.nodeName + "\"}";
}


void GOR_MessageReceived(String topic, String payload) {

}


void GOR_Setup() {

}


void GOR_Loop() {

}