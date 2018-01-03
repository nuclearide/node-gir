#include "function_type.h"
#include "../function.h"
#include "../namespace_loader.h"
#include "../util.h"

#include <nan.h>
#include <node.h>
#include <string.h>

using namespace std;
using namespace v8;

namespace gir {

void GIRFunction::initialize(Handle<Object> target, GIFunctionInfo *function_info) {
    const char *native_name = g_base_info_get_name(function_info);
    string js_name = Util::toCamelCase(string(native_name));

    // Create new function
    Local<FunctionTemplate> js_function_template = Func::create_function(function_info);
    Local<Function> js_function = js_function_template->GetFunction();

    // Set the function name
    js_function->SetName(Nan::New(js_name.c_str()).ToLocalChecked());

    // Set the function on the target object
    target->Set(Nan::New(js_name.c_str()).ToLocalChecked(), js_function);
}

} // namespace gir
