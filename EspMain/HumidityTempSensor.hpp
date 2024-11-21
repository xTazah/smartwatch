#include "AbstractDevice.hpp"

class HumidityTempSensor: public AbstractDevice{
    String type() override;
    String name() override;
    void init() override;
    void sendToServer(JsonDocument& out) override;
    void recvFromServer(const JsonDocument& in, JsonDocument& out) override;
};