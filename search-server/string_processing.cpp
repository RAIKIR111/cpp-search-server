#include "string_processing.h"

using namespace std;

vector<string_view> SplitIntoWords(const string_view& input_text) {
    string_view text = input_text;

    vector<string_view> dst;

    text.remove_prefix(min(text.find_first_not_of(" "), text.size()));

    if (text.empty()) return dst;
    
    while (true) {
        int64_t space = text.find(" ");
        dst.push_back(text.substr(0, space));

        if (space == -1) break;

        text.remove_prefix(++space);

        int64_t remove = text.find_first_not_of(" "s);

        if (remove == -1) break;

        text.remove_prefix(remove);
    }
    return dst;
}