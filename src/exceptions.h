#pragma once

#include <stdexcept>

namespace gir {

using namespace std;

class NativeGError : public runtime_error {
public:
    NativeGError() : runtime_error("Native GError") {}
    NativeGError(string message) : runtime_error("Native GError: " + message) {}
};

class UnsupportedGIType : public runtime_error {
public:
    UnsupportedGIType() : runtime_error("Unsupported GI Type") {}
    UnsupportedGIType(string message) : runtime_error("Unsupported GI Type: " + message) {}
};

class UnsupportedGValueType : public runtime_error {
public:
    UnsupportedGValueType() : runtime_error("Unsupported GValue Type") {}
    UnsupportedGValueType(string message) : runtime_error("Unsupported GValue Type: " + message) {}
};

class JSArgumentTypeError : public runtime_error {
public:
    JSArgumentTypeError() : runtime_error("Type Error") {}
    JSArgumentTypeError(string message) : runtime_error("Type Error: " + message) {}
};

class JSValueError : public runtime_error {
public:
    JSValueError() : runtime_error("Value Error") {}
    JSValueError(string message) : runtime_error("Value Error: " + message) {}
};

} // namespace gir
