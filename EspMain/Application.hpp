#pragma once
#include <vector>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

class AbstractDevice;

class Application {
public:
    static Application& get() {
        static Application app;
        return app;
    }
    // lifetime is expected to be eternal
    void addDevice(AbstractDevice* sensor);
    void init(String name);
    void loop();
private:
    Application() {}

    void connect(bool firstTime);

    std::vector<AbstractDevice*> mDevices;
    WiFiUDP udp;
    Coap coap{udp, 1024 * 8}; // 8KB Buffer Size
    String mName;
};

