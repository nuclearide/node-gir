#ifndef SIGNAL_CLOSURE_H
#define SIGNAL_CLOSURE_H

#include <string>
#include <node.h>
#include <nan.h>
#include <girepository.h>
#include "./types/object.h"

namespace gir {

using namespace std;
using namespace v8;

typedef struct _GIRSignalClosure {
  GClosure closure;
  GISignalInfo *signal_info;
  Nan::Persistent<Function, CopyablePersistentTraits<Function>> callback;
} GIRSignalClosure;

GClosure* gir_new_signal_closure(GIRObject *instance,
                                 GType signal_g_type,
                                 const char* signal_name,
                                 Nan::Persistent<Function,
                                 CopyablePersistentTraits<Function>> callback);

}

#endif
