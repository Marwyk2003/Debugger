#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <chrono>
#include <string>
#include <functional>

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

int childProcess(char* program, char* argv[], char* static_pid) {
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

        execvp(program, argv);
    }

    // LISTENER <- PROGRAM
    close(pipe_fd_out[1]);
    close(pipe_fd_err[1]);
    dup2(pipe_fd_out[0], FD_OUT);
    dup2(pipe_fd_err[0], FD_ERR);

    pid_t pid = static_pid ? atoll(static_pid) : getpid();
    listen_on_fds([pid](int size, char* buffer, int fd) {process(size, buffer, fd, pid);});
    return 0;
}
