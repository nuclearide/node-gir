#pragma once

#include <v8.h>
#include <nan.h>
#include <glib.h>
#include <girepository.h>

namespace gir {

using namespace v8;

class Func {
  public:
    static v8::Local<v8::Value> Call(GObject *obj, GIFunctionInfo *info, const Nan::FunctionCallbackInfo<v8::Value>&args, bool ignore_function_name);
    static v8::Handle<v8::Value> CallAndGetPtr(GObject *obj, GIFunctionInfo *info, const Nan::FunctionCallbackInfo<v8::Value>&args, bool ignore_function_name, GIArgument *retval, GITypeInfo **returned_type_info, gint *returned_array_length);

    static Local<FunctionTemplate> CreateFunction(GIFunctionInfo *function_info);
    static Local<FunctionTemplate> CreateMethod(GIFunctionInfo *function_info);

    static NAN_METHOD(InvokeFunction);
    static NAN_METHOD(InvokeMethod);
};

}
