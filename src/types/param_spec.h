#pragma once

#include <glib.h>
#include <nan.h>
#include <v8.h>

namespace gir {

using namespace v8;
using namespace std;

/**
 * This class wraps a native GParamSpec.
 * I'm not yes sure exactly how this should be implemented but both GJS
 * and PyGObject use a runtime wrapper object so I think it's safe to say
 * node-gir should do something similar. This class is intended as that
 * runtime wrapper.
 * for future reference, here are the docs for the implementation in:
 * - PyGObject: https://lazka.github.io/pgi-docs/GObject-2.0/classes/ParamSpec.html
 * - GJS: https://github.com/GNOME/gjs/blob/master/gi/param.cpp#L206
 */
class GIRParamSpec : public Nan::ObjectWrap {
private:
    GIRInfoUniquePtr param_spec;

public:
    static Local<Value> from_existing(GParamSpec *param_spec);
};

} // namespace gir
