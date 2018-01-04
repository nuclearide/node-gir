#include "./enum.h"
#include <nan.h>
#include <string>
#include "../util.h"

using namespace std;

namespace gir {

Local<Object> GIREnum::prepare(GIEnumInfo *enum_info) {
    Local<Object> js_enum_object = Nan::New<Object>();
    // for every value in the enum, convert the key to uppercase
    // and then set key=value on a JS object that will represent the enum
    for (int i = 0; i < g_enum_info_get_n_values(enum_info); i++) {
        GIValueInfo *value = g_enum_info_get_value(enum_info, i);
        string native_enum_key_name = string(g_base_info_get_name(value));
        Util::to_upper_case(native_enum_key_name);
        Local<String> enum_key_name = Nan::New(native_enum_key_name.c_str()).ToLocalChecked();
        Local<Number> enum_key_value = Nan::New<Number>(g_value_info_get_value(value));
        js_enum_object->Set(enum_key_name, enum_key_value);
        g_base_info_unref(value);
    }
    return js_enum_object;
}

} // namespace gir
