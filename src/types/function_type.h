#pragma once

#include <map>
#include <v8.h>
#include <nan.h>
#include <glib.h>
#include <girepository.h>

namespace gir {

class GIRFunction : public Nan::ObjectWrap {
  public:
    GIRFunction() {};

    static void initialize(v8::Handle<v8::Object> target, GIObjectInfo *info);
    static NAN_METHOD(Execute);
    static char *to_camel_case(const char *str);

   private:
};

}
