#pragma once

#include <map>
#include <string>
#include <iostream>

#include "search_server.h"

bool AreSameWords(std::map<std::string, double> first, std::map<std::string, double> second);

void RemoveDuplicates(SearchServer& search_server);