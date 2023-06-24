#include "process_queries.h"

using namespace std;

vector<vector<Document>> ProcessQueries(const SearchServer& search_server, const vector<string>& queries) {
    vector<vector<Document>> dst(queries.size());

    transform(execution::par, queries.begin(), queries.end(), dst.begin(), [&search_server](const string& s) {
        return search_server.FindTopDocuments(s);
    });

    return dst;
}

list<Document> ProcessQueriesJoined(const SearchServer& search_server, const vector<string>& queries) {
    list<Document> dst;

    for (const vector<Document>& item : ProcessQueries(search_server, queries)) {
        for (const Document& doc : item) {
            dst.push_back(doc);
        }
    }

    return dst;
}