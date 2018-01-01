#pragma once

#include <v8.h>
#include <nan.h>
#include <glib.h>
#include <girepository.h>
#include "arguments.h"

namespace gir {

using namespace v8;

class Func {
  public:
    static GIArgument CallNative(GIFunctionInfo *function_info, Args &function_arguments);
    static v8::Local<v8::Value> Call(GObject *obj, GIFunctionInfo *function_info, const Nan::FunctionCallbackInfo<v8::Value>&args);

    static Local<FunctionTemplate> CreateFunction(GIFunctionInfo *function_info);
    static Local<FunctionTemplate> CreateMethod(GIFunctionInfo *function_info);

    static NAN_METHOD(InvokeFunction);
    static NAN_METHOD(InvokeMethod);

  private:
    static Local<Value> JSReturnValueFromNativeCall(GIFunctionInfo *function_info, Args &args, GIArgument &native_call_result);
};

}
