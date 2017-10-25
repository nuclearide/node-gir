#include "./signal_closure.h"
#include "./values.h"

namespace gir {

static void gir_signal_closure_marshal(GClosure *closure,
                                       GValue *return_value,
                                       guint n_param_values,
                                       const GValue *param_values,
                                       gpointer invocation_hint,
                                       gpointer marshal_data) {
  GirSignalClosure *gir_signal_closure = (GirSignalClosure *)closure;
  Nan::HandleScope scope;

  // get a local reference to the function
  Local<Function> local_callback = Nan::New<Function>(gir_signal_closure->callback);

  // Call the function. We will pass 'global' as the value of 'this' inside the callback
  // Generally people should never use the value of 'this' in a callback function as it's
  // unreliable (funtion binds, arrow functions are better). If we could set 'this' to 'undefined'
  // then that would be better than setting it to 'global' to make it clear we don't intend
  // for people to use it!
  Nan::MaybeLocal<Value> maybe_result = Nan::Call(local_callback, Nan::GetCurrentContext()->Global(), 0, nullptr);
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

void gir_signal_closure_finalize_handler(gpointer notify_data, GClosure *closure) {
  GirSignalClosure *gir_signal_closure = (GirSignalClosure *)closure;

  // reset (free) the JS persistent function
  gir_signal_closure->callback.Reset();
}

/**
 * The instance and signal_name currently aren't used in this
 * function, but there is a good chance that they might need to be
 * used to implement signal closures so i've left them in the function prototype
 * to avoid a breaking change if we need them in the future.
 */
GClosure* gir_new_signal_closure(GIRObject *instance, char *signal_name, Nan::Persistent<Function, CopyablePersistentTraits<Function>> callback) {
  // create a custom GClosure
  GClosure *closure = g_closure_new_simple(sizeof(GirSignalClosure), NULL);
  GirSignalClosure *gir_signal_closure = (GirSignalClosure *)closure;

  // connect the finalaize notifier and marshaller
  g_closure_add_finalize_notifier(closure, NULL, gir_signal_closure_finalize_handler);
  g_closure_set_marshal(closure, gir_signal_closure_marshal);

  gir_signal_closure->callback = callback;

  return closure;
}


}
