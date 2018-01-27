#include "namespace_loader.h"
#include "types/enum.h"
#include "types/function.h"
#include "types/object.h"
#include "types/struct.h"
#include "util.h"

#include <cstring>

namespace gir {

using namespace std;

NAN_METHOD(NamespaceLoader::load) {
    if (info.Length() < 1) {
        Nan::ThrowError("too few arguments");
    }
    if (!info[0]->IsString()) {
        Nan::ThrowError("argument has to be a string");
    }
    Local<Value> exports;
    String::Utf8Value library_namespace(info[0]);
    if (info.Length() > 1 && info[1]->IsString()) {
        String::Utf8Value version(info[1]);
        exports = NamespaceLoader::load_namespace(*library_namespace, *version);
    } else {
        exports = NamespaceLoader::load_namespace(*library_namespace, nullptr);
    }
    info.GetReturnValue().Set(exports);
}

Handle<Value> NamespaceLoader::load_namespace(const char *library_namespace, const char *version) {
    auto repository = g_irepository_get_default();
    GError *error = nullptr;
    g_irepository_require(repository, library_namespace, version, (GIRepositoryLoadFlags)0, &error);
    if (error != nullptr) {
        Nan::ThrowError(error->message);
        g_error_free(error);
        return Nan::Undefined();
    }
    return NamespaceLoader::build_exports(library_namespace);
}

Handle<Value> NamespaceLoader::build_exports(const char *library_namespace) {
    auto repository = g_irepository_get_default();
    Handle<Object> module = Nan::New<Object>();
    Local<Value> exported_value = Nan::Null();

    int length = g_irepository_get_n_infos(repository, library_namespace);
    for (int i = 0; i < length; i++) {
        auto info = GIRInfoUniquePtr(g_irepository_get_info(repository, library_namespace, i));

        switch (g_base_info_get_type(info.get())) {
            case GI_INFO_TYPE_OBJECT:
                exported_value = GIRObject::prepare(info.get());
                break;
            case GI_INFO_TYPE_FUNCTION:
                exported_value = GIRFunction::prepare(info.get());
                break;
            case GI_INFO_TYPE_BOXED:
            case GI_INFO_TYPE_STRUCT:
                exported_value = GIRStruct::prepare(info.get());
                break;
            case GI_INFO_TYPE_ENUM:
                exported_value = GIREnum::prepare(info.get());
                break;
            case GI_INFO_TYPE_FLAGS:
                exported_value = GIREnum::prepare(info.get());
                break;
            case GI_INFO_TYPE_UNION:
            case GI_INFO_TYPE_INTERFACE:
            case GI_INFO_TYPE_INVALID:
            case GI_INFO_TYPE_CALLBACK:
            case GI_INFO_TYPE_CONSTANT:
            case GI_INFO_TYPE_INVALID_0:
            case GI_INFO_TYPE_VALUE:
            case GI_INFO_TYPE_SIGNAL:
            case GI_INFO_TYPE_VFUNC:
            case GI_INFO_TYPE_PROPERTY:
            case GI_INFO_TYPE_FIELD:
            case GI_INFO_TYPE_ARG:
            case GI_INFO_TYPE_TYPE:
            case GI_INFO_TYPE_UNRESOLVED:
                // do nothing
                break;
        }

        if (exported_value != Nan::Null()) {
            string exported_name = Util::base_info_canonical_name(info.get());
            module->Set(Nan::New(exported_name).ToLocalChecked(), exported_value);
            exported_value = Nan::Null();
        }
    }

    return module;
}

} // namespace gir
