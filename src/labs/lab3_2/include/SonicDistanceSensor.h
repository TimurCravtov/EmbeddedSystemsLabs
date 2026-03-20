#ifndef SONIC_DISTANCE_SENSOR_H
#define SONIC_DISTANCE_SENSOR_H

#include "ISensor.h"

class SonicDistanceSensor : public ISensor {
private:
    int trigPin;
    int echoPin;
public:
    SonicDistanceSensor(int trig, int echo);
    float read() override;
};

#endif // SONIC_DISTANCE_SENSOR_H
