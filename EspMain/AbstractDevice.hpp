#pragma once
#include <ArduinoJson.h>


class AbstractDevice {
public:
    virtual String name() = 0;
    virtual String type() = 0;
    /*{
        return "humidity_temp_sensor";
    }*/

    virtual void init() {}
    virtual void loop() {}

    virtual void wifiDisonnected() {};
    virtual void wifiConnected() {};
    
    virtual void sendToServer(JsonDocument& out) = 0;
    /*{
        out["humidity_percent"] = 20;
        out["temp_c"] = 22;
    }*/

    virtual void recvFromServer(const JsonDocument& in, JsonDocument& out) = 0;
    /*{
        if(doc["command"] == "set_precision") {
            //...
        }
    }*/
private:

};