#pragma once

#include <girepository.h>
#include <glib.h>
#include <nan.h>
#include <v8.h>
#include <map>

namespace gir {

using namespace v8;

class GIRFunction : public Nan::ObjectWrap {
public:
    GIRFunction() = default;
    static Local<Function> prepare(GIFunctionInfo *info);
};

} // namespace gir
