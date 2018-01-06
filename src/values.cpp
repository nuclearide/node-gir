#include "values.h"
#include "namespace_loader.h"
#include "util.h"

#include <cstdlib>
#include <sstream>
#include "exceptions.h"
#include "types/object.h"
#include "types/struct.h"

using namespace v8;

namespace gir {

Handle<Value> GIRValue::from_g_value(GValue *v, GIBaseInfo *base_info) {
    GType type = G_VALUE_TYPE(v);
    Handle<Value> value = Nan::Undefined();
    const char *tmpstr;
    char *str;
    GIBaseInfo *boxed_info;

    switch (G_TYPE_FUNDAMENTAL(type)) {
        case G_TYPE_CHAR:
            str = new char[2];
            str[0] = g_value_get_schar(v);
            str[1] = '\0';
            value = Nan::New<String>(str).ToLocalChecked();
            delete[] str;
            return value;

        case G_TYPE_UCHAR:
            str = new char[2];
            str[0] = g_value_get_uchar(v);
            str[1] = '\0';
            value = Nan::New<String>(str).ToLocalChecked();
            delete[] str;
            return value;

        case G_TYPE_BOOLEAN:
            return Nan::New<Boolean>(g_value_get_boolean(v));

        case G_TYPE_INT:
            return Nan::New<Number>(g_value_get_int(v));

        case G_TYPE_UINT:
            return Nan::New<Number>(g_value_get_uint(v));

        case G_TYPE_LONG:
            return Nan::New<Number>(g_value_get_long(v));

        case G_TYPE_ULONG:
            return Nan::New<Number>(g_value_get_ulong(v));

        case G_TYPE_INT64:
            return Nan::New<Number>(g_value_get_int64(v));

        case G_TYPE_UINT64:
            return Nan::New<Number>(g_value_get_uint64(v));

        case G_TYPE_ENUM:
            return Nan::New<Number>(g_value_get_enum(v));

        case G_TYPE_FLAGS:
            return Nan::New<Number>(g_value_get_flags(v));

        case G_TYPE_FLOAT:
            return Nan::New<Number>(g_value_get_float(v));

        case G_TYPE_DOUBLE:
            return Nan::New<Number>(g_value_get_double(v));

        case G_TYPE_STRING:
            tmpstr = g_value_get_string(v);
            return Nan::New<String>(tmpstr ? tmpstr : "").ToLocalChecked();

        case G_TYPE_BOXED:
            if (G_VALUE_TYPE(v) == G_TYPE_ARRAY) {
                Nan::ThrowError("GIRValue - GValueArray conversion not supported");
            } else {
                // Handle C structure held by boxed type
                if (base_info == nullptr)
                    Nan::ThrowError("GIRValue - missed base_info for boxed type");
                boxed_info = g_irepository_find_by_gtype(g_irepository_get_default(), G_VALUE_TYPE(v));
                return GIRStruct::from_existing((GIRStruct *)g_value_get_boxed(v), boxed_info);
            }
            break;

        case G_TYPE_OBJECT:
            return GIRObject::from_existing(G_OBJECT(g_value_get_object(v)), base_info);

        default:
            Nan::ThrowError("GIRValue - conversion of '%s' type not supported");
    }

    return value;
}

// TODO: refactor to follow the style that Args::ToGType does
// i.e. return a GValue and throw std::exceptions on failure
GValue GIRValue::to_g_value(Local<Value> js_value, GType g_type) {
    GValue g_value = G_VALUE_INIT;

    if (g_type == G_TYPE_INVALID || g_type == 0) {
        g_type = GIRValue::guess_type(js_value);
    }

    if (g_type == G_TYPE_INVALID) {
        throw JSValueError("Could not guess the native value type from JS");
    }

    g_value_init(&g_value, g_type);

    switch (G_TYPE_FUNDAMENTAL(g_type)) {
        case G_TYPE_INTERFACE:
        case G_TYPE_OBJECT:
            g_value_set_object(&g_value, Nan::ObjectWrap::Unwrap<GIRObject>(js_value->ToObject())->get_gobject());
            break;

        case G_TYPE_CHAR: {
            String::Utf8Value value(js_value->ToString());
            g_value_set_schar(&g_value, (*value)[0]);
        } break;

        case G_TYPE_UCHAR: {
            String::Utf8Value value(js_value->ToString());
            g_value_set_uchar(&g_value, (*value)[0]);
        } break;

        case G_TYPE_BOOLEAN:
            g_value_set_boolean(&g_value, js_value->BooleanValue());

            break;

        case G_TYPE_INT:
            g_value_set_int(&g_value, js_value->Int32Value());
            break;

        case G_TYPE_UINT:
            g_value_set_uint(&g_value, js_value->Uint32Value());
            break;

        case G_TYPE_LONG:
            g_value_set_long(&g_value, js_value->NumberValue());
            break;

        case G_TYPE_ULONG:
            g_value_set_ulong(&g_value, js_value->NumberValue());
            break;

        case G_TYPE_INT64:
            g_value_set_int64(&g_value, js_value->IntegerValue());
            break;

        case G_TYPE_UINT64:
            g_value_set_uint64(&g_value, js_value->IntegerValue());
            break;

        case G_TYPE_ENUM:
            g_value_set_enum(&g_value, js_value->IntegerValue());
            break;

        case G_TYPE_FLAGS:
            g_value_set_flags(&g_value, js_value->IntegerValue());
            break;

        case G_TYPE_FLOAT:
            g_value_set_float(&g_value, js_value->NumberValue());
            break;

        case G_TYPE_DOUBLE:
            g_value_set_double(&g_value, js_value->NumberValue());
            break;

        case G_TYPE_STRING: {
            String::Utf8Value value(js_value->ToString());
            g_value_set_string(&g_value, *value);
        } break;

        case G_TYPE_BOXED:
            if (g_type_is_a(g_type, G_TYPE_VALUE)) {
                // we have a special case for GValue itself. If we need to convert a JS
                // value into a GValue we must guess the native type. The easiest
                // way to do that is to just call this same function (to_g_value).
                // GIRValue::guess_type can't return G_TYPE_VALUE so we're save
                // from infinite recursion.
                return GIRValue::to_g_value(js_value, GIRValue::guess_type(js_value));
            } else {
                // otherwise we expect the js value to be a GIRStruct
                // FIXME: error handling for invalid unwraps
                GIRStruct *gir_struct = Nan::ObjectWrap::Unwrap<GIRStruct>(js_value->ToObject());
                g_value_set_boxed(&g_value, gir_struct->get_native_ptr());
            }
            break;

        case G_TYPE_PARAM:
        case G_TYPE_POINTER: {
            stringstream message;
            message << "Native value type '" << g_type_name(g_type) << "' is not yet supported";
            throw UnsupportedGValueType(message.str());
        } break;

        default:
            throw JSValueError("Failed to convert value");
            break;
    }
    return g_value;
}

GType GIRValue::guess_type(Handle<Value> value) {
    if (value->IsString()) {
        return G_TYPE_STRING;
    }

    if (value->IsArray()) {
        return G_TYPE_ARRAY;
    }

    if (value->IsBoolean()) {
        return G_TYPE_BOOLEAN;
    }

    if (value->IsInt32()) {
        return G_TYPE_INT;
    }

    if (value->IsUint32()) {
        return G_TYPE_UINT;
    }

    if (value->IsNumber()) {
        return G_TYPE_DOUBLE;
    }

    if (value->IsNull()) {
        return G_TYPE_POINTER;
    }

    return G_TYPE_INVALID;
}

} // namespace gir
