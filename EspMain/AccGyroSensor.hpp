#include "AbstractDevice.hpp"

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// AccGyroSensor
class AccGyroSensor : public AbstractDevice {
public:
    AccGyroSensor(String n) {
        mSensorName = n;
    }

    String name() override {
        return mSensorName;
    }
    String type() override {
        return "acceleration_gyroscope_sensor";
    }

    void init() override {
        Serial.println("MP6050 init!");

        if (!mMPU.begin()) {
            //Fehler falls Sensor nicht gefunden werden konnte
            Serial.println("Failed to find MPU6050 chip");
            mFoundChip = false;
            return;
        }
        mFoundChip = true;

        //Setzen der Arbeitsbereiche
        mMPU.setAccelerometerRange(MPU6050_RANGE_8_G);
        mMPU.setGyroRange(MPU6050_RANGE_500_DEG);
        mMPU.setFilterBandwidth(MPU6050_BAND_21_HZ);
    }

    void recvFromServer(const JsonDocument &in, JsonDocument &out) override {
        if(!mFoundChip) {
            out["type"] = "error";
            out["msg"] = "MPU6050 not found";
            return;
        }
        if(in["command"] == "set_accel_rang") {
            mMPU.setAccelerometerRange(in["accel_range"]);
        }
    }

    void sendToServer(JsonDocument &out) override {
        //sende Fehlermeldung an Server falls Sensor nicht gefunden wurde
        if(!mFoundChip) {
            out["type"] = "error";
            out["msg"] = "MPU6050 not found";
            return;
        }
        sensors_event_t a, g, temp;
        mMPU.getEvent(&a, &g, &temp); // hole die Daten vom Sensor
        // Beschleunigungssensor
        out["x_acceleration_m/s^2"] = a.acceleration.x;
        out["y_acceleration_m/s^2"] = a.acceleration.y;
        out["z_acceleration_m/s^2"] = a.acceleration.z;
        // Gyroskop
        out["a_rotation_deg/s"] = g.gyro.x;
        out["b_rotation_deg/s"] = g.gyro.y;
        out["c_rotation_deg/s"] = g.gyro.z;
        // Temperatursensor
        out["temperature_degC"] = temp.temperature;
    }

private:
    String mSensorName;
    Adafruit_MPU6050 mMPU;
    bool mFoundChip{false};
};