/*
 * Copyright (C) 2025 M. Wykpis, A. Szwaja, P. Kubicki, S. Szulc, K. Socha
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <chrono>

#include "package_header.hpp"
#include "parser.hpp"

using namespace std;
using namespace chrono;

void callhandlerProcess(char* program, char* argv[]) {
    pid_t pid = getpid();

    auto now = system_clock::now();
    auto currentTime = duration_cast<milliseconds>(now.time_since_epoch()).count();

    unsetenv(ENV_DEBUG);

    string filename = get_file_name(to_string(currentTime), "nohup");
    setenv(ENV_STATIC_FILENAME, filename.c_str(), 1);

    package_header head;
    head.type = 1;
    head.header_key = HEADER_CONST;
    head.pid = pid;
    head.parent_pid = 0;
    head.time = currentTime;

    write(STDOUT_FILENO, &head, sizeof(package_header));
    write(STDOUT_FILENO, filename.c_str(), filename.size());
    write(STDOUT_FILENO, "\n", 1);

    execvp(program, argv);
}
