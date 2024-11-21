#include "Application.hpp"
#include "AbstractDevice.hpp"
// if this file is missing, copy template_secret.h to secret.h and insert your own values.
#include "secret.h"
#include "ESPmDNS.h"

void Application::init(String name) {
    mName = name;
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    for(auto device: mDevices) {
        // post command for a device
        coap.server([this, device](CoapPacket &packet, IPAddress ip, int port) {
            StaticJsonDocument<1024> response;
            auto sendResponse = [&] {
                String responseStr;
                serializeJson(response, responseStr);
                coap.sendResponse(ip, port, packet.messageid, responseStr.c_str(), responseStr.length(), 
                    COAP_CONTENT, COAP_APPLICATION_JSON, packet.token, packet.tokenlen);
            };
            if(packet.code != COAP_POST) {
                response["type"] = "error";
                response["msg"] = "Unknown request type";
                sendResponse();
                return;
            }
            StaticJsonDocument<4096> request;
            auto deserializeStatus = deserializeJson(request, packet.payload, packet.payloadlen);
            if(deserializeStatus.code() != DeserializationError::Ok) {
                response["type"] = "error";
                response["msg"] = deserializeStatus.c_str();
                sendResponse();
                return;
            }
            response["type"] = "ok";
            device->recvFromServer(request, response);
            sendResponse();
        }, String(mName + "/device/") + device->name() + "/command");

        // read sensor value
        coap.server([this, device](CoapPacket &packet, IPAddress ip, int port) {
            StaticJsonDocument<1024> response;
            device->sendToServer(response);

            String responseStr;
            serializeJson(response, responseStr);
            coap.sendResponse(ip, port, packet.messageid, responseStr.c_str(), responseStr.length(), 
                COAP_CONTENT, COAP_APPLICATION_JSON, packet.token, packet.tokenlen);
        }, String(mName + "/device/") + device->name() + "/value");
    }

    // get all devices
    coap.server([this](CoapPacket &packet, IPAddress ip, int port) {
        Serial.println("Got device request with message id " + String(packet.messageid));
        StaticJsonDocument<1024> response;
        auto sendResponse = [&] {
            String responseStr;
            serializeJson(response, responseStr);
            auto sendStatus = coap.sendResponse(ip, port, packet.messageid, responseStr.c_str(), responseStr.length(), 
                COAP_CONTENT, COAP_APPLICATION_JSON, packet.token, packet.tokenlen);
            //coap.sendResponse(ip, port, packet.messageid, responseStr.c_str(), responseStr.length());
            if(sendStatus == 0) {
                Serial.println("Failed to send reply");
            } else {
                Serial.println("Resonse to message sent with id " + String(packet.messageid) + ": " + responseStr);

            }
        };
        if(packet.code != COAP_GET) {
            response["type"] = "error";
            response["msg"] = "Unknown request type";
            sendResponse();
            return;
        }
        response["type"] = "ok";
        auto devices = response.createNestedObject("devices");
        for(size_t i = 0; i < mDevices.size(); ++i) {
            auto description = devices.createNestedObject(mDevices.at(i)->name());
            description["type"] = mDevices.at(i)->type();
        }
        sendResponse();
    }, mName + "/device");

    // get all sensor values (this is the block transfer code)
    // each json documented is delimited with a null byte/terminator
    coap.server([this](CoapPacket &packet, IPAddress ip, int port) {
        String responseStr;

        for(auto d: mDevices) {
            StaticJsonDocument<1024> response;
            response["device_type"] = d->type();
            response["device_name"] = d->name();
            d->sendToServer(response);
            String responsePart;
            serializeJson(response, responsePart);
            responseStr += responsePart;
            responseStr += '\0';
        }

        coap.sendResponse(ip, port, packet.messageid, responseStr.c_str(), responseStr.length(), 
            COAP_CONTENT, COAP_APPLICATION_JSON, packet.token, packet.tokenlen);
    }, mName + "/all_device_values");
    
    // default response handler
    coap.response([&](CoapPacket &packet, IPAddress, int) {
        Serial.println("Got response");
    });
    // init all devices
    for(auto dev: mDevices) {
        dev->init();
    }
    // connect to wifi
    connect(true);
    Serial.println("Init of application done!");
}

void Application::connect(bool firstTime) {
    if (!firstTime && WiFi.status() == WL_CONNECTED) {
        return;
    }

    if(!firstTime) {
        for(auto d: mDevices) {
            d->wifiDisonnected();
        }
    }

    WiFi.setHostname(mName.c_str());

    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1);
        for(auto d: mDevices) {
            d->loop();
        }
        Serial.print(".");
    }

    Serial.println("WiFi connected!");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    MDNS.begin(mName.c_str());
    MDNS.addService(mName.c_str(), "udp", COAP_PORT);

    
    coap.start(COAP_PORT);

    for(auto d: mDevices) {
        d->wifiConnected();
    }
}

void Application::loop() {
    connect(false);
    coap.loop();
    for(auto d: mDevices) {
        d->loop();
    }
    // this is to keep the rtcos stuff happy, without this line we get random crashes 
    delay(1);
}

void Application::addDevice(AbstractDevice* sensor){
    mDevices.push_back(sensor);
};