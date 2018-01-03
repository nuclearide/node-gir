#pragma once

#include <girepository.h>
#include <glib.h>
#include <nan.h>
#include <v8.h>
#include <map>

namespace gir {

class GIRFunction : public Nan::ObjectWrap {
public:
    GIRFunction() = default;

    static void initialize(v8::Handle<v8::Object> target, GIObjectInfo *info);
};

} // namespace gir
