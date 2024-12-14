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

using namespace std;

void sendPacket(int, bool&, char*, char*);

static void process(int end, char* buf, int fd){
    write(STDOUT_FILENO, buf, end);
}

int childProcess(char* program, char* argv[])
{
    // create pipe LISTENER <- PROGRAM
    int pipe_fd_out[2], pipe_fd_err[2];
    pipe(pipe_fd_out);
    pipe(pipe_fd_err);

    pid_t pid = fork();
    if (pid == 0)
    {
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

    listen_on_fds(process);
    return 0;
}

void sendPacket(int fd, bool& input_ended, char* buf, char* program_name) {
    auto now = chrono::system_clock::now();
    auto currentTime = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

    string pref = BEGIN_OUT + string("\n") + to_string(getpid()) + "\n" + program_name + '\n' + to_string(currentTime) + "\n", suf = END_OUT + string("\n");

    int rbytes = read(fd, buf + pref.size(), BUF_SIZE);
    if (rbytes <= 0)
        input_ended = true;
    else
    {
        memcpy(buf, pref.c_str(), pref.size());                      // copy prefix
        memcpy(buf + rbytes + pref.size(), suf.c_str(), suf.size()); // copy suffix
        write(fd - 2, buf, pref.size() + rbytes + suf.size());
    }
}
