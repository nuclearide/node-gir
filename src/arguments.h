#pragma once

#include <vector>
#include <v8.h>
#include <glib.h>
#include <girepository.h>
#include <nan.h>

namespace gir {

using namespace std;
using namespace v8;

class Args {
  public:
    vector<GIArgument> in;
    vector<GIArgument> out;

    Args(GICallableInfo *callable_info);

    void loadJSArguments(const Nan::FunctionCallbackInfo<Value> &js_callback_info);
    void loadContext(GObject *this_object);

  private:
    unique_ptr<GICallableInfo, void(*)(GICallableInfo*)> callable_info;
    GIArgument GetInArgumentValue(const Local<Value> &js_value, GIArgInfo &argument_info);
    GIArgument GetOutArgumentValue(GIArgInfo &argument_info);
    static GITypeTag MapGTypeTag(GITypeTag type);

  public:
    // these functions are legacy and need to be refactored
    // there are many missing features within them as well such as missing type conversions (types that aren't supported like structs.)
    static GIArgument ToGType(GIArgInfo &argument_info, Local<Value> js_value);
    static Local<Value> FromGTypeArray(GIArgument *arg, GIArgInfo *info, int array_length);
    static Local<Value> FromGType(GIArgument *arg, GIArgInfo *info, int array_length);
};

}
