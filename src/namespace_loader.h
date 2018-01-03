#pragma once

#include <map>
#include <v8.h>
#include <glib.h>
#include <girepository.h>
#include <nan.h>

namespace gir {

class NamespaceLoader : public Nan::ObjectWrap {
  public:
    static GIRepository *repo;
    static std::map<char *, GITypelib*> type_libs;

    static NAN_METHOD(Load);
    static NAN_METHOD(SearchPath);

  private:
   static v8::Handle<v8::Value> load_namespace(char *namespace_, char *version);
   static v8::Handle<v8::Value> build_classes(char *namespace_);

   static void parse_struct(GIStructInfo *info,
                                                        v8::Handle<v8::Object> &exports);
   static void parse_interface(GIInterfaceInfo *info,
                                                              v8::Handle<v8::Object> &exports);
   static void parse_union(GIUnionInfo *info, v8::Handle<v8::Object> &exports);
   static void parse_function(GIFunctionInfo *info,
                                                            v8::Handle<v8::Object> &exports);
};

}
