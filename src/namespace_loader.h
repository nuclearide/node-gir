#pragma once

#include <nan.h>
#include <v8.h>

namespace gir {

using namespace v8;

class NamespaceLoader : public Nan::ObjectWrap {
public:
    static NAN_METHOD(load);

private:
    static Local<Value> load_namespace(const char *library_namespace, const char *version);
    static Local<Value> build_exports(const char *library_namespace);
};

} // namespace gir
