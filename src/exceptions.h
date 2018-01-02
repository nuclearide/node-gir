#pragma once

#include <stdexcept>

namespace gir {

using namespace std;

class UnsupportedGIType: public runtime_error {
public:
    UnsupportedGIType() : runtime_error("Unsupported GI Type") {}
    UnsupportedGIType(string message) : runtime_error(message) {}
};

}
