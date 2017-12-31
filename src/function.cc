#include "function.h"
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

GIArgument Func::CallNative(GIFunctionInfo *function_info, Args &args) {
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
    Args args = Args::Prepare((GICallableInfo *)function_info);
    args.loadJSArguments(js_callback_info);
    if (g_callable_info_is_method(function_info)) {
        args.loadContext(obj);
    }

    Nan::TryCatch exception_handler;
    GIArgument result = Func::CallNative(function_info, args);
    if (exception_handler.HasCaught()) {
        exception_handler.ReThrow();
        return Nan::Undefined();
    }

    GITypeInfo return_type_info;
    g_callable_info_load_return_type(function_info, &return_type_info);

    Local<Value> js_return_value = Args::FromGType(
        &result,
        &return_type_info,
        0 // 0 array size. TODO: can we avoid this? What does it even mean?
    );

    auto name = g_base_info_get_name(function_info);

    if (args.out.size() >= 1) {
        bool skip_return_value = g_callable_info_skip_return(function_info) || g_type_info_get_tag(&return_type_info) == GI_TYPE_TAG_VOID;
        int js_return_array_size = args.out.size();
        if (!skip_return_value) {
            js_return_array_size += 1;
        }
        Local<Array> js_return_array = Nan::New<Array>(js_return_array_size);
        if (!skip_return_value) {
            js_return_array->Set(0, js_return_value);
        }
        for (int i = 0; i < args.out.size(); i++) {
            GIArgInfo out_arg_info;
            g_callable_info_load_arg(function_info, i, &out_arg_info); // FIXME: this will will fail if there are also IN_ARGS.
            GITypeInfo out_arg_type_info;
            g_arg_info_load_type(&out_arg_info, &out_arg_type_info);
            js_return_array->Set(i + (skip_return_value ? 0 : 1), Args::FromGType(&args.out[i], &out_arg_type_info, 0));
        }
        return js_return_array;
    }

    return js_return_value;
}

}
