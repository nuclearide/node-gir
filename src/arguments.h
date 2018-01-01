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
    GITypeInfo *return_type_info;

    Args(GICallableInfo *callable_info);
    ~Args();

    void loadJSArguments(const Nan::FunctionCallbackInfo<v8::Value> &js_callback_info);
    void loadContext(GObject *this_object);

  private:
    GICallableInfo *callable_info = nullptr;
    GIArgument GetInArgumentValue(const Local<Value> &js_value, GIArgInfo &argument_info);
    GIArgument GetOutArgumentValue(GIArgInfo &argument_info);

  public:
    // these functions are legacy and need to be refactored
    // there are many missing features within them as well such as missing type conversions (types that aren't supported like structs.)
    static bool ToGType(v8::Handle<v8::Value>, GIArgument *arg, GIArgInfo *info, GITypeInfo *type_info, bool out);
    static v8::Handle<v8::Value> FromGTypeArray(GIArgument *arg, GIArgInfo *info, int array_length);
    static v8::Local<v8::Value> FromGType(GIArgument *arg, GIArgInfo *info, int array_length);
    static inline GITypeTag ReplaceGType(GITypeTag type);
    static bool ArrayToGList(v8::Handle<v8::Array> arr, GIArgInfo *info, GList **list_p);
    static bool ArrayToGList(v8::Handle<v8::Array> arr, GIArgInfo *info, GSList **list_p);
};

}
