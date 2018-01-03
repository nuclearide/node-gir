#ifndef UTIL_H
#define UTIL_H

#include <girepository.h>
#include <glib.h>
#include <v8.h>
#include <map>
#include <string>
#include <vector>

using namespace std;

extern "C" void debug_printf(const char *fmt, ...);

namespace gir {

struct GIBaseInfoDeleter {
    void operator()(GIBaseInfo *info) const;
};

/**
 * This alias allows for a more readable way of wrapping a GIBaseInfo pointer
 * or some derivative (GICallableInfo, etc) in a std::unique_ptr with a custom
 * deleter that calls `g_base_info_unref`.
 * @example
 * // this
 * unique_ptr<GIBaseInfo, void(*)(GIBaseInfo *)> argument_interface_info = unique_ptr<GIBaseInfo, void(*)(GIBaseInfo
 * *)>(g_type_info_get_interface(&argument_type_info));
 * // becomes
 * GIRInfoUniquePtr argument_interface_info = GIRInfoUniquePtr(g_type_info_get_interface(&argument_type_info));
 */
using GIRInfoUniquePtr = unique_ptr<GIBaseInfo, GIBaseInfoDeleter>;

namespace Util {

gchar *utf8StringFromValue(v8::Handle<v8::Value> value);
string toCamelCase(const string input);
string toSnakeCase(const string input);
void toUpperCase(string &input);
string base_info_canonical_name(GIBaseInfo *base_info);

/**
 * this uses the same underlying values as the string_vector
 * i.e. it does not copy the data! the output of this function
 * should not be mutated!
 */
vector<const char *> stringsToCStrings(vector<string> &string_vector);

template<typename TK, typename TV>
vector<TK> extract_keys(map<TK, TV> &input_map) {
    vector<TK> retval;
    retval.reserve(input_map.size());
    for (auto const &element : input_map) {
        retval.push_back(element.first);
    }
    return retval;
}

template<typename TK, typename TV>
vector<TV> extract_values(map<TK, TV> &input_map) {
    vector<TV> retval;
    retval.reserve(input_map.size());
    for (auto const &element : input_map) {
        retval.push_back(element.second);
    }
    return retval;
}
} // namespace Util

} // namespace gir

#endif
