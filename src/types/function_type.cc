#include "function_type.h"
#include "../function.h"
#include "../namespace_loader.h"
#include "../util.h"

#include <nan.h>
#include <node.h>
#include <cstring>

using namespace std;
using namespace v8;

namespace gir {

Local<Function> GIRFunction::prepare(GIFunctionInfo *function_info) {
    // Create new function
    Local<FunctionTemplate> js_function_template = Func::create_function(function_info);
    Local<Function> js_function = js_function_template->GetFunction();

    // Set the function name
    const char *native_name = g_base_info_get_name(function_info);
    string js_name = Util::to_camel_case(string(native_name));
    js_function->SetName(Nan::New(js_name.c_str()).ToLocalChecked());

    return js_function;
}

} // namespace gir
