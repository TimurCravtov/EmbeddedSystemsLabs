#ifndef ISENSOR_H
#define ISENSOR_H

class ISensor {
public:
    virtual ~ISensor() {}
    virtual float read() = 0;
};

#endif // ISENSOR_H
