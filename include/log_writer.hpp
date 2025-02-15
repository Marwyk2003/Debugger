/*
 * Copyright (C) 2025 M. Wykpis, A. Szwaja, P. Kubicki, S. Szulc, K. Socha
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef LOG_WRITER_HPP
#define LOG_WRITER_HPP

#include <chrono>
#include <fstream>
#include <string>
#include <sstream>


void writeHeader(std::ofstream& file_strem, std::string program_name);

void writeLine(std::ofstream& file_stream, std::string line, std::string time_str, bool is_error);

void writeLink(std::ofstream& result, std::string time_str, std::string pid, std::string name, std::string file_name);

void registerLink(std::string time_str, std::string pid, std::string name, std::string file_name);


#endif // LOG_WRITER_HPP
