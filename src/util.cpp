#include "util.h"
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <regex>

using namespace std;

namespace gir {

namespace Util {

/**
 * This function returns a new string, converting the input
 * string from snake case to camel case.
 * e.g. 'set_label' --> 'setLabel'
 */
string to_camel_case(const string input) {
    string output;
    bool next_is_capital = false;
    for (size_t i = 0; i < input.size(); i++) {
        char letter = input[i];
        if (next_is_capital) {
            output.append(1, toupper(letter));
            next_is_capital = false;
        } else {
            if (letter == '_' && i != 0) {
                next_is_capital = true;
            } else {
                output.append(1, letter);
            }
        }
    }
    return output;
}

string to_snake_case(const string input) {
    string output;
    for (auto letter : input) {
        if (isupper(letter)) { // FIXME: this will break on methods that begin with a capital (i should write a test
                               // case!)
            output.append(1, '_');
            output.append(1, (char)tolower(letter));
        } else {
            output.append(1, letter);
        }
    }
    return output;
}

/**
 * modifies the input string
 */
void to_upper_case(string &input) {
    transform(input.begin(), input.end(), input.begin(), [](unsigned char character) { return toupper(character); });
}

string base_info_canonical_name(GIBaseInfo *base_info) {
    const gchar *original_name = g_base_info_get_name(base_info);
    return to_camel_case(string(original_name));
}

// TODO: I think this can segfault the caller because of c_str().
vector<const char *> strings_to_cstrings(vector<string> &string_vector) {
    vector<const char *> c_string_vector;
    c_string_vector.reserve(string_vector.size());
    for (auto &i : string_vector) {
        c_string_vector.push_back(const_cast<char *>(i.c_str()));
    }
    return c_string_vector;
}

} // namespace Util
} // namespace gir
