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
#include "log_writer.hpp"
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

static void writeInfoToTmpFiles(pid_t pid, pid_t child_pid) {
    int status = 0;
    pid_t wpid = waitpid(child_pid, &status, 0);
    
    int exitCode = -1;
    if (wpid > 0 && WIFEXITED(status)) {
       exitCode = WEXITSTATUS(status);
    }

    createJsFile(pid, exitCode);
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

    pid_t pid = getpid();
    listen_on_fds([pid](int size, char* buffer, int fd) {process(size, buffer, fd, pid);});
    writeInfoToTmpFiles(pid, fork_pid);

    return 0;
}
