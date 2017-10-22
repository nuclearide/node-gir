#include <node.h>

#include <v8.h>

#include "util.h"
#include "namespace_loader.h"
#include "loop.h"

NAN_MODULE_INIT(InitAll) {
    Nan::Set(target, Nan::New("load").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(gir::NamespaceLoader::Load)).ToLocalChecked());
    Nan::Set(target, Nan::New("search_path").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(gir::NamespaceLoader::SearchPath)).ToLocalChecked());
    Nan::Set(target, Nan::New("StartLoop").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(gir::StartLoop)).ToLocalChecked());
}

NODE_MODULE(girepository, InitAll)
