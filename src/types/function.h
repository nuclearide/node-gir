#pragma once

#include <girepository.h>
#include <glib.h>
#include <nan.h>
#include <v8.h>
#include <map>
#include "arguments.h"

namespace gir {

using namespace v8;

class GIRFunction : public Nan::ObjectWrap {
public:
    static Local<Function> prepare(GIFunctionInfo *info);
    static Local<FunctionTemplate> create_function(GIFunctionInfo *function_info);
    static Local<FunctionTemplate> create_method(GIFunctionInfo *function_info);

public:
    // call_native and call should be private
    // we are just waiting for GIRStruct to be rewritten
    static GIArgument call_native(GIFunctionInfo *function_info, Args &function_arguments);
    static v8::Local<v8::Value> call(GObject *obj,
                                     GIFunctionInfo *function_info,
                                     const Nan::FunctionCallbackInfo<v8::Value> &args);

private:
    GIRFunction() = default;
    static Local<Value> js_return_value_from_native_call(GIFunctionInfo *function_info,
                                                         Args &args,
                                                         GIArgument &native_call_result);
    static NAN_METHOD(InvokeFunction);
    static NAN_METHOD(InvokeMethod);
};

} // namespace gir
