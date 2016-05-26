#include "GLRage.hpp"

namespace glrage {

RuntimePatcher GLRage::m_patcher;

GLRAPI Context& GLRage::getContext()
{
    return ContextImpl::instance();
}

GLRAPI RuntimePatcher& GLRage::getPatcher()
{
    return m_patcher;
}

} // namespace glrage