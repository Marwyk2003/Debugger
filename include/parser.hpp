#ifndef PARSER_HPP
#define PARSER_HPP

#include <chrono>
#include <fstream>
#include <map>

#include "const.hpp"
#include "log_writer.hpp"

using namespace std;

void parse_buffer(map<string, ofstream>& pidMap, map<string, string>& dataMap, char* buf, bool isError, int end);

string get_file_name(string time, string line, string pid);

#endif // PARSER_HPP
