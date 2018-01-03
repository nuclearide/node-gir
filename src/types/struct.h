#pragma once

#include <girepository.h>
#include <glib.h>
#include <nan.h>
#include <v8.h>
#include <vector>

namespace gir {

using namespace v8;

class GIRStruct;

typedef Nan::Persistent<FunctionTemplate, CopyablePersistentTraits<FunctionTemplate>> PersistentFunctionTemplate;
typedef Nan::Persistent<Value, CopyablePersistentTraits<Value>> PersistentValue;

struct StructFunctionTemplate {
    char *type_name;
    GIObjectInfo *info;
    PersistentFunctionTemplate function;
    GType type;
    char *namespace_;
};

struct StructMarshalData {
    GIRStruct *that;
    char *event_name;
};

struct StructData {
    PersistentValue instance;
    GIRStruct *gir_structure;
};

class GIRStruct : public Nan::ObjectWrap {
public:
    GIRStruct(){};
    GIRStruct(GIObjectInfo *info);

    gpointer c_structure;
    bool abstract;
    GIBaseInfo *info;

    static std::vector<StructData> instances;
    static std::vector<StructFunctionTemplate> templates;

    static v8::Handle<v8::Value> from_existing(gpointer c_structure, GIStructInfo *info);
    static NAN_METHOD(New);

    static Local<Value> prepare(GIStructInfo *info);
    static void register_methods(GIStructInfo *info,
                                 const char *namespace_,
                                 v8::Handle<v8::FunctionTemplate> object_template);

    static void initialize(v8::Handle<v8::Object> target, char *namespace_);

    static NAN_METHOD(CallMethod);

    static void push_instance(GIRStruct *obj, v8::Handle<v8::Value>);
    static v8::Handle<v8::Value> get_structure(gpointer c_structure);

private:
    static v8::Handle<v8::Object> property_list(GIObjectInfo *info);
    static v8::Handle<v8::Object> method_list(GIObjectInfo *info);
    static v8::Handle<v8::Object> interface_list(GIObjectInfo *info);
    static v8::Handle<v8::Object> field_list(GIObjectInfo *info);
};

} // namespace gir
