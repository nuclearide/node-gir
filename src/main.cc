#include <node.h>
#include <v8.h>

#include "loop.h"
#include "namespace_loader.h"

NAN_MODULE_INIT(InitAll) {
    Nan::Set(target,
             Nan::New("load").ToLocalChecked(),
             Nan::GetFunction(Nan::New<v8::FunctionTemplate>(gir::NamespaceLoader::load)).ToLocalChecked());
    Nan::Set(target,
             Nan::New("startLoop").ToLocalChecked(),
             Nan::GetFunction(Nan::New<v8::FunctionTemplate>(gir::start_loop)).ToLocalChecked());
}

NODE_MODULE(girepository, InitAll)
