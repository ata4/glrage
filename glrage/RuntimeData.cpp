#include "RuntimeData.hpp"
#include <sstream>

namespace glrage {

RuntimeData::RuntimeData() {
}

RuntimeData::RuntimeData(const std::vector<uint8_t>& data) {
    m_data = data;
}

void RuntimeData::clear() {
    m_data.clear();
}

const std::vector<uint8_t>& RuntimeData::data() const {
    return m_data;
}

}