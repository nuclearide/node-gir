#include "function.h"
#include "arguments.h"
#include "util.h"
#include <nan.h>
#include <vector>

#include "types/object.h"

using namespace v8;

namespace gir {

Local<FunctionTemplate> Func::CreateFunction(GIFunctionInfo *function_info) {
    Local<External> function_info_extern = Nan::New<External>((void *)g_base_info_ref(function_info));
    Local<FunctionTemplate> function_template = Nan::New<FunctionTemplate>(Func::InvokeFunction, function_info_extern);
    return function_template;
}

// FIXME: this needs to be refactored to support anyone creating a function
// that executes the native function specified by GIFunctionInfo with a given GObject
// not just GIRObject's as is the case currently with Func::InvokeMethod!
Local<FunctionTemplate> Func::CreateMethod(GIFunctionInfo *function_info) {
    Local<External> function_info_extern = Nan::New<External>((void *)g_base_info_ref(function_info));
    Local<FunctionTemplate> function_template = Nan::New<FunctionTemplate>(Func::InvokeMethod, function_info_extern);
    return function_template;
}

NAN_METHOD(Func::InvokeFunction) {
    Local<External> function_info_extern = Local<External>::Cast(info.Data());
    GIFunctionInfo *function_info = (GIFunctionInfo *)function_info_extern->Value();
    Local<Value> js_func_result = Func::Call(nullptr, function_info, info);
    info.GetReturnValue().Set(js_func_result);
}

NAN_METHOD(Func::InvokeMethod) {
    if (!info.This()->IsObject()) {
        Nan::ThrowTypeError("the value of 'this' is not an object");
        return;
    }

    GIRObject *that = Nan::ObjectWrap::Unwrap<GIRObject>(info.This()->ToObject());
    GObject *native_object = that->obj;
    Local<External> function_info_extern = Local<External>::Cast(info.Data());
    GIFunctionInfo *function_info = (GIFunctionInfo *)function_info_extern->Value();

    Local<Value> js_func_result = Func::Call(native_object, function_info, info);
    info.GetReturnValue().Set(js_func_result);
}

GIArgument Func::CallNative(GObject *obj, GIFunctionInfo *function_info, const Nan::FunctionCallbackInfo<v8::Value> &js_callback_info) {
    Args args = Args::Prepare((GICallableInfo *)function_info);
    args.loadJSArguments(js_callback_info);
    if (g_callable_info_is_method(function_info)) {
        args.loadContext(obj);
    }

    GIArgument return_value;
    GError *error = nullptr;
    g_function_info_invoke(
        function_info,
        args.in.data(),
        args.in.size(),
        args.out.data(),
        args.out.size(),
        &return_value,
        &error
    );

    if (error != nullptr) {
        Nan::ThrowError(Nan::New(error->message).ToLocalChecked());
        g_error_free(error);
    }

    return return_value;
}

Local<Value> Func::Call(GObject *obj, GIFunctionInfo *function_info, const Nan::FunctionCallbackInfo<v8::Value> &js_callback_info) {
    Nan::TryCatch exception_handler;
    GIArgument result = Func::CallNative(obj, function_info, js_callback_info);
    if (exception_handler.HasCaught()) {
        exception_handler.ReThrow();
        return Nan::Undefined();
    }

    GITypeInfo return_type_info;
    g_callable_info_load_return_type(function_info, &return_type_info);

    Local<Value> js_return_value = Args::FromGType(
        &result,
        &return_type_info,
        0 // 0 array size. TODO: can we avoid have to do this?
    );

    return js_return_value;
}

}
