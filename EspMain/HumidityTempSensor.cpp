#include "HumidityTempSensor.hpp"
#include "AbstractDevice.hpp"
#include <DHT.h>
#include <DHT_U.h>
#include "secret.h"

DHT dht(DHTPIN, DHTTYPE);

String HumidityTempSensor::name(){
    return "hum-temp-sensor-1";
};

String HumidityTempSensor::type(){
    return "DHT11";
};
void HumidityTempSensor::init() {
    dht.begin();
}

void HumidityTempSensor::sendToServer(JsonDocument& out){
    auto temp = dht.readTemperature(false);
    if(isnan(temp)){
        out["type"] = "error";
        out["msg"] = "Error reading temperature";
        return;
    }
    out["temperature_degC"] = temp;
    
    auto humidity = dht.readHumidity();
    if(isnan(humidity)){
        out["type"] = "error";
        out["msg"] = "Error reading humidity";
        return;
    }
    out["humidity"] = humidity;
}

void HumidityTempSensor::recvFromServer(const JsonDocument& in, JsonDocument& out){

}