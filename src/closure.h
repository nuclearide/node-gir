#pragma once

#include <girepository.h>
#include <nan.h>
#include <node.h>
#include <string>
#include "types/object.h"
#include <ffi.h>
#include <girffi.h>

namespace gir {

using namespace std;
using namespace v8;

using PersistentFunction = Nan::Persistent<Function, CopyablePersistentTraits<Function>>;

class GIRClosure {
private:
    GClosure closure;
    GICallableInfo *callable_info;
    PersistentFunction callback;

public:
    static GClosure *create(GICallableInfo *callable_info,
                            Local<Function> callback);

    static ffi_closure *create_ffi(GICallableInfo *callable_info,
                                 Local<Function> callback);

private:
    GIRClosure() = default;
    static void closure_marshal(GClosure *closure,
                                GValue *return_value,
                                guint n_param_values,
                                const GValue *param_values,
                                gpointer invocation_hint,
                                gpointer marshal_data);
    // static GICallableInfo *get_signal(GType signal_g_type, const char *signal_name);
    static void finalize_handler(gpointer notify_data, GClosure *closure);

    static void ffi_closure_callback(ffi_cif *cif, void *result, void **args, gpointer user_data);

};

} // namespace gir
