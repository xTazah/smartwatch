#include "WaterSensor.hpp"
#include "AbstractDevice.hpp"
#include "secret.h"


String WaterSensor::name(){
    return "water-sensor-1";
};

String WaterSensor::type(){
    return "HW-038";
};

void WaterSensor::sendToServer(JsonDocument& out){
     int value = analogRead(HWPIN);
     if(isnan(value)){
        out["type"] = "error";
        out["msg"] = "Error reading sweat level";
     }
    out["value"] = value;
    //empirically determined value
    if(value > SWEAT_THRESHHOLD){
        out["sweating"] = true;
    }else{
        out["sweating"] = false;
    }
}

void WaterSensor::recvFromServer(const JsonDocument& in, JsonDocument& out){

}