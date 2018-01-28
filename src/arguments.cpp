#include "arguments.h"
#include "values.h"

#include <cstring>
#include <sstream>
#include <vector>
#include "closure.h"
#include "exceptions.h"
#include "types/object.h"
#include "types/struct.h"

using namespace v8;

namespace gir {

Args::Args(GICallableInfo *callable_info) : callable_info(callable_info) {
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
void Args::load_js_arguments(const Nan::FunctionCallbackInfo<v8::Value> &js_callback_info) {
    // get the number of arguments the native function requires
    guint8 gi_argc = g_callable_info_get_n_args(this->callable_info.get());

    // for every expected native argument, we'll take a given JS argument and
    // convert it into a GIArgument, adding it to the in/out args array depending
    // on it's direction.
    for (guint8 i = 0; i < gi_argc; i++) {
        GIArgInfo argument_info;
        g_callable_info_load_arg(this->callable_info.get(), i, &argument_info);
        GIDirection argument_direction = g_arg_info_get_direction(&argument_info);

        if (argument_direction == GI_DIRECTION_IN) {
            GIArgument argument = Args::arg_to_g_type(argument_info, js_callback_info[i]);
            this->in.push_back(argument);
        }

        if (argument_direction == GI_DIRECTION_OUT) {
            GIArgument argument = this->get_out_argument_value(argument_info);
            this->out.push_back(argument);
        }

        if (argument_direction == GI_DIRECTION_INOUT) {
            GIArgument argument = Args::arg_to_g_type(argument_info, js_callback_info[i]);
            this->in.push_back(argument);

            // TODO: is it correct to handle INOUT arguments like IN args?
            // do we need to handle callee (native) allocates or empty input
            // GIArguments like we do with OUT args? i'm just assuming this is how it
            // should work (treating it like an IN arg). Hopfully I can find some
            // examples to make some test cases asserting the correct behaviour
            this->out.push_back(argument);
        }
    }
}

/**
 * This function loads the context (i.e. this value of `this`) into the native call arguments.
 * By convention, the context value (a GIRObject in JS or a GObject in native) is put at the
 * start (position 0) of the function call's "in" arguments.
 */
void Args::load_context(GObject *this_object) {
    GIArgument this_object_argument = {
        .v_pointer = this_object,
    };
    this->in.insert(this->in.begin(), this_object_argument);
}

GIArgument Args::get_out_argument_value(GIArgInfo &argument_info) {
    GITypeInfo argument_type_info;
    g_arg_info_load_type(&argument_info, &argument_type_info);
    if (g_arg_info_is_caller_allocates(&argument_info)) {
        GITypeTag arg_type_tag = g_type_info_get_tag(&argument_type_info);
        // If the caller is responsible for allocating the out arguments memeory
        // then we'll have to look up the argument's type infomation and allocate
        // a slice of memory for the GIArgument's .v_pointer (native function will
        // fill it up)
        if (arg_type_tag == GI_TYPE_TAG_INTERFACE) {
            GIRInfoUniquePtr argument_interface_info = GIRInfoUniquePtr(g_type_info_get_interface(&argument_type_info));
            GIInfoType argument_interface_type = g_base_info_get_type(argument_interface_info.get());
            gsize argument_size;

            if (argument_interface_type == GI_INFO_TYPE_STRUCT) {
                argument_size = g_struct_info_get_size((GIStructInfo *)argument_interface_info.get());
            } else if (argument_interface_type == GI_INFO_TYPE_UNION) {
                argument_size = g_union_info_get_size((GIUnionInfo *)argument_interface_info.get());
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
            //    passed back to JS then that JS object should own it and be
            //    responsible for cleaning it up.
            // * both choices have caveats so it's worth understanding the
            // implications
            //   of each, or other options to solve this leak!
            // * from reading GJS code, it seems like they copy the data before
            // passing
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
    GIArgument argument = {.v_pointer = nullptr};
    return argument;
}

GIArgument Args::arg_to_g_type(GIArgInfo &argument_info, Local<Value> js_value) {
    GITypeInfo argument_type_info;
    g_arg_info_load_type(&argument_info, &argument_type_info);
    GITypeTag argument_type_tag = g_type_info_get_tag(&argument_type_info);

    if (js_value->IsNullOrUndefined()) {
        if (g_arg_info_may_be_null(&argument_info) || argument_type_tag == GI_TYPE_TAG_VOID) {
            GIArgument argument_value;
            argument_value.v_pointer = nullptr;
            argument_value.v_string = nullptr;
            return argument_value;
        }
        stringstream message;
        message << "Argument '" << g_base_info_get_name(&argument_info) << "' may not be null or undefined";
        throw JSArgumentTypeError(message.str());
    }

    try {
        return Args::type_to_g_type(argument_type_info, js_value);
    } catch (JSArgumentTypeError &error) {
        // we want to nicely format all type errors so we'll catch them and rethrow
        // using a nice message
        Nan::Utf8String js_type_name(js_value->TypeOf(Isolate::GetCurrent()));
        stringstream message;
        message << "Expected type '" << g_type_tag_to_string(argument_type_tag);
        message << "' for Argument '" << g_base_info_get_name(&argument_info);
        message << "' but got type '" << *js_type_name << "'";
        throw JSArgumentTypeError(message.str());
    }
}

GIArgument Args::type_to_g_type(GITypeInfo &argument_type_info, Local<Value> js_value) {
    GITypeTag argument_type_tag = g_type_info_get_tag(&argument_type_info);

    // if the arg type is a GTYPE (which is an integer)
    // then we want to pretend it's a GI_TYPE_TAG_INTX
    // where x is the sizeof the GTYPE. This helper function
    // does the mapping for us.
    if (argument_type_tag == GI_TYPE_TAG_GTYPE) {
        argument_type_tag = Args::map_g_type_tag(argument_type_tag);
    }

    // this is what we'll return after we correctly
    // set it's values depending on the argument_type_tag
    // also, we'll make sure it's pointers are null initially!
    GIArgument argument_value;
    argument_value.v_pointer = nullptr;
    argument_value.v_string = nullptr;

    switch (argument_type_tag) {
        case GI_TYPE_TAG_VOID:
            argument_value.v_pointer = nullptr;
            break;

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
            } else {
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

        case GI_TYPE_TAG_INTERFACE: {
            auto interface_info = GIRInfoUniquePtr(g_type_info_get_interface(&argument_type_info));
            GIInfoType interface_type = g_base_info_get_type(interface_info.get());

            switch (interface_type) {
                case GI_INFO_TYPE_OBJECT:
                    // if the interface type is an object, then we expect
                    // the JS value to be a GIRObject so we can unwrap it
                    // and pass the GObject pointer to the GIArgument's v_pointer.
                    if (!js_value->IsObject()) {
                        throw JSArgumentTypeError();
                    } else {
                        GIRObject *gir_object = Nan::ObjectWrap::Unwrap<GIRObject>(js_value->ToObject());
                        argument_value.v_pointer = gir_object->get_gobject();
                    }
                    break;

                case GI_INFO_TYPE_VALUE:
                case GI_INFO_TYPE_STRUCT:
                case GI_INFO_TYPE_UNION:
                case GI_INFO_TYPE_BOXED: {
                    GType g_type = g_registered_type_info_get_g_type(interface_info.get());
                    if (g_type_is_a(g_type, G_TYPE_VALUE)) {
                        GValue gvalue = GIRValue::to_g_value(js_value, g_type);
                        argument_value.v_pointer = g_boxed_copy(g_type, &gvalue); // FIXME: should we copy? where do
                                                                                  // we deallocate?
                    } else {
                        GIRStruct *gir_struct = Nan::ObjectWrap::Unwrap<GIRStruct>(js_value->ToObject());
                        argument_value.v_pointer = gir_struct->get_native_ptr();
                    }
                } break;

                case GI_INFO_TYPE_FLAGS:
                case GI_INFO_TYPE_ENUM:
                    argument_value.v_int = js_value->IntegerValue();
                    break;

                case GI_INFO_TYPE_CALLBACK:
                    if (js_value->IsFunction()) {
                        auto closure = GIRClosure::create_ffi(interface_info.get(), js_value.As<Function>());
                        argument_value.v_pointer = closure;
                    } else {
                        throw JSArgumentTypeError();
                    }
                    break;

                default:
                    stringstream message;
                    message << "argument's with interface type \"" << g_info_type_to_string(interface_type)
                            << "\" is unsupported.";
                    throw UnsupportedGIType(message.str());
            }
        } break;

        default:
            stringstream message;
            message << "argument type \"" << g_type_tag_to_string(argument_type_tag) << "\" is unsupported.";
            throw UnsupportedGIType(message.str());
    }

    return argument_value;
}

Local<Value> Args::from_g_type_array(GIArgument *arg, GITypeInfo *type, int array_length) {
    GIArrayType array_type_info = g_type_info_get_array_type(type);
    auto element_type_info = GIRInfoUniquePtr(g_type_info_get_param_type(type, 0));
    GITypeTag param_tag = g_type_info_get_tag(element_type_info.get());

    switch (array_type_info) {
        case GI_ARRAY_TYPE_C:
            if (g_type_info_is_zero_terminated(type)) {
                GIArgument element;
                gpointer *native_array = (gpointer *)arg->v_pointer;
                Local<Array> js_array = Nan::New<Array>();
                for (int i = 0; native_array[i]; i++) {
                    element.v_pointer = native_array[i];
                    Local<Value> js_element = Args::from_g_type(&element, element_type_info.get(), 0);
                    js_array->Set(i, js_element);
                }
                return js_array;
            } else {
                // use array_length param once the layers above this
                // pas it correctly.
                // the length param comes from the native function call's
                // out arguments but, currently, it doesn't get passed own
                // here correctly.
                throw UnsupportedGIType("Converting non null terminated arrays is not yet supported");
            }
            break;
        default:
            throw UnsupportedGIType("cannot convert native array type");
    }

    stringstream message;
    message << "Converting array of type '" << g_type_tag_to_string(param_tag);
    message << "' is not supported";
    throw UnsupportedGIType(message.str());
}

// TODO: refactor this function and most of the code below this.
// can we reuse code from GIRValue?
Local<Value> Args::from_g_type(GIArgument *arg, GITypeInfo *type, int array_length) {
    GITypeTag tag = g_type_info_get_tag(type);

    switch (tag) {
        case GI_TYPE_TAG_VOID:
            return Nan::Undefined();

        case GI_TYPE_TAG_BOOLEAN:
            return Nan::New<Boolean>(arg->v_boolean);

        case GI_TYPE_TAG_INT8:
            return Nan::New(arg->v_int8);

        case GI_TYPE_TAG_UINT8:
            return Nan::New(arg->v_uint8);

        case GI_TYPE_TAG_INT16:
            return Nan::New(arg->v_int16);

        case GI_TYPE_TAG_UINT16:
            return Nan::New(arg->v_uint16);

        case GI_TYPE_TAG_INT32:
            return Nan::New(arg->v_int32);

        case GI_TYPE_TAG_UINT32:
            return Nan::New(arg->v_uint32);

        case GI_TYPE_TAG_INT64:
            // the ECMA script spec doesn't support int64 values
            // and V8 internally stores them as doubles.
            // This will lose precision if the original value
            // has more than 53 significant binary digits.
            return Nan::New(static_cast<double>(arg->v_int64));

        case GI_TYPE_TAG_UINT64:
            // the ECMA script spec doesn't support uint64 values
            // and V8 internally stores them as doubles.
            // This will lose precision if the original value
            // has more than 53 significant binary digits.
            return Nan::New(static_cast<double>(arg->v_uint64));

        case GI_TYPE_TAG_FLOAT:
            return Nan::New(arg->v_float);

        case GI_TYPE_TAG_DOUBLE:
            return Nan::New(arg->v_double);

        case GI_TYPE_TAG_GTYPE:
            return Nan::New(arg->v_uint);

        case GI_TYPE_TAG_UTF8:
            return Nan::New(arg->v_string).ToLocalChecked();

        case GI_TYPE_TAG_FILENAME:
            return Nan::New(arg->v_string).ToLocalChecked();

        case GI_TYPE_TAG_ARRAY:
            return Args::from_g_type_array(arg, type, array_length);

        case GI_TYPE_TAG_INTERFACE: {
            GIBaseInfo *interface_info = g_type_info_get_interface(type);
            GIInfoType interface_type = g_base_info_get_type(interface_info);
            switch (interface_type) {
                case GI_INFO_TYPE_OBJECT:
                    if (arg->v_pointer == nullptr) {
                        return Nan::Null();
                    }
                    return GIRObject::from_existing(G_OBJECT(arg->v_pointer), interface_info);

                case GI_INFO_TYPE_INTERFACE:
                case GI_INFO_TYPE_UNION:
                case GI_INFO_TYPE_STRUCT:
                case GI_INFO_TYPE_BOXED:
                    if (arg->v_pointer == nullptr) {
                        return Nan::Null();
                    }
                    return GIRStruct::from_existing(arg->v_pointer, interface_info);

                case GI_INFO_TYPE_VALUE:
                    return GIRValue::from_g_value(static_cast<GValue *>(arg->v_pointer), nullptr);

                case GI_INFO_TYPE_FLAGS:
                case GI_INFO_TYPE_ENUM:
                    return Nan::New(arg->v_int);

                // unsupported ...
                default:
                    stringstream message;
                    message << "cannot convert '" << g_info_type_to_string(interface_type) << "' to a JS value";
                    throw UnsupportedGIType(message.str());
            }
        } break;

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

GITypeTag Args::map_g_type_tag(GITypeTag type) {
    if (type == GI_TYPE_TAG_GTYPE) {
        switch (sizeof(GType)) {
            case 1:
                return GI_TYPE_TAG_UINT8;
            case 2:
                return GI_TYPE_TAG_UINT16;
            case 4:
                return GI_TYPE_TAG_UINT32;
            case 8:
                return GI_TYPE_TAG_UINT64;
            default:
                g_assert_not_reached();
        }
    }
    return type;
}

} // namespace gir
