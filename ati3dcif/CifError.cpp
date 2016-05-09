#include "CifError.hpp"
#include "CifUtils.hpp"

namespace cif {

CifError::CifError(const char* message, C3D_EC errorCode)
    : std::runtime_error(message)
    , m_errorCode(errorCode)
{
}

CifError::CifError(const std::string& message, C3D_EC errorCode)
    : CifError(message.c_str(), errorCode)
{
}

CifError::CifError(const char* message)
    : CifError(message, C3D_EC_GENFAIL)
{
}

CifError::CifError(const std::string& message)
    : CifError(message.c_str(), C3D_EC_GENFAIL)
{
}

C3D_EC
CifError::getErrorCode() const
{
    return m_errorCode;
}

const char* CifError::getErrorName() const
{
    return C3D_EC_NAMES[m_errorCode];
}

} // namespace cif