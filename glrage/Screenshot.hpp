#pragma once

#include <cstdint>

namespace glrage {

class Screenshot {
public:
    Screenshot();
    void schedule();
    void captureScheduled();
    void capture();
private:
    uint32_t m_index;
    bool m_schedule;
};

}