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
#include <algorithm>

#include "fds_listner.hpp"
#include "package_header.hpp"
#include "const.hpp"

static bool read_to_buffer(int fd, char* buf, int& buf_size, bool ended) {
    ssize_t bytes_read = read(fd, buf + buf_size, BUF_SIZE - buf_size);
    if (bytes_read > 0) {
        buf_size += bytes_read;
    } else if (bytes_read == 0) {
        ended = true; // EOF 
    }
    return ended;
}

static void process_buffer(char* buf, int& buf_size, bool& ended, std::function<void(int, char*)> process) {
    if (buf_size == 0) return;
    unsigned int header_key;
    int start, i;
    while (true) {
        memcpy(&header_key, buf, sizeof(int));
        start = (header_key == HEADER_CONST) ? sizeof(package_header) : 0;
        for (i = start; i < buf_size; i++) {
            if (buf[i] == '\n') {
                break;
            }
        }
        if (i >= buf_size) {
            if (ended) {
                process(buf_size, buf);
                buf_size = 0;
            }
            break;
        }
        process(i + 1, buf);
        memmove(buf, buf + i + 1, (buf_size - i - 1) * sizeof(char));
        buf_size -= i + 1;
    }
}

void listen_on_fds(std::function<void(int, char*, int)> process) {
    fd_set read_fds;
    int select_fd = 5;
    fcntl(FD_OUT, F_SETFL, O_NONBLOCK);
    fcntl(FD_ERR, F_SETFL, O_NONBLOCK);

    char buf_out[BUF_SIZE], buf_err[BUF_SIZE];
    int buf_out_size = 0, buf_err_size = 0;
    bool out_ended = false, err_ended = false;
    while (!out_ended || !err_ended) {
        FD_ZERO(&read_fds);
        FD_SET(FD_OUT, &read_fds);
        FD_SET(FD_ERR, &read_fds);

        int s = select(select_fd, &read_fds, nullptr, nullptr, nullptr);
        if (s == -1)
            break;

        if (!out_ended && FD_ISSET(FD_OUT, &read_fds)) {
            out_ended = read_to_buffer(FD_OUT, buf_out, buf_out_size, out_ended);
            process_buffer(buf_out, buf_out_size, out_ended, [process](int size, char* buf) {process(size, buf, STDOUT_FILENO);});
        }

        if (!err_ended && FD_ISSET(FD_ERR, &read_fds)) {
            err_ended = read_to_buffer(FD_ERR, buf_err, buf_err_size, err_ended);
            process_buffer(buf_err, buf_err_size, err_ended, [process](int size, char* buf) {process(size, buf, STDERR_FILENO);});
        }
    }
}
