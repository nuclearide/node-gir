#pragma once

#include <girepository.h>
#include <v8.h>

namespace gir {

using namespace v8;

class GIREnum {
public:
    static Local<Object> prepare(GIEnumInfo *enum_info);
};

} // namespace gir
