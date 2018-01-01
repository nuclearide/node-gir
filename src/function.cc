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
        // FIXME: we need a better way to handle errors.
        Nan::ThrowError(Nan::New(error->message).ToLocalChecked());
        g_error_free(error);
    }

    return return_value;
}

Local<Value> Func::Call(GObject *obj, GIFunctionInfo *function_info, const Nan::FunctionCallbackInfo<v8::Value> &js_callback_info) {
    // create the arguments for the native function
    Args args = Args::Prepare((GICallableInfo *)function_info);
    args.loadJSArguments(js_callback_info);
    if (g_callable_info_is_method(function_info)) {
        args.loadContext(obj);
    }

    // call the native function and handle errors
    // FIXME: CallNative throwing JS errors doesn't really make sence
    // I'm still learning c++ best practices and i'm not sure how exceptions
    // or failures should be handled. People online seem to say
    // not to use c++ exceptions but I'm not sure what to do :/
    Nan::TryCatch exception_handler;
    GIArgument result = Func::CallNative(function_info, args);
    if (exception_handler.HasCaught()) {
        exception_handler.ReThrow();
        return Nan::Undefined();
    }

    // handle the return value that we should pass back to JS.
    // there are some rules to decide how to handle there output from the native function
    // so we'll use a helper function to handle that logic for us.
    Local<Value> js_return_value = Func::JSReturnValueFromNativeCall(function_info, args, result);

    return js_return_value;
}

/**
 * This function applies some rules as to how return values from a native call should
 * map to the JS function's return values. These rules should follow GJS exactly, if they
 * do not then we should consider it a bug.
 * Rules:
 * - If the native function has a return value and no out-args then just return the return value
 * - If the native function has no return value (skipped or void) and 1 out-arg, return just the out arg as a value
 * - If the native function has no return value (skipped or void) and more than 1 out-arg then return them as an array: [out-arg-1, out-arg-2, ..., out-arg-n]
 * - If the native function has a return value and 1 or more out-args then return them as an array with the return value in position 0: [return-value, out-arg-1, out-arg-2, ..., out-arg-n]
 */
Local<Value> Func::JSReturnValueFromNativeCall(GIFunctionInfo *function_info, Args &args, GIArgument &native_call_result) {
    // load the function's return type info
    GITypeInfo return_type_info;
    g_callable_info_load_return_type(function_info, &return_type_info);

    // if the function's metadata says to skip the return value (meaning the return value is only useful in C)
    // or the return value is void, then we can skip the return value when determining what should be returned
    // from native to JS.
    bool skip_return_value = g_callable_info_skip_return(function_info) || g_type_info_get_tag(&return_type_info) == GI_TYPE_TAG_VOID;
    int number_of_return_values = skip_return_value ? args.out.size() : args.out.size() + 1;

    Local<Array> js_result_array = Nan::New<Array>(number_of_return_values);

    // if we should NOT skip the native return value, then we should convert it to JS and set it in position 0 of the returned value array
    if (!skip_return_value) {
        Local<Value> js_return_value = Args::FromGType(&native_call_result, &return_type_info, 0);
        js_result_array->Set(0, js_return_value);
    }

    // for each out argument, convert it to JS and set it in the return array
    // if we don't have any out args, this loop doesn't need to do anything
    for (int i = 0; i < args.out.size(); i++) {
        GIArgInfo out_arg_info;
        g_callable_info_load_arg(function_info, i, &out_arg_info); // FIXME: this will will fail if there are also IN_ARGS due to incorrect indexing!!!.
        GITypeInfo out_arg_type_info;
        g_arg_info_load_type(&out_arg_info, &out_arg_type_info);
        int position = i + (skip_return_value ? 0 : 1); // if there is a return_value then we need to offset the out args by 1 i.e. [return_value, out-arg-1, out-arg-2, ...]
        js_result_array->Set(position, Args::FromGType(&args.out[i], &out_arg_type_info, 0));
    }

    // based on the number of return values from the native function call we'll decide what to send back to JS
    switch (number_of_return_values) {
        case 0:
            // if there is no return value (skipped return value, void return, and no out-args) then return `undefined`
            return Nan::Undefined();

        case 1:
            // if there's only 1 return value we need to send it to JS as a Value (not an array with 1 value in it)
            return js_result_array->Get(0);

        default:
            // else there must be more then 1 return value meaning we should return them as a JS array
            return js_result_array;
    }
}

}
