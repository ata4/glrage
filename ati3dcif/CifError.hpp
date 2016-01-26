#pragma once

#include "ati3dcif.h"

#include <stdexcept>
#include <string>

namespace cif {

class CifError : public std::runtime_error {
public:
    CifError(const char* message, C3D_EC errorCode);
    CifError(const std::string& message, C3D_EC errorCode);
    CifError(const char* message);
    CifError(const std::string& message);
    C3D_EC getErrorCode() const;
    const char* getErrorName() const;

private:
    C3D_EC m_errorCode;
};

}