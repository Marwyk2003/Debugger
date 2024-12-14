#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <chrono>
#include <string>
#include <functional>

#include "fds_listner.hpp"
#include "const.hpp"

static void read_to_buffer(int fd, char* buf, int& buf_end, bool& ended){
    ssize_t bytes_read = read(fd, buf + buf_end, BUF_SIZE - buf_end);
    if (bytes_read > 0) {
        buf_end += bytes_read;
    } else if (bytes_read == 0) {
        ended = true; // EOF
    }
}

static void process_buffer(int fd, char* buf, int & buf_end, bool& ended, std::function<void(int,char*,int)> process){
    while(true){
        int i;
        for(i =0; i<buf_end; i++){
            if(buf[i] == '\n'){
                break;
            }
        }
        if(i == buf_end){
            if(ended){
                process(i, buf, fd);
                buf_end = 0;
            }
            break;
        }
        process(i + 1, buf, fd);
        memmove(buf, buf + i + 1, (buf_end - i - 1)*sizeof(char));
        buf_end -= i + 1;
    }
}

void listen_on_fds(std::function<void(int,char*,int)> process){

    fd_set read_fds;
    int select_fd = 5;
    fcntl(FD_OUT, F_SETFL, O_NONBLOCK);
    fcntl(FD_ERR, F_SETFL, O_NONBLOCK);

    char buf_out[BUF_SIZE];
    char buf_err[BUF_SIZE];
    int buf_out_end = 0, buf_err_end = 0;
    bool out_ended = false, err_ended = false;
    while (!out_ended || !err_ended)
    {
        FD_ZERO(&read_fds);
        FD_SET(FD_OUT, &read_fds);
        FD_SET(FD_ERR, &read_fds);

        int s = select(select_fd, &read_fds, nullptr, nullptr, nullptr);
        if (s == -1)
            break;

        if (!out_ended && FD_ISSET(FD_OUT, &read_fds))
        {
            read_to_buffer(FD_OUT, buf_out, buf_out_end, out_ended);
            process_buffer(FD_OUT, buf_out, buf_out_end, out_ended, process);
        }

        if (!err_ended && FD_ISSET(FD_ERR, &read_fds))
        {
            read_to_buffer(FD_ERR, buf_err, buf_err_end, err_ended);
            process_buffer(FD_ERR, buf_err, buf_err_end, err_ended, process);
        }
    }
}