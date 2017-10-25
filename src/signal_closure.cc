#include "./signal_closure.h"
#include "./values.h"

namespace gir {

static void gir_signal_closure_marshal(GClosure *closure,
                                       GValue *return_value,
                                       guint n_param_values,
                                       const GValue *param_values,
                                       gpointer invocation_hint,
                                       gpointer marshal_data) {
  GIRSignalClosure *gir_signal_closure = (GIRSignalClosure *)closure;
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
    // we can get this information from the original signal_info that the
    // signal_closure was created for.
    GIArgInfo *arg_info = g_callable_info_get_arg(gir_signal_closure->signal_info, i); // FIXME: CRITICAL: we should assert that the length of the callable params matches the n_param_values to avoid an array overrun!!!!!!!
    GITypeInfo *type_info = g_arg_info_get_type(arg_info);

    // convert the native GValue to a v8::Value
    Local<Value> js_param = GIRValue::FromGValue(&native_param, type_info); // TODO: this can throw nan errors if the conversion fails. how should we handle cleaning up memory if this happens?

    // clean up memory
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
  Nan::MaybeLocal<Value> maybe_result = Nan::Call(local_callback, Nan::GetCurrentContext()->Global(), n_param_values, callback_argv.data());

  // handle the result of the JS callback call
  if (maybe_result.IsEmpty() || maybe_result.ToLocalChecked()->IsNullOrUndefined()) {
    // we don't have a return value
    return_value = NULL; // set the signal return value to NULL
    return;
  } else {
    // we have a return value
    Local<Value> result = maybe_result.ToLocalChecked(); // this is safe because we checked if the handle was empty!

    // attempt to convert the Local<Value> to a the return_value's GValue type.
    // if the conversion fails we'll throw an exception to JS land.
    // if the conversion is successful, then the return_value will be set!
    if (!GIRValue::ToGValue(result, G_IS_VALUE(return_value), return_value)) {
      Nan::ThrowError("cannot convert return value of callback to a GI type");
    }
    return;
  }
}

/**
 * this handler gets called when a GIRSignalClosure is ready to be
 * totally freed. We need to clean up memory and other resources associated
 * with the GIRSignalClosure in this function
 */
void gir_signal_closure_finalize_handler(gpointer notify_data, GClosure *closure) {
  GIRSignalClosure *gir_signal_closure = (GIRSignalClosure *)closure;

  // unref (free) the GI signal_info
  g_base_info_unref(gir_signal_closure->signal_info);

  // reset (free) the JS persistent function
  gir_signal_closure->callback.Reset();
}

/**
 * returns NULL if the signal can't be found.
 */
GISignalInfo* gir_get_signal(GType signal_g_type, const char* signal_name) {
  GIRepository *repository = g_irepository_get_default();
  GIBaseInfo *target_info = g_irepository_find_by_gtype(repository, signal_g_type);
  if (!target_info) {
    return NULL;
  }
  GISignalInfo *signal_info = NULL;
  if (GI_IS_OBJECT_INFO(target_info)) {
    signal_info = g_object_info_find_signal((GIObjectInfo *)target_info, signal_name);
  }
  else if (GI_IS_INTERFACE_INFO(target_info)) {
    signal_info = g_interface_info_find_signal((GIInterfaceInfo *)target_info, signal_name);
  }
  g_base_info_unref(target_info);
  return signal_info;
}

/**
 * returns NULL if the signal doesn't exist
 * Perhaps we should throw a JS exception instead?
 * Perhaps we should throw a c++ exception instead?
 * Perhaps we should redesign this function to not have this sort of
 * error condition, it's suppose to be a 'constructor' function (constructing a closure struct)
 * and constructors shouldn't really 'error' like this.
 */
GClosure* gir_new_signal_closure(GIRObject *instance,
                                 GType signal_g_type,
                                 const char* signal_name,
                                 Nan::Persistent<Function,
                                 CopyablePersistentTraits<Function>> callback) {
  GISignalInfo *signal_info = gir_get_signal(signal_g_type, signal_name);
  if (signal_info == NULL) {
    return NULL;
  }

  // create a custom GClosure
  GClosure *closure = g_closure_new_simple(sizeof(GIRSignalClosure), NULL);
  GIRSignalClosure *gir_signal_closure = (GIRSignalClosure *)closure;

  // connect the finalaize notifier and marshaller
  g_closure_add_finalize_notifier(closure, NULL, gir_signal_closure_finalize_handler);
  g_closure_set_marshal(closure, gir_signal_closure_marshal);

  gir_signal_closure->callback = callback;
  gir_signal_closure->signal_info = signal_info;

  return closure;
}


}
