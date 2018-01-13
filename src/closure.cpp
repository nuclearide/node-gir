#include "closure.h"
#include <sstream>
#include "arguments.h"
#include "exceptions.h"
#include "util.h"
#include "values.h"

namespace gir {

/**
 * returns NULL if the signal doesn't exist
 * Perhaps we should throw a JS exception instead?
 * Perhaps we should throw a c++ exception instead?
 * Perhaps we should redesign this function to not have this sort of
 * error condition, it's suppose to be a 'constructor' function (constructing a closure struct)
 * and constructors shouldn't really 'error' like this.
 */
GClosure *GIRClosure::create(GICallableInfo *callable_info, Local<Function> callback) {
    // create a custom GClosure
    GClosure *closure = g_closure_new_simple(sizeof(GIRClosure), nullptr);
    GIRClosure *gir_signal_closure = (GIRClosure *)closure;

    // connect the finalaize notifier and marshaller
    g_closure_add_finalize_notifier(closure, nullptr, GIRClosure::finalize_handler);
    g_closure_set_marshal(closure, GIRClosure::closure_marshal);

    gir_signal_closure->callback = PersistentFunction(callback);
    g_base_info_ref(callable_info); // FIXME: where do we unref, we should use a smart pointer
    gir_signal_closure->callable_info = callable_info;

    return closure;
}

void GIRClosure::ffi_closure_callback(ffi_cif *cif, void *result, void **args, gpointer user_data) {
    GIRClosure *gir_closure = static_cast<GIRClosure *>(user_data);
    int n_native_args = g_callable_info_get_n_args(gir_closure->callable_info);
    vector<Local<Value>> js_args;
    js_args.reserve(n_native_args);
    // FIXME: no clue why this works but it does.
    // if someone could explain it (or show why it's likely very broken)
    // that'd be amazing.
    GIArgument **gi_args = reinterpret_cast<GIArgument **>(args);
    for (int i = 0; i < n_native_args; i++) {
        auto arg_info = GIRInfoUniquePtr(g_callable_info_get_arg(gir_closure->callable_info, i));
        auto arg_type_info = GIRInfoUniquePtr(g_arg_info_get_type(arg_info.get()));
        if (g_type_info_get_tag(arg_type_info.get()) == GI_TYPE_TAG_VOID) {
            // skip void arguments
            continue;
        }
        js_args.push_back(Args::from_g_type(gi_args[i], arg_type_info.get(), 0));
    }
    Local<Function> js_callback = Nan::New<Function>(gir_closure->callback);
    Nan::Call(js_callback, Nan::GetCurrentContext()->Global(), js_args.size(), js_args.data());
}

ffi_closure *GIRClosure::create_ffi(GICallableInfo *callable_info, Local<Function> js_callback) {
    ffi_cif *cif = new ffi_cif(); // FIXME: where do we free this
    GClosure *gclosure = GIRClosure::create(callable_info, js_callback);
    return g_callable_info_prepare_closure(callable_info, cif, GIRClosure::ffi_closure_callback, gclosure);
}

void GIRClosure::closure_marshal(GClosure *closure,
                                 GValue *return_value,
                                 guint n_param_values,
                                 const GValue *param_values,
                                 gpointer invocation_hint,
                                 gpointer marshal_data) {
    GIRClosure *gir_signal_closure = (GIRClosure *)closure;
    Nan::HandleScope scope;

    // create a list of JS values to be passed as arguments to the callback.
    // the list will be created from using the param_values array.
    vector<Local<Value>> callback_argv(n_param_values);

    // for each value in param_values, convert to a Local<Value> using
    // converters defined in values.h for GValue -> v8::Value conversions.
    for (guint i = 0; i < n_param_values; i++) {
        // then get some GI information for this argument (find it's Type)
        // we can get this information from the original callable_info that the
        // signal_closure was created for.
        auto arg_info = GIRInfoUniquePtr(g_callable_info_get_arg(gir_signal_closure->callable_info,
                                                                 i)); // FIXME: CRITICAL: we should assert that the
                                                                      // length of the callable params matches the
                                                                      // n_param_values to avoid an array overrun!!!!!!!
        auto type_info = GIRInfoUniquePtr(g_arg_info_get_type(arg_info.get()));

        // convert the native GValue to a v8::Value
        Local<Value> js_param = GIRValue::from_g_value(&param_values[i], type_info.get());

        // put the value into 'argv', ready for the callback!
        callback_argv[i] = js_param;
    }

    // get a local reference to the closure's callback (a JS function)
    Local<Function> local_callback = Nan::New<Function>(gir_signal_closure->callback);

    // Call the function. We will pass 'global' as the value of 'this' inside the callback
    // Generally people should never use the value of 'this' in a callback function as it's
    // unreliable (funtion binds, arrow functions are better). If we could set 'this' to 'undefined'
    // then that would be better than setting it to 'global' to make it clear we don't intend
    // for people to use it!
    Nan::MaybeLocal<Value> maybe_result = Nan::Call(local_callback,
                                                    Nan::GetCurrentContext()->Global(),
                                                    n_param_values,
                                                    callback_argv.data());

    // handle the result of the JS callback call
    if (maybe_result.IsEmpty() || maybe_result.ToLocalChecked()->IsNull() ||
        maybe_result.ToLocalChecked()->IsUndefined()) {
        // we don't have a return value
        return_value = nullptr; // set the signal return value to NULL
        return;
    } else {
        // we have a return value
        Local<Value> result = maybe_result.ToLocalChecked();
        GValue g_value = GIRValue::to_g_value(result, G_VALUE_TYPE(return_value));
        g_value_copy(&g_value, return_value);
        return;
    }
}

/**
 * this handler gets called when a GIRClosure is ready to be
 * totally freed. We need to clean up memory and other resources associated
 * with the GIRClosure in this function
 */
void GIRClosure::finalize_handler(gpointer notify_data, GClosure *closure) {
    GIRClosure *gir_signal_closure = (GIRClosure *)closure;

    // unref (free) the GI callable_info
    g_base_info_unref(gir_signal_closure->callable_info);

    // reset (free) the JS persistent function
    gir_signal_closure->callback.Reset();
}

} // namespace gir
