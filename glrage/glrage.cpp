#include "GLRage.hpp"

namespace glrage {

ContextImpl GLRage::m_context;
RuntimePatcher GLRage::m_patcher;

GLRAPI Context& GLRage::getContext()
{
    return m_context;
}

GLRAPI RuntimePatcher& GLRage::getPatcher()
{
    return m_patcher;
}

} // namespace glrage