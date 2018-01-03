#ifndef SIGNAL_CLOSURE_H
#define SIGNAL_CLOSURE_H

#include <girepository.h>
#include <nan.h>
#include <node.h>
#include <string>
#include "./types/object.h"

namespace gir {

using namespace std;
using namespace v8;

using PersistentFunction = Nan::Persistent<Function, CopyablePersistentTraits<Function>>;

class GIRSignalClosure {
private:
    GClosure closure;
    GISignalInfo *signal_info;
    PersistentFunction callback;

public:
    static GClosure *create(GIRObject *instance,
                            GType signal_g_type,
                            const char *signal_name,
                            Local<Function> callback);

private:
    GIRSignalClosure() = default;
    static void closure_marshal(GClosure *closure,
                                GValue *return_value,
                                guint n_param_values,
                                const GValue *param_values,
                                gpointer invocation_hint,
                                gpointer marshal_data);
    static GISignalInfo *get_signal(GType signal_g_type, const char *signal_name);
    static void finalize_handler(gpointer notify_data, GClosure *closure);
};

} // namespace gir

#endif
