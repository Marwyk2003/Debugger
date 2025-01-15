#ifndef PARSER_HPP
#define PARSER_HPP

#include <chrono>
#include <fstream>
#include <map>

#include "const.hpp"
#include "log_writer.hpp"

using namespace std;

void parse_buffer(map<string, ofstream>& pidMap, char* buf, bool isError, int end, const string& debugger_path);

#endif // PARSER_HPP
