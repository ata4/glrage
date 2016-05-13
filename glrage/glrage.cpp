#include "GLRage.hpp"

namespace glrage {

ContextImpl GLRage::m_context;
RuntimePatcherMain GLRage::m_patcher;

GLRAPI Context& GLRage::getContext()
{
    return m_context;
}

GLRAPI RuntimePatcherMain& GLRage::getPatcher()
{
    return m_patcher;
}

} // namespace glrage