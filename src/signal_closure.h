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

typedef struct _GirSignalClosure {
  GClosure closure;
  Nan::Persistent<Function, CopyablePersistentTraits<Function>> callback;
} GirSignalClosure;

GClosure* gir_new_signal_closure(GIRObject *instance, char *signal_name, Nan::Persistent<Function, CopyablePersistentTraits<Function>> callback);

}

#endif
