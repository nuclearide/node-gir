#pragma once

#include <vector>
#include <set>
#include <map>
#include <v8.h>
#include <nan.h>
#include <glib.h>
#include <girepository.h>

namespace gir {

using namespace v8;
using namespace std;

class GIRObject;

typedef Nan::Persistent<FunctionTemplate, CopyablePersistentTraits<FunctionTemplate>> PersistentFunctionTemplate;

struct ObjectFunctionTemplate {
    char *type_name;
    GIObjectInfo *info;
    PersistentFunctionTemplate object_template;
    GType type;
    char *namespace_;
};

struct MarshalData {
    GIRObject *that;
    char *event_name;
};

struct InstanceData {
    Local<Value> instance;
    GIRObject *obj;
};


class GIRObject : public Nan::ObjectWrap {
  public:
    GIRObject() {};
    GIRObject(GIObjectInfo *info_, map<string, GValue> &properties);
    virtual ~GIRObject();

    GObject *obj;
    bool abstract;
    GIBaseInfo *info;

    static std::set<GIRObject *> instances;
    static std::vector<ObjectFunctionTemplate *> templates;

    static Local<Value> from_existing(GObject *obj, GType t);
    static NAN_METHOD(New);

    static Local<Object> prepare(GIObjectInfo *object_info);
    static void register_methods(GIObjectInfo *object_info,
                                                                  const char *namespace_,
                                                                  Local<FunctionTemplate> &object_template);
    static void set_custom_fields(Local<FunctionTemplate> &object_template,
                                                                    GIObjectInfo *object_info);
    static void set_custom_prototype_methods(

        Local<FunctionTemplate> &object_template);
    static void extend_parent(Local<FunctionTemplate> &object_template,
                                                            GIObjectInfo *object_info);

    static void initialize(Local<Object> target, char *namespace_);

    static NAN_METHOD(GetProperty);
    static NAN_METHOD(SetProperty);
    static NAN_METHOD(GetInterface);
    static NAN_METHOD(GetField);
    static NAN_METHOD(CallVFunc);
    static NAN_METHOD(Connect);
    static NAN_METHOD(Disconnect);

    static MaybeLocal<Value> get_instance(GObject *obj);
    static ObjectFunctionTemplate  *create_object_template(

        GIObjectInfo *object_info);
    static ObjectFunctionTemplate  *find_template_from_object_info(

        GIObjectInfo *object_info);
    static ObjectFunctionTemplate  *find_or_create_template_from_object_info(

        GIObjectInfo *object_info);

    static GIFunctionInfo *find_method(GIObjectInfo *inf, char *name);
    static GIFunctionInfo *find_property(GIObjectInfo *inf, char *name);
    static GIFunctionInfo *find_interface(GIObjectInfo *inf, char *name);
    static GIFunctionInfo *find_field(GIObjectInfo *inf, char *name);
    static GIFunctionInfo *find_signal(GIObjectInfo *inf, char *name);
    static GIFunctionInfo *find_v_func(GIObjectInfo *inf, char *name);

    private:
    static void set_method(Local<FunctionTemplate> &target,
                                                      GIFunctionInfo *function_info);

    static Local<ObjectTemplate> property_list(GIObjectInfo *info);
    static Local<ObjectTemplate> method_list(GIObjectInfo *info);
    static Local<ObjectTemplate> interface_list(GIObjectInfo *info);
    static Local<ObjectTemplate> field_list(GIObjectInfo *info);
    static Local<ObjectTemplate> signal_list(GIObjectInfo *info);
    static Local<ObjectTemplate> v_func_list(GIObjectInfo *info);
    static GType get_object_property_type(GIObjectInfo *object_info,
                                                                                    const char *property_name);
    static map<string, GValue> parse_constructor_argument(

        Local<Object> properties_object, GIObjectInfo *object_info);
};

}
