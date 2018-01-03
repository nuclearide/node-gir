#include "arguments.h"
#include "util.h"
#include "values.h"

#include "types/object.h"
#include "exceptions.h"
#include <string.h>
#include <vector>
#include <sstream>


using namespace v8;

namespace gir {

Args::Args(GICallableInfo *callable_info): callable_info(callable_info, g_base_info_unref) {
    g_base_info_ref(callable_info); // because we keep a reference to the info we need to tell glib
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
    guint8 gi_argc = g_callable_info_get_n_args(this->callable_info.get());

    // for every expected native argument, we'll take a given JS argument and convert
    // it into a GIArgument, adding it to the in/out args array depending on it's direction.
    for (guint8 i = 0; i < gi_argc; i++) {
        GIArgInfo argument_info;
        g_callable_info_load_arg(this->callable_info.get(), i, &argument_info);
        GIDirection argument_direction = g_arg_info_get_direction(&argument_info);

        if (argument_direction == GI_DIRECTION_IN) {
            GIArgument argument = Args::ToGType(argument_info, js_callback_info[i]);
            this->in.push_back(argument);
        }

        if (argument_direction == GI_DIRECTION_OUT) {
            GIArgument argument = this->GetOutArgumentValue(argument_info);
            this->out.push_back(argument);
        }

        if (argument_direction == GI_DIRECTION_INOUT) {
            GIArgument argument = Args::ToGType(argument_info, js_callback_info[i]);
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

GIArgument Args::GetOutArgumentValue(GIArgInfo &argument_info) {
    GITypeInfo argument_type_info;
    g_arg_info_load_type(&argument_info, &argument_type_info);
    if (g_arg_info_is_caller_allocates(&argument_info)) {
        GITypeTag arg_type_tag = g_type_info_get_tag(&argument_type_info);
        // If the caller is responsible for allocating the out arguments memeory
        // then we'll have to look up the argument's type infomation and allocate
        // a slice of memory for the GIArgument's .v_pointer (native function will fill it up)
        if (arg_type_tag == GI_TYPE_TAG_INTERFACE) {
            GIRInfoUniquePtr argument_interface_info = GIRInfoUniquePtr(g_type_info_get_interface(&argument_type_info));
            GIInfoType argument_interface_type = g_base_info_get_type(argument_interface_info.get());
            gsize argument_size;

            if (argument_interface_type == GI_INFO_TYPE_STRUCT) {
                argument_size = g_struct_info_get_size((GIStructInfo*)argument_interface_info.get());
            } else if (argument_interface_type == GI_INFO_TYPE_UNION) {
                argument_size = g_union_info_get_size((GIUnionInfo*)argument_interface_info.get());
            } else {
                stringstream message;
                message << "type \"" << g_type_tag_to_string(arg_type_tag) << "\" for out caller-allocates";
                message << " Expected a struct or union.";
                throw UnsupportedGIType(message.str());
            }

            GIArgument argument;
            // FIXME: who deallocates?
            // I imagine the original function caller (in JS land) will need
            // to use the structure that the native function puts into this
            // slice of memory, meaning we can't deallocate when Args is destroyed.
            // Perhaps we should research into GJS and PyGObject to understand
            // the problem of "out arguments with caller allocation" better.
            // Some thoughts:
            // 1. if the data is **copied** into a gir object/struct when passed back
            //    to JS then we can safely implement a custom deleter for Args.out
            //    that cleans this up.
            // 2. if the pointer to the data is passed to a gir object/struct when
            //    passed back to JS then that JS object should own it and be responsible
            //    for cleaning it up.
            // * both choices have caveats so it's worth understanding the implications
            //   of each, or other options to solve this leak!
            // * from reading GJS code, it seems like they copy the data before passing
            // * to JS meaning option 1.
            argument.v_pointer = g_slice_alloc0(argument_size);
            return argument;
        } else {
            stringstream message;
            message << "type \"" << g_type_tag_to_string(arg_type_tag) << "\" for out caller-allocates";
            throw UnsupportedGIType(message.str());
        }
    }
    // else, if we're not responsible for allocation then we can just return an
    // empty GIArgument with a NULL .v_pointer (native call will set it with a
    // memory location)
    GIArgument argument = {
        .v_pointer = NULL
    };
    return argument;
}


GIArgument Args::ToGType(GIArgInfo &argument_info, Local<Value> js_value) {
    GITypeInfo argument_type_info;
    g_arg_info_load_type(&argument_info, &argument_type_info);
    GITypeTag argument_type_tag = g_type_info_get_tag(&argument_type_info);

    // if the arg type is a GTYPE (which is an integer)
    // then we want to pretend it's a GI_TYPE_TAG_INTX
    // where x is the sizeof the GTYPE. This helper function
    // does the mapping for us.
    if (argument_type_tag == GI_TYPE_TAG_GTYPE) {
        argument_type_tag = Args::MapGTypeTag(argument_type_tag);
    }

    // this is what we'll return after we correctly
    // set it's values depending on the argument_type_tag
    GIArgument argument_value;

    if (js_value->IsNullOrUndefined() && (g_arg_info_may_be_null(&argument_info) || argument_type_tag == GI_TYPE_TAG_VOID)) {
        argument_value.v_pointer = nullptr;
        return argument_value;
    }

    try {
        switch (argument_type_tag) {
            case GI_TYPE_TAG_BOOLEAN:
                argument_value.v_boolean = js_value->ToBoolean()->Value();
                break;

            case GI_TYPE_TAG_INT8:
                argument_value.v_uint8 = js_value->NumberValue();
                break;

            case GI_TYPE_TAG_UINT8:
                argument_value.v_uint8 = js_value->NumberValue();
                break;

            case GI_TYPE_TAG_INT16:
                argument_value.v_int16 = js_value->NumberValue();
                break;

            case GI_TYPE_TAG_UINT16:
                argument_value.v_uint16 = js_value->NumberValue();
                break;

            case GI_TYPE_TAG_INT32:
                argument_value.v_int32 = js_value->Int32Value();
                break;

            case GI_TYPE_TAG_UINT32:
                argument_value.v_uint32 = js_value->Uint32Value();
                break;

            case GI_TYPE_TAG_INT64:
                argument_value.v_int64 = js_value->IntegerValue();
                break;

            case GI_TYPE_TAG_UINT64:
                argument_value.v_uint64 = js_value->IntegerValue();
                break;

            case GI_TYPE_TAG_FLOAT:
                argument_value.v_float = js_value->NumberValue();
                break;

            case GI_TYPE_TAG_DOUBLE:
                argument_value.v_double = js_value->NumberValue();
                break;

            case GI_TYPE_TAG_UTF8:
            case GI_TYPE_TAG_FILENAME:
                if (!js_value->IsString()) {
                    throw JSArgumentTypeError();
                }
                {
                    Nan::Utf8String js_string(js_value->ToString());
                    // FIXME: memory leak as we never g_free(.v_string)
                    // ideally the memory would be freed when the GIArgument
                    // was destroyed. Perhaps we need to use a unique_ptr
                    // with a custom deleter?
                    // I think it's clear that the memory needs to be owned by
                    // the GIArgument (or value ToGType returns) because
                    // we can't expect callers to know they are responsible
                    // for deallocation, and we can't expect to borrow the
                    // memory (if we don't copy the string then tests start
                    // failing with corrupt memory rather than the original strings).
                    argument_value.v_string = strdup(*js_string);
                }
                break;

            // case GI_TYPE_TAG_GLIST: // TODO: implement
            //     if (!js_value->IsArray()) {
            //         throw JSArgumentTypeError("expected \"array\"");
            //     }
            //     //GList *list = nullptr;
            //     //ArrayToGList(v, info, &list); // FIXME!!!
            //     break;

            // case GI_TYPE_TAG_GSLIST: // TODO: implement
            //     if (!js_value->IsArray()) {
            //         throw JSArgumentTypeError("expected \"array\"");
            //     }
            //     //GSList *list = nullptr;
            //     //ArrayToGList(v, info, &list); // FIXME!!!
            //     break;

            // case GI_TYPE_TAG_ARRAY: // TODO: implement
            //     if (!js_value->IsArray()) {
            //         throw "TODO: js type error";
            //     }

            //     GIArrayType arr_type = g_type_info_get_array_type(info);

            //     if (arr_type == GI_ARRAY_TYPE_C) {

            //     }
            //     else if (arr_type == GI_ARRAY_TYPE_ARRAY) {

            //     }
            //     else if (arr_type == GI_ARRAY_TYPE_PTR_ARRAY) {

            //     }
            //     else if (arr_type == GI_ARRAY_TYPE_BYTE_ARRAY) {

            //     }
            //     break;

            // case GI_TYPE_TAG_GHASH: // TODO: implement
            //     if (!js_value->IsObject()) {
            //         throw "TODO: js type error";
            //     }

            //     GITypeInfo *key_param_info, *val_param_info;
            //     //GHashTable *ghash;

            //     key_param_info = g_type_info_get_param_type(info, 0);
            //     g_assert(key_param_info != nullptr);
            //     val_param_info = g_type_info_get_param_type(info, 1);
            //     g_assert(val_param_info != nullptr);

            //     // TODO: implement

            //     g_base_info_unref((GIBaseInfo*) key_param_info);
            //     g_base_info_unref((GIBaseInfo*) val_param_info);

            //     break;

            case GI_TYPE_TAG_INTERFACE:
                {
                    GIBaseInfo *interface_info = g_type_info_get_interface(&argument_type_info);
                    GIInfoType interface_type = g_base_info_get_type(interface_info);

                    GType gtype;
                    switch(interface_type) {
                        case GI_INFO_TYPE_STRUCT:
                        case GI_INFO_TYPE_ENUM:
                        case GI_INFO_TYPE_OBJECT:
                        case GI_INFO_TYPE_INTERFACE:
                        case GI_INFO_TYPE_UNION:
                        case GI_INFO_TYPE_BOXED:
                            gtype = g_registered_type_info_get_g_type(interface_info);
                            break;
                        case GI_INFO_TYPE_VALUE:
                            gtype = G_TYPE_VALUE;
                            break;
                        default:
                            gtype = G_TYPE_NONE;
                            break;
                    }

                    // if the interface type is an object, then we expect
                    // the JS value to be a GIRObject so we can unwrap it
                    // and pass the GObject pointer to the GIArgument's v_pointer.
                    if (g_type_is_a(gtype, G_TYPE_OBJECT)) {
                        if (!js_value->IsObject()) {
                            throw JSArgumentTypeError();
                        }
                        GIRObject *gir_object = Nan::ObjectWrap::Unwrap<GIRObject>(js_value->ToObject());
                        argument_value.v_pointer = gir_object->obj;
                    }

                    // if it's a GValue then we need to use our GIRValue helper
                    // to convert it.
                    if (g_type_is_a(gtype, G_TYPE_VALUE)) {
                        GValue gvalue = {0, {{0}}};
                        if (!GIRValue::ToGValue(js_value, G_TYPE_INVALID, &gvalue)) {
                            throw "TODO: refactor GIRValue::ToGValue to throw appropriate errors";
                        }
                        // FIXME I've to free this somewhere
                        argument_value.v_pointer = g_boxed_copy(G_TYPE_VALUE, &gvalue);
                        g_value_unset(&gvalue); // TODO: make this exception safe
                    }
                }
                break;

            default:
                stringstream message;
                message << "argument type \"" << g_type_tag_to_string(argument_type_tag) << "\" is unsupported.";
                throw UnsupportedGIType(message.str());
        }
    } catch (JSArgumentTypeError &error) {
        // we want to nicely format all type errors so we'll catch them and rethrow using a nice message
        Nan::Utf8String js_type_name(js_value->TypeOf(Isolate::GetCurrent()));
        stringstream message;
        message << "Expected type '" << g_base_info_get_name(&argument_info);
        message << "' for Argument '" << g_type_tag_to_string(argument_type_tag);
        message << "' but got type '" << *js_type_name << "'";
        throw JSArgumentTypeError(message.str());
    }

    return argument_value;
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
                arr->Set(i, GIRObject::FromExisting(o, g_registered_type_info_get_g_type(interface_info)));
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
            Local<Value> new_instance = GIRObject::FromExisting(G_OBJECT(arg->v_pointer), g_registered_type_info_get_g_type(interface_info));
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
            return GIRObject::FromExisting(o, G_OBJECT_TYPE(o));
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

GITypeTag Args::MapGTypeTag(GITypeTag type) {
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

}
