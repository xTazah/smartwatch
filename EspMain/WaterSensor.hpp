#include "AbstractDevice.hpp"

class WaterSensor: public AbstractDevice{
    String type() override;
    String name() override;
    void sendToServer(JsonDocument& out) override;
    void recvFromServer(const JsonDocument& in, JsonDocument& out) override;
};