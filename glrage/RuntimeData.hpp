#pragma once

#include <vector>
#include <cstdint>

namespace glrage {

class RuntimeData {
public:
    RuntimeData();
    RuntimeData(const std::vector<uint8_t>& data);

    void clear();
    const std::vector<uint8_t>& data() const;

    template<class T>
    friend RuntimeData& operator<<(RuntimeData& data, T value) {
        auto valueBytes = reinterpret_cast<uint8_t const*>(&value);
        for (size_t i = 0; i < sizeof(T); i++) {
            data.m_data.push_back(valueBytes[i]);
        }
        return data;
    }

private:
    std::vector<uint8_t> m_data;
};

}