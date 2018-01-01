#ifndef UTIL_H
#define UTIL_H

#include <v8.h>
#include <glib.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

extern "C" void debug_printf(const char *fmt, ...);

namespace gir {

namespace Util {
    gchar *utf8StringFromValue(v8::Handle<v8::Value> value);
    string toCamelCase(const string input);
    string toSnakeCase(const string input);
    void toUpperCase(string &input);

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
        for (auto const& element : input_map) {
            retval.push_back(element.first);
        }
        return retval;
    }

    template<typename TK, typename TV>
    vector<TV> extract_values(map<TK, TV> &input_map) {
        vector<TV> retval;
        retval.reserve(input_map.size());
        for (auto const& element : input_map) {
            retval.push_back(element.second);
        }
        return retval;
    }
}

}

#endif
