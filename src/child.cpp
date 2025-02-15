/*
 * Copyright (C) 2025 M. Wykpis, A. Szwaja, P. Kubicki, S. Szulc, K. Socha
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <chrono>
#include <string>
#include <functional>
#include <sys/wait.h>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "const.hpp"
#include "child.hpp"
#include "fds_listner.hpp"
#include "package_header.hpp"

using namespace std;

static void process(int size, char* buf, int fd, pid_t pid) {
    package_header head;
    memcpy(&head, buf, sizeof(package_header));

    if (size < sizeof(package_header) || head.header_key != HEADER_CONST) { // new line creating header
        auto now = chrono::system_clock::now();
        auto currentTime = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

        head.type = 0;
        head.time = currentTime;
        head.pid = pid;
        head.parent_pid = 0;
        head.header_key = HEADER_CONST;

        write(fd, &head, sizeof(package_header));
    } else { // line with a header
        if (head.parent_pid == 0) { // line without parent
            head.parent_pid = getpid();
            head.time += 1;
            memcpy(buf, &head, sizeof(package_header));
        }
    }
    write(fd, buf, size);
}

static void writeInfoToTmpFiles(pid_t pid) {
    int status = 0;
    pid_t childPid = waitpid(pid, &status, 0);
    auto now = chrono::system_clock::now(); // last entry
    auto time = chrono::system_clock::to_time_t(now);
    
    int exitCode = -1;
    if (childPid > 0 && WIFEXITED(status)) {
       exitCode = WEXITSTATUS(status);
    }

    pid_t codedPid = getpid();

    ofstream exitCodeStream(string(TMP_INFO_DIR_PATH) + "/exitcode_" + to_string(codedPid));
    exitCodeStream << exitCode;
    exitCodeStream.flush();
    exitCodeStream.close();

    ofstream lastEntryStream(string(TMP_INFO_DIR_PATH) + "/lastentry_" + to_string(codedPid));
    lastEntryStream << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
    lastEntryStream.flush();
    lastEntryStream.close();
}

int childProcess(char* program, char* argv[]) {
    // create pipe LISTENER <- PROGRAM
    int pipe_fd_out[2], pipe_fd_err[2];
    pipe(pipe_fd_out);
    pipe(pipe_fd_err);

    pid_t fork_pid = fork();
    if (fork_pid == 0) {
        // PROGRAM -> LISTENER
        close(pipe_fd_out[0]);
        close(pipe_fd_err[0]);
        dup2(pipe_fd_out[1], STDOUT_FILENO);
        dup2(pipe_fd_err[1], STDERR_FILENO);

        write(STDOUT_FILENO, program, strlen(program));
        write(STDOUT_FILENO, "\n", 1);

        execvp(program, argv);
    }

    // LISTENER <- PROGRAM
    close(pipe_fd_out[1]);
    close(pipe_fd_err[1]);
    dup2(pipe_fd_out[0], FD_OUT);
    dup2(pipe_fd_err[0], FD_ERR);

    listen_on_fds([](int size, char* buffer, int fd) {process(size, buffer, fd, getpid());});
    writeInfoToTmpFiles(fork_pid);

    return 0;
}
