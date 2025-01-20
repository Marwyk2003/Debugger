#ifndef LOG_WRITER_HPP
#define LOG_WRITER_HPP

#include <chrono>
#include <fstream>
#include <string>
#include <sstream>


void writeHeader(std::ofstream& file_strem, std::string program_name);

void writeLine(std::ofstream& file_stream, std::string line, std::string time_str, bool is_error);

void writeLink(std::ofstream& result, std::string time_str, std::string pid, std::string name);

void registerLink(std::string time_str, std::string pid, std::string name, const std::string& debugger_path);


#endif // LOG_WRITER_HPP
