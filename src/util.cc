#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <regex>
#include <cctype>
#include <algorithm>

using namespace std;

static char *_format_message(const char *fmt, va_list args)
{
  if(!fmt)
    return g_strdup("");

  return g_strdup_vprintf(fmt, args);
}

extern "C" void debug_printf(const char *fmt, ...)
{
  const char *debug = getenv("NODE_GIR_DEBUG");
  if (debug == NULL || *debug == '\0') {
    return;
  }
  printf(" DEBUG: ");

  va_list args;
  va_start (args, fmt);
  char *msg = _format_message(fmt, args);
  va_end(args);

  printf("%s", msg);
  g_free(msg);
}

vector<string> split_string(const string &text, const regex &re, bool include_full_match = false) {
  auto parts = vector<string>(
    sregex_token_iterator(text.begin(), text.end(), re, -1),
    sregex_token_iterator()
  );
  return parts;
}

namespace gir {
namespace Util {

gchar *utf8StringFromValue(v8::Handle<v8::Value> value) {
  v8::Local<v8::String> str = value->ToString();
  gchar *buffer = g_new(gchar, str->Utf8Length());
  str->WriteUtf8(buffer);
  return buffer;
}

/**
 * This function returns a new string, converting the input
 * string from snake case to camel case.
 * e.g. 'set_label' --> 'setLabel'
 */
string toCamelCase(const string input) {
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

string toSnakeCase(const string input) {
  string output;
  for (auto letter : input) {
    if (isupper(letter)) { // FIXME: this will break on methods that begin with a capital (i should write a test case!)
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
void toUpperCase(string &input) {
  transform(input.begin(), input.end(), input.begin(), [](unsigned char character) {
    return toupper(character);
  });
}

// TODO: I think this can segfault the caller because of c_str().
vector<const char *> stringsToCStrings(vector<string> &string_vector) {
    vector<const char *> c_string_vector;
    c_string_vector.reserve(string_vector.size());
    for (size_t i = 0; i < string_vector.size(); i++) {
        c_string_vector.push_back(const_cast<char *>(string_vector[i].c_str()));
    }
    return c_string_vector;
}

}
}
