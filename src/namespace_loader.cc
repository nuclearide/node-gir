#include "namespace_loader.h"
#include "util.h"

#include "types/object.h"
#include "types/function_type.h"
#include "types/struct.h"
#include "types/enum.h"

#include <string.h>

namespace gir {

using namespace std;
using namespace v8;

GIRepository *NamespaceLoader::repo = nullptr;
std::map<char *, GITypelib*> NamespaceLoader::type_libs;

NAN_METHOD(NamespaceLoader::Load) {
    if (info.Length() < 1) {
        Nan::ThrowError("too few arguments");
    }
    if (!info[0]->IsString()) {
        Nan::ThrowError("argument has to be a string");
    }

    Local<Value> exports;
    String::Utf8Value namespace_(info[0]);

    if(info.Length() > 1 && info[1]->IsString()) {
        String::Utf8Value version(info[1]);
        exports = NamespaceLoader::load_namespace(*namespace_, *version);
    }
    else {
      exports = NamespaceLoader::load_namespace(*namespace_, nullptr);
    }

    info.GetReturnValue().Set(exports);
}

Handle<Value> NamespaceLoader::load_namespace(char *namespace_, char *version) {
  if (!repo) {
    repo = g_irepository_get_default();
  }

  GError *er = NULL;
  GITypelib *lib = g_irepository_require(repo, namespace_, version,
                                         (GIRepositoryLoadFlags)0, &er);
  if (!lib) {
    Nan::ThrowError(er->message);
  }

  type_libs.insert(std::make_pair(namespace_, lib));

  Handle<Value> res = build_classes(namespace_);
  return res;
}

Handle<Value> NamespaceLoader::build_classes(char *namespace_) {
  Handle<Object> module = Nan::New<Object>();
  Local<Value> exported_value = Nan::Null();

  int length = g_irepository_get_n_infos(repo, namespace_);
  for (int i = 0; i < length; i++) {
    GIBaseInfo *info = g_irepository_get_info(repo, namespace_, i);

    switch (g_base_info_get_type(info)) {
      case GI_INFO_TYPE_BOXED:
        // FIXME: GIStructInfo or GIUnionInfo
      case GI_INFO_TYPE_STRUCT:
        exported_value = GIRStruct::prepare((GIStructInfo *)info);
        break;
      case GI_INFO_TYPE_ENUM:
        exported_value = GIREnum::prepare((GIEnumInfo *)info);
        break;
      case GI_INFO_TYPE_FLAGS:
        exported_value = GIREnum::prepare((GIEnumInfo *)info);
        break;
      case GI_INFO_TYPE_OBJECT:
        exported_value = GIRObject::prepare((GIObjectInfo *)info);
        break;
      case GI_INFO_TYPE_INTERFACE:
        parse_interface((GIInterfaceInfo *)info, module);
        break;
      case GI_INFO_TYPE_UNION:
        parse_union((GIUnionInfo *)info, module);
        break;
      case GI_INFO_TYPE_FUNCTION:
        GIRFunction::initialize(module, (GIFunctionInfo *)info);
        break;
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
        // Do nothing
        break;
    }

    if (exported_value != Nan::Null()) {
      module->Set(Nan::New(g_base_info_get_name(info)).ToLocalChecked(),
                  exported_value);
      exported_value = Nan::Null();
    }

    g_base_info_unref(info);
  }

  // when all classes have been created we can inherit them
  GIRStruct::initialize(module, namespace_);

  return module;
}

void NamespaceLoader::parse_struct(GIStructInfo *info,
                                   Handle<Object> &exports) {}

void NamespaceLoader::parse_interface(GIInterfaceInfo *info,
                                      Handle<Object> &exports) {}

void NamespaceLoader::parse_union(GIUnionInfo *info, Handle<Object> &exports) {}

NAN_METHOD(NamespaceLoader::SearchPath) {
    if(!repo) {
        repo = g_irepository_get_default();
    }
    GSList *ls = g_irepository_get_search_path();
    int l = g_slist_length(ls);
    Local<Array> res = Nan::New<Array>(l);

    for(int i=0; i<l; i++) {
        gpointer p = g_slist_nth_data(ls, i);
        Nan::Set(res, Nan::New(i), Nan::New((gchar*)p).ToLocalChecked());
    }

    info.GetReturnValue().Set(res);
}

}
