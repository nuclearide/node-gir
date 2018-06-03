#include "types/object.h"

namespace gir {

Local<Value> GIRParamSpec::from_existing(GParamSpec *param_spec) {
    auto gir_param_spec = new GIRParamSpec();
    gir_param_spec->param_spec = GIRInfoUniquePtr(param_spec);
    auto js_obj = Nan::New<Object>();
    gir_param_spec->Wrap(js_obj);
    return js_obj;
}

} // namespace gir
