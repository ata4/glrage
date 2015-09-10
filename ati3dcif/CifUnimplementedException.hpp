#pragma once

#include <stdexcept>
#include <string>

namespace cif {

class CifUnimplementedException : public std::logic_error {
public:
    CifUnimplementedException(const std::string& msg) : std::logic_error(msg) {}
    CifUnimplementedException(const char* msg) : std::logic_error(msg) {}
};

}