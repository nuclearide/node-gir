#pragma once

#include <girepository.h>
#include <glib.h>
#include <nan.h>
#include <v8.h>
#include <map>
#include <set>
#include <vector>

namespace gir {

using namespace v8;
using namespace std;

class GIRObject;

using PersistentFunctionTemplate = Nan::Persistent<FunctionTemplate, CopyablePersistentTraits<FunctionTemplate>>;

struct ObjectFunctionTemplate {
    char *type_name;
    GIObjectInfo *info; // FIXME: use GIRInfoUniquePtr
    PersistentFunctionTemplate object_template;
    GType type;
    char *namespace_;
};

class GIRObject : public Nan::ObjectWrap {
private:
    static std::set<GIRObject *> instances; // FIXME: use smart pointers
    static std::vector<ObjectFunctionTemplate *> templates; // FIXME: use smart pointers
    GObject *obj;
    GIBaseInfo *info;

public:
    static Local<Object> prepare(GIObjectInfo *object_info);
    static Local<Value> from_existing(GObject *obj, GType t);
    GObject* get_gobject();

private:
    GIRObject() = default;
    GIRObject(GIObjectInfo *info_, map<string, GValue> &properties);

    static MaybeLocal<Value> get_instance(GObject *obj);
    static ObjectFunctionTemplate *create_object_template(GIObjectInfo *object_info);
    static ObjectFunctionTemplate *find_template_from_object_info(GIObjectInfo *object_info);
    static ObjectFunctionTemplate *find_or_create_template_from_object_info(GIObjectInfo *object_info);

    static map<string, GValue> parse_constructor_argument(Local<Object> properties_object, GIObjectInfo *object_info);
    static GType get_object_property_type(GIObjectInfo *object_info, const char *property_name);

    static void register_methods(GIObjectInfo *object_info,
                                 const char *namespace_,
                                 Local<FunctionTemplate> &object_template);
    static void set_method(Local<FunctionTemplate> &target, GIFunctionInfo *function_info);
    static void set_custom_fields(Local<FunctionTemplate> &object_template, GIObjectInfo *object_info);
    static void set_custom_prototype_methods(Local<FunctionTemplate> &object_template);
    static void extend_parent(Local<FunctionTemplate> &object_template, GIObjectInfo *object_info);
    static GIFunctionInfo *find_property(GIObjectInfo *inf, char *name);

    static NAN_METHOD(constructor);
    static NAN_METHOD(connect);
    static NAN_METHOD(disconnect);
    static NAN_PROPERTY_GETTER(property_get_handler);
    static NAN_PROPERTY_SETTER(property_set_handler);
    static NAN_PROPERTY_QUERY(property_query_handler);

};

} // namespace gir
