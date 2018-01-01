#include "arguments.h"
#include "util.h"
#include "values.h"

#include "types/object.h"
#include <string.h>

#include <vector>

using namespace v8;

namespace gir {

Args::Args(GICallableInfo *callable_info) {
    g_base_info_ref(callable_info);
    this->callable_info = callable_info;
}

Args::~Args() {
    g_base_info_unref(this->callable_info);
}

/**
 * This method, given a JS function call object will load each JS argument
 * into Args datastructure (in and out arguments). Each JS argument value
 * is converted to it's native equivalient. This method will also handle
 * native "out" arguments even though they aren't passed in via JS function
 * calls.
 * @param js_callback_info is a JS function call info object
 */
void Args::loadJSArguments(const Nan::FunctionCallbackInfo<v8::Value> &js_callback_info) {
    // get the number of arguments the native function requires
    guint8 gi_argc = g_callable_info_get_n_args(this->callable_info);

    // for every expected native argument, we'll take a given JS argument and convert
    // it into a GIArgument, adding it to the in/out args array depending on it's direction.
    for (guint8 i = 0; i < gi_argc; i++) {
        GIArgInfo argument_info;
        g_callable_info_load_arg(this->callable_info, i, &argument_info);
        GIDirection argument_direction = g_arg_info_get_direction(&argument_info);

        if (argument_direction == GI_DIRECTION_IN) {
            GIArgument argument = this->GetInArgumentValue(js_callback_info[i], argument_info);
            this->in.push_back(argument);
        }

        if (argument_direction == GI_DIRECTION_OUT) {
            // FIXME: we should not
            GIArgument argument = this->GetOutArgumentValue(argument_info);
            this->out.push_back(argument);
        }

        if (argument_direction == GI_DIRECTION_INOUT) {
            GIArgument argument = this->GetInArgumentValue(js_callback_info[i], argument_info);
            this->in.push_back(argument);

            // TODO: is it correct to handle INOUT arguments like IN args?
            // do we need to handle callee (native) allocates or empty input GIArguments like we do with OUT args?
            // i'm just assuming this is how it should work (treating it like an IN arg). Hopfully I can find
            // some examples to make some test cases asserting the correct behaviour
            this->out.push_back(argument);
        }
    }
}

/**
 * This function loads the context (i.e. this value of `this`) into the native call arguments.
 * By convention, the context value (a GIRObject in JS or a GObject in native) is put at the
 * start (position 0) of the function call's "in" arguments.
 */
void Args::loadContext(GObject *this_object) {
    GIArgument this_object_argument = {
        .v_pointer = this_object,
    };
    this->in.insert(this->in.begin(), this_object_argument);
}

GIArgument Args::GetInArgumentValue(const Local<Value> &js_value, GIArgInfo &argument_info) {
    GITypeInfo argument_type_info;
    g_arg_info_load_type(&argument_info, &argument_type_info);
    GIArgument argument;
    Args::ToGType(js_value, &argument, &argument_info, &argument_type_info, false);
    return argument;
}


GIArgument Args::GetOutArgumentValue(GIArgInfo &argument_info) {
    GITypeInfo argument_type_info;
    g_arg_info_load_type(&argument_info, &argument_type_info);
    if (g_arg_info_is_caller_allocates(&argument_info)) {
        // If the caller is responsible for allocating the out arguments memeory
        // then we'll have to look up the argument's type infomation and allocate
        // a slice of memory for the GIArgument's .v_pointer (native function will fill it up)
        if (g_type_info_get_tag(&argument_type_info) == GI_TYPE_TAG_INTERFACE) {
            GIBaseInfo *argument_interface_info = g_type_info_get_interface(&argument_type_info);
            GIInfoType argument_interface_type = g_base_info_get_type(argument_interface_info);
            gsize argument_size;

            if (argument_interface_type == GI_INFO_TYPE_STRUCT) {
                argument_size = g_struct_info_get_size((GIStructInfo*)argument_interface_info);
            } else if (argument_interface_type == GI_INFO_TYPE_UNION) {
                argument_size = g_union_info_get_size((GIUnionInfo*)argument_interface_info);
            } else {
                // TODO: FAIL WITH EXCEPTION
                // I dislike boolean return values that signal success/failure
                // but everyone seems to dislike c++ exceptions
                // how should a function like Args::loadJSArguments fail?
                fprintf(stderr, "TODO: FAILE WITH EXCEPTION");
            }

            g_base_info_unref(argument_interface_info);

            GIArgument argument;
            // FIXME: who deallocates?
            // I imagine the original function caller (in JS land) will need
            // to use the structure that the native function puts into this
            // slice of memory, meaning we can't deallocate when Args is destroyed.
            // Perhaps we should research into GJS and PyGObject to understand
            // the problem of "out arguments with caller allocation" better.
            argument.v_pointer = g_slice_alloc0(argument_size);
            return argument;
        } else {
            // TODO: FAIL WITH EXCEPTION i forgot why :(
            fprintf(stderr, "TODO: FAIL WITH EXCEPTION");
        }
    } else {
        // else, if we're not responsible for allocation then we can just return an
        // empty GIArgument with a NULL .v_pointer (native call will set it with a
        // memory location)
        GIArgument argument = {
            .v_pointer = NULL
        };
        return argument;
    }
}


bool Args::ToGType(Handle<Value> v, GIArgument *arg, GIArgInfo *info, GITypeInfo *type_info, bool out) {
    GITypeInfo *type = type_info;
    if (info != nullptr) {
        type = g_arg_info_get_type(info);
    }
    GITypeTag tag = ReplaceGType(g_type_info_get_tag(type));

    // nullify string so it be freed safely later
    arg->v_string = nullptr;

    if (out == TRUE) {
        return true;
    }

    if ((v == Nan::Null() || v == Nan::Undefined()) && (g_arg_info_may_be_null(info) || tag == GI_TYPE_TAG_VOID)) {
        arg->v_pointer = nullptr;
        return true;
    }
    if (tag == GI_TYPE_TAG_BOOLEAN) {
        arg->v_boolean = v->ToBoolean()->Value();
        return true;
    }
    if (tag == GI_TYPE_TAG_INT8) {
        arg->v_uint8 = v->NumberValue();
        return true;
    }
    if (tag == GI_TYPE_TAG_UINT8) {
        arg->v_uint8 = v->NumberValue();
        return true;
    }
    if (tag == GI_TYPE_TAG_INT16) {
        arg->v_int16 = v->NumberValue();
        return true;
    }
    if (tag == GI_TYPE_TAG_UINT16) {
        arg->v_uint16 = v->NumberValue();
        return true;
    }
    if (tag == GI_TYPE_TAG_INT32) {
        arg->v_int32 = v->Int32Value();
        return true;
    }
    if (tag == GI_TYPE_TAG_UINT32) {
        arg->v_uint32 = v->Uint32Value();
        return true;
    }
    if (tag == GI_TYPE_TAG_INT64) {
        arg->v_int64 = v->IntegerValue();
        return true;
    }
    if (tag == GI_TYPE_TAG_UINT64) {
        arg->v_uint64 = v->IntegerValue();
        return true;
    }
    if (tag == GI_TYPE_TAG_FLOAT) {
        arg->v_float = v->NumberValue();
        return true;
    }
    if (tag == GI_TYPE_TAG_DOUBLE) {
        arg->v_double = v->NumberValue();
        return true;
    }
    if (tag == GI_TYPE_TAG_UTF8 || tag == GI_TYPE_TAG_FILENAME) {
        String::Utf8Value v8str(v->ToString());
        arg->v_string = g_strdup(*v8str);
        return true;
    }
    if (tag == GI_TYPE_TAG_GLIST) {
        if (!v->IsArray()) { return false; }
        //GList *list = nullptr;
        //ArrayToGList(v, info, &list); // FIXME!!!
        return false;
    }
    if (tag == GI_TYPE_TAG_GSLIST) {
        if (!v->IsArray()) { return false; }
        //GSList *list = nullptr;
        //ArrayToGList(v, info, &list); // FIXME!!!
        return false;
    }
    if (tag == GI_TYPE_TAG_ARRAY) {
        if (!v->IsArray()) {
            if (v->IsString()) {
                String::Utf8Value _str(v->ToString());
                arg->v_pointer = (gpointer *) g_strdup(*_str);
                return true;
            }
            return false;
        }

        GIArrayType arr_type = g_type_info_get_array_type(info);

        if (arr_type == GI_ARRAY_TYPE_C) {

        }
        else if (arr_type == GI_ARRAY_TYPE_ARRAY) {

        }
        else if (arr_type == GI_ARRAY_TYPE_PTR_ARRAY) {

        }
        else if (arr_type == GI_ARRAY_TYPE_BYTE_ARRAY) {

        }
        return false;
    }
    if (tag == GI_TYPE_TAG_GHASH) {
        if (!v->IsObject()) { return false; }

        GITypeInfo *key_param_info, *val_param_info;
        //GHashTable *ghash;

        key_param_info = g_type_info_get_param_type(info, 0);
        g_assert(key_param_info != nullptr);
        val_param_info = g_type_info_get_param_type(info, 1);
        g_assert(val_param_info != nullptr);

        // TODO: implement

        g_base_info_unref((GIBaseInfo*) key_param_info);
        g_base_info_unref((GIBaseInfo*) val_param_info);

        return false;
    }
    if (tag == GI_TYPE_TAG_INTERFACE) {
        GIBaseInfo *interface_info = g_type_info_get_interface(type);
        g_assert(interface_info != nullptr);
        GIInfoType interface_type = g_base_info_get_type(interface_info);

        GType gtype;
        switch(interface_type) {
            case GI_INFO_TYPE_STRUCT:
            case GI_INFO_TYPE_ENUM:
            case GI_INFO_TYPE_OBJECT:
            case GI_INFO_TYPE_INTERFACE:
            case GI_INFO_TYPE_UNION:
            case GI_INFO_TYPE_BOXED:
                gtype = g_registered_type_info_get_g_type
                    ((GIRegisteredTypeInfo*)interface_info);
                break;
            case GI_INFO_TYPE_VALUE:
                gtype = G_TYPE_VALUE;
                break;

            default:
                gtype = G_TYPE_NONE;
                break;
        }

        if (g_type_is_a(gtype, G_TYPE_OBJECT)) {
            if (!v->IsObject()) { return false; }
            GIRObject *gir_object = Nan::ObjectWrap::Unwrap<GIRObject>(v->ToObject());
            arg->v_pointer = gir_object->obj;
            return true;
        }
        if (g_type_is_a(gtype, G_TYPE_VALUE)) {
            GValue gvalue = {0, {{0}}};
            if (!GIRValue::ToGValue(v, G_TYPE_INVALID, &gvalue)) {
                return false;
            }
            //FIXME I've to free this somewhere
            arg->v_pointer = g_boxed_copy(G_TYPE_VALUE, &gvalue);
            g_value_unset(&gvalue);
            return true;
        }
    }

    return false;
}

Handle<Value> Args::FromGTypeArray(GIArgument *arg, GITypeInfo *type, int array_length) {

    GITypeInfo *param_info = g_type_info_get_param_type(type, 0);
    //bool is_zero_terminated = g_type_info_is_zero_terminated(param_info);
    GITypeTag param_tag = g_type_info_get_tag(param_info);

    //g_base_info_unref(param_info);

    int i = 0;
    v8::Local<v8::Array> arr;
    GIBaseInfo *interface_info;

    switch(param_tag) {
        case GI_TYPE_TAG_UINT8:
            if (arg->v_pointer == nullptr)
                return Nan::New("", 0).ToLocalChecked();
            // TODO, copy bytes to array
            // http://groups.google.com/group/v8-users/browse_thread/thread/8c5177923675749e?pli=1
            return Nan::New((char *)arg->v_pointer, array_length).ToLocalChecked();

        case GI_TYPE_TAG_GTYPE:
            if (arg->v_pointer == nullptr)
                return Nan::New<Array>();
            arr = Nan::New<Array>(array_length);
            for (i = 0; i < array_length; i++) {
                Nan::Set(arr, i, Nan::New((int)GPOINTER_TO_INT( ((gpointer*)arg->v_pointer)[i] )));
            }
            return arr;

        case GI_TYPE_TAG_INTERFACE:
            if (arg->v_pointer == nullptr) {
                return Nan::New<Array>();
            }
            arr = Nan::New<Array>(array_length);
            interface_info = g_type_info_get_interface(param_info);
            for (i = 0; i < array_length; i++) {
                GObject *o = (GObject*)((gpointer*)arg->v_pointer)[i];
                arr->Set(i, GIRObject::New(o, g_registered_type_info_get_g_type(interface_info)));
            }
            g_base_info_unref(interface_info);
            return arr;

        default:
            gchar *exc_msg = g_strdup_printf("Converting array of '%s' is not supported", g_type_tag_to_string(param_tag));
            Nan::ThrowError(exc_msg);
            return Nan::Undefined();
    }
}

// TODO: refactor this function and most of the code below this.
// can we reuse code from GIRValue?
Local<Value> Args::FromGType(GIArgument *arg, GITypeInfo *type, int array_length) {
    GITypeTag tag = g_type_info_get_tag(type);

    if (tag == GI_TYPE_TAG_INTERFACE) {
        GIBaseInfo *interface_info = g_type_info_get_interface(type);
        g_assert(interface_info != nullptr);
        GIInfoType interface_type = g_base_info_get_type(interface_info);

        if (interface_type == GI_INFO_TYPE_OBJECT) {
            Local<Value> new_instance = GIRObject::New(G_OBJECT(arg->v_pointer), g_registered_type_info_get_g_type(interface_info));
            g_base_info_unref(interface_info);
            return new_instance;
        }
    }

    if (tag == GI_TYPE_TAG_INTERFACE) {
        GIBaseInfo *interface_info = g_type_info_get_interface(type);
        g_assert(interface_info != nullptr);
        GIInfoType interface_type = g_base_info_get_type(interface_info);

        GType gtype;
        switch(interface_type) {
            case GI_INFO_TYPE_STRUCT:
            case GI_INFO_TYPE_ENUM:
            case GI_INFO_TYPE_OBJECT:
            case GI_INFO_TYPE_INTERFACE:
            case GI_INFO_TYPE_UNION:
            case GI_INFO_TYPE_BOXED:
                gtype = g_registered_type_info_get_g_type
                    ((GIRegisteredTypeInfo*)interface_info);
                break;
            case GI_INFO_TYPE_VALUE:
                gtype = G_TYPE_VALUE;
                break;

            default:
                gtype = G_TYPE_NONE;
                break;
        }

        if (g_type_is_a(gtype, G_TYPE_OBJECT)) {
            GObject *o = G_OBJECT(arg->v_pointer);
            return GIRObject::New(o, G_OBJECT_TYPE(o));
        }
        if (g_type_is_a(gtype, G_TYPE_VALUE)) {
            GIRValue::FromGValue((GValue*)arg->v_pointer, nullptr);
        }
    }

    switch(tag) {
        case GI_TYPE_TAG_VOID:
            return Nan::Undefined();
        case GI_TYPE_TAG_BOOLEAN:
            return Nan::New<Boolean>(arg->v_boolean);
        case GI_TYPE_TAG_INT8:
            return Nan::New(arg->v_int8);
        case GI_TYPE_TAG_UINT8:
            return Integer::NewFromUnsigned(v8::Isolate::GetCurrent(), arg->v_uint8);
        case GI_TYPE_TAG_INT16:
            return Nan::New(arg->v_int16);
        case GI_TYPE_TAG_UINT16:
            return Integer::NewFromUnsigned(v8::Isolate::GetCurrent(), arg->v_uint16);
        case GI_TYPE_TAG_INT32:
            return Nan::New(arg->v_int32);
        case GI_TYPE_TAG_UINT32:
            return Integer::NewFromUnsigned(v8::Isolate::GetCurrent(), arg->v_uint32);
        case GI_TYPE_TAG_INT64:
            return Nan::New((double) arg->v_int64);
        case GI_TYPE_TAG_UINT64:
            return Integer::NewFromUnsigned(v8::Isolate::GetCurrent(), arg->v_uint64);
        case GI_TYPE_TAG_FLOAT:
            return Nan::New(arg->v_float);
        case GI_TYPE_TAG_DOUBLE:
            return Nan::New(arg->v_double);
        case GI_TYPE_TAG_GTYPE:
            return Integer::NewFromUnsigned(v8::Isolate::GetCurrent(), arg->v_uint);
        case GI_TYPE_TAG_UTF8:
            return Nan::New(arg->v_string).ToLocalChecked();
        case GI_TYPE_TAG_FILENAME:
            return Nan::New(arg->v_string).ToLocalChecked();
        case GI_TYPE_TAG_ARRAY:
            return Args::FromGTypeArray(arg, type, array_length);
        case GI_TYPE_TAG_INTERFACE:
            return Nan::Undefined();
        case GI_TYPE_TAG_GLIST:
            return Nan::Undefined();
        case GI_TYPE_TAG_GSLIST:
            return Nan::Undefined();
        case GI_TYPE_TAG_GHASH:
            return Nan::Undefined();
        case GI_TYPE_TAG_ERROR:
            return Nan::Undefined();
        case GI_TYPE_TAG_UNICHAR:
            return Nan::Undefined();
        default:
            return Nan::Undefined();
    }
}

GITypeTag Args::ReplaceGType(GITypeTag type) {
    if (type == GI_TYPE_TAG_GTYPE) {
        switch (sizeof(GType)) {
        case 1: return GI_TYPE_TAG_UINT8;
        case 2: return GI_TYPE_TAG_UINT16;
        case 4: return GI_TYPE_TAG_UINT32;
        case 8: return GI_TYPE_TAG_UINT64;
        default: g_assert_not_reached();
        }
    }
    return type;
}

bool Args::ArrayToGList(Handle<Array> arr, GIArgInfo *info, GList **list_p) {
    GList *list = nullptr;

    int l = arr->Length();
    for(int i=0; i<l; i++) {
        GIArgument arg = {0,};
        if (!Args::ToGType(arr->Get(Nan::New(i)), &arg, g_type_info_get_param_type(info, 0), nullptr, FALSE)) {
            return false;
        }
        list = g_list_prepend(list, arg.v_pointer);
    }

    list = g_list_reverse(list);
    *list_p = list;

    return true;
}

bool Args::ArrayToGList(Handle<Array> arr, GIArgInfo *info, GSList **slist_p) {
    GSList *slist = nullptr;

    int l = arr->Length();
    for(int i=0; i<l; i++) {
        GIArgument arg = {0,};
        if (!Args::ToGType(arr->Get(Nan::New(i)), &arg, g_type_info_get_param_type(info, 0), nullptr, FALSE)) {
            return false;
        }
        slist = g_slist_prepend(slist, arg.v_pointer);
    }

    slist = g_slist_reverse(slist);
    *slist_p = slist;

    return true;
}


}
