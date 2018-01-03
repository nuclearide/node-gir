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

struct GIRSignalClosure {
    GClosure closure;
    GISignalInfo *signal_info;
    PersistentFunction callback;

    static GClosure *create(GIRObject *instance,
                            GType signal_g_type,
                            const char *signal_name,
                            Local<Function> callback);
};

} // namespace gir

#endif
