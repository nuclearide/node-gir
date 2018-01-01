#pragma once

#include <v8.h>
#include <girepository.h>

namespace gir {

using namespace v8;

class GIREnum {
  public:
    static Local<Object> Prepare(GIEnumInfo *enum_info);
};

}
