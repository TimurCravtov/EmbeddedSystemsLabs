#pragma once

class ISensor {
public:
    virtual ~ISensor() {}
    virtual float read() = 0;
};
