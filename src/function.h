#pragma once

#include <girepository.h>
#include <glib.h>
#include <nan.h>
#include <v8.h>
#include "arguments.h"

namespace gir {

using namespace v8;

class Func {
public:
    static GIArgument call_native(GIFunctionInfo *function_info, Args &function_arguments);
    static v8::Local<v8::Value> call(GObject *obj,
                                     GIFunctionInfo *function_info,
                                     const Nan::FunctionCallbackInfo<v8::Value> &args);

    static Local<FunctionTemplate> create_function(GIFunctionInfo *function_info);
    static Local<FunctionTemplate> create_method(GIFunctionInfo *function_info);

    static NAN_METHOD(InvokeFunction);
    static NAN_METHOD(InvokeMethod);

private:
    static Local<Value> js_return_value_from_native_call(GIFunctionInfo *function_info,
                                                         Args &args,
                                                         GIArgument &native_call_result);
};

} // namespace gir
