#pragma once

#include <girepository.h>
#include <glib.h>
#include <nan.h>
#include <v8.h>
#include <vector>
#include "util.h"

namespace gir {

using namespace std;
using namespace v8;

class Args {
public:
    vector<GIArgument> in;
    vector<GIArgument> out;

    Args(GICallableInfo *callable_info);

    void load_js_arguments(const Nan::FunctionCallbackInfo<Value> &js_callback_info);
    void load_context(GObject *this_object);

private:
    GIRInfoUniquePtr callable_info;
    GIArgument get_in_argument_value(const Local<Value> &js_value, GIArgInfo &argument_info);
    GIArgument get_out_argument_value(GIArgInfo &argument_info);
    static GITypeTag map_g_type_tag(GITypeTag type);

public:
    // these functions are legacy and need to be refactored
    // there are many missing features within them as well such as missing type conversions (types that aren't supported
    // like structs.)
    static GIArgument to_g_type(GIArgInfo &argument_info, Local<Value> js_value);
    static Local<Value> from_g_type_array(GIArgument *arg, GIArgInfo *info, int array_length);
    static Local<Value> from_g_type(GIArgument *arg, GITypeInfo *type_info, int array_length);
};

} // namespace gir
