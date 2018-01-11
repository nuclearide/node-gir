#include "./closure.h"
#include "./values.h"
#include "util.h"
#include "arguments.h"
#include "exceptions.h"
#include <sstream>

namespace gir {

/**
 * returns NULL if the signal doesn't exist
 * Perhaps we should throw a JS exception instead?
 * Perhaps we should throw a c++ exception instead?
 * Perhaps we should redesign this function to not have this sort of
 * error condition, it's suppose to be a 'constructor' function (constructing a closure struct)
 * and constructors shouldn't really 'error' like this.
 */
GClosure *GIRClosure::create(GICallableInfo *callable_info,
                                   Local<Function> callback) {
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

GValue ffi_arg_to_g_value(GIArgInfo *argument_info, void *ffi_arg_value) {
    auto argument_type_info = GIRInfoUniquePtr(g_arg_info_get_type(argument_info));
    auto name = g_base_info_get_name(argument_info);
    auto argument_type_tag = g_type_info_get_tag(argument_type_info.get());
    GValue gvalue = G_VALUE_INIT;
    switch (argument_type_tag) {
        case GI_TYPE_TAG_BOOLEAN:
            // gvalue.v_boolean = *(gboolean*) args[i];
            break;
        case GI_TYPE_TAG_INT8:
            // gvalue.v_int8 = *(gint8*) args[i];
            break;
        case GI_TYPE_TAG_UINT8:
            // gvalue.v_uint8 = *(guint8*) args[i];
            break;
        case GI_TYPE_TAG_INT16:
            // gvalue.v_int16 = *(gint16*) args[i];
            break;
        case GI_TYPE_TAG_UINT16:
            // gvalue.v_uint16 = *(guint16*) args[i];
            break;
        case GI_TYPE_TAG_INT32:
            // gvalue.v_int32 = *(gint32*) args[i];
            break;
        case GI_TYPE_TAG_UINT32:
            // gvalue.v_uint32 = *(guint32*) args[i];
            break;
        case GI_TYPE_TAG_INT64:
            // gvalue.v_int64 = *(gint64*) args[i];
            break;
        case GI_TYPE_TAG_UINT64:
            // gvalue.v_uint64 = *(guint64*) args[i];
            break;
        case GI_TYPE_TAG_FLOAT:
            // gvalue.v_float = *(gfloat*) args[i];
            break;
        case GI_TYPE_TAG_DOUBLE:
            // gvalue.v_double = *(gdouble*) args[i];
            break;
        case GI_TYPE_TAG_UTF8:
            g_value_init(&gvalue, G_TYPE_STRING);
            g_value_set_string(&gvalue, (gchar *)ffi_arg_value);
            printf("%s", g_value_get_string(&gvalue));
            break;
        case GI_TYPE_TAG_VOID:
            gvalue.g_type = G_TYPE_NONE;
            // g_value_set_pointer(&gvalue, ffi_arg_value); // FIXME: what should this really be? nullptr?
            break;
        default:
            stringstream message;
            message << "Callback closure doesn't support arguments of type '"
                    << g_type_tag_to_string(argument_type_tag) << "' yet";
            throw UnsupportedGIType(message.str());
    }
    return gvalue;
}

void GIRClosure::ffi_closure_callback(ffi_cif *cif, void *result, void **args, gpointer user_data) {
    GIRClosure *gir_closure = static_cast<GIRClosure *>(user_data);
    Local<Function> js_callback = Nan::New<Function>(gir_closure->callback);
    Nan::Call(js_callback, Nan::GetCurrentContext()->Global(), 0, nullptr);
}

ffi_closure *GIRClosure::create_ffi(GICallableInfo *callable_info,
                                          Local<Function> js_callback) {
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
        // we need to get the native parameter
        GValue native_param = param_values[i];

        // then get some GI information for this argument (find it's Type)
        // we can get this information from the original callable_info that the
        // signal_closure was created for.
        GIArgInfo *arg_info = g_callable_info_get_arg(gir_signal_closure->callable_info,
                                                      i); // FIXME: CRITICAL: we should assert that the length of the
                                                          // callable params matches the n_param_values to avoid an
                                                          // array overrun!!!!!!!
        GITypeInfo *type_info = g_arg_info_get_type(arg_info);

        // convert the native GValue to a v8::Value
        Local<Value> js_param = GIRValue::from_g_value(&native_param,
                                                       type_info);

        // clean up memory
        // FIXME: should use unique_ptrs for exception safety
        g_base_info_unref(arg_info);
        g_base_info_unref(type_info);

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
