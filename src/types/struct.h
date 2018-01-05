#pragma once

#include "util.h"
#include <girepository.h>
#include <glib.h>
#include <nan.h>
#include <v8.h>
#include <vector>

namespace gir {

using namespace v8;

class GIRStruct;

class GIRStruct : public Nan::ObjectWrap {
public:
    gpointer get_native_ptr();

    static Local<Function> prepare(GIStructInfo *info);
    static Local<Value> from_existing(gpointer c_structure, GIStructInfo *info);

private:
    gpointer c_structure;
    GIBaseInfo *info;

    static GIRInfoUniquePtr find_native_constructor(GIStructInfo *struct_info);
    static void register_methods(GIStructInfo *info,
                                 const char *namespace_,
                                 Local<FunctionTemplate> object_template);
    static NAN_METHOD(constructor);
    static NAN_METHOD(call_method);
    static NAN_PROPERTY_GETTER(property_get_handler);
    static NAN_PROPERTY_SETTER(property_set_handler);
    static NAN_PROPERTY_QUERY(property_query_handler);

    GIRStruct() = default;

};

} // namespace gir
