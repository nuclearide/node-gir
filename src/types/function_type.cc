#include "function_type.h"
#include "../namespace_loader.h"
#include "../util.h"
#include "../function.h"

#include <string.h>
#include <node.h>
#include <nan.h>

using namespace std;
using namespace v8;

namespace gir {

void GIRFunction::Initialize(Handle<Object> target, GIObjectInfo *info)
{
    const char *native_name = g_base_info_get_name(info);
    const char *js_name = string(Util::toCamelCase(string(native_name))).c_str();

    // Create new function
    Local<FunctionTemplate> js_function_template = Nan::New<FunctionTemplate>(GIRFunction::Execute);
    Local<Function> js_function = js_function_template->GetFunction();

    // Set the function name
    js_function->SetName(Nan::New(js_name).ToLocalChecked());

    // Create external to hold GIBaseInfo and set it as a private data property
    v8::Handle<v8::External> info_ptr = Nan::New<v8::External>((void*)g_base_info_ref(info));
    Nan::SetPrivate(js_function, Nan::New("GIInfo").ToLocalChecked(), info_ptr);

    // Set the function on the target object
    target->Set(Nan::New(js_name).ToLocalChecked(), js_function);
}

NAN_METHOD(GIRFunction::Execute)
{
    // Get GIFunctionInfo pointer
    v8::Handle<v8::External> info_ptr = v8::Handle<v8::External>::Cast(Nan::GetPrivate(info.Callee(), Nan::New("GIInfo").ToLocalChecked()).ToLocalChecked());
    GIBaseInfo *func  = (GIBaseInfo*) info_ptr->Value();

    debug_printf("EXECUTE namespace: '%s',  name: '%s', symbol: '%s' \n",
            g_base_info_get_namespace(func),
            g_base_info_get_name(func),
            g_function_info_get_symbol(func));

    if(func) {
        info.GetReturnValue().Set(Func::Call(NULL, func, info, TRUE));
    }
    else {
        Nan::ThrowError("no such function");
    }

    info.GetReturnValue().SetUndefined();
}

}
