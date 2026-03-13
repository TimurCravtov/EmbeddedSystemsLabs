#pragma once
#include <stdint.h>

template <typename Derived>
class GenericTask {
public:
    void run(void* parameters = nullptr) {
        static_cast<Derived*>(this)->_run(parameters);
    }
};
