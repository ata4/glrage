#include "GLRage.hpp"

namespace glrage {

GLRAPI Context& GLRage::getContext()
{
    return ContextImpl::instance();
}

GLRAPI RuntimePatcher& GLRage::getPatcher()
{
    return RuntimePatcher::instance();
}

} // namespace glrage