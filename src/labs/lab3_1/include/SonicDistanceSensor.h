#pragma once

#include "ISensor.h"

class SonicDistanceSensor : public ISensor {
private:
    int trigPin;
    int echoPin;
public:
    SonicDistanceSensor(int trig, int echo);
    float read() override;
};
