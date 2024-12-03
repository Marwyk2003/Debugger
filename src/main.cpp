#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <chrono>
#include <iomanip>

#include "parser.hpp"
#include "const.hpp"

using namespace std;

int rootProcess()
{
    map<string, ofstream> pids;
    pids.clear();

    // SELECT
    fd_set read_fds;
    int select_fd = 5;
    fcntl(FD_OUT, F_SETFL, O_NONBLOCK);
    fcntl(FD_ERR, F_SETFL, O_NONBLOCK);

    char buf[BUF_SIZE];
    int rbytes;
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
            memset(buf, 0, sizeof(buf));
            rbytes = read(FD_OUT, buf, BUF_SIZE);
            if (rbytes <= 0)
                out_ended = true;
            else
                writePackage(pids, buf, false);
        }

        if (!err_ended && FD_ISSET(FD_ERR, &read_fds))
        {
            memset(buf, 0, sizeof(buf));
            rbytes = read(FD_ERR, buf, BUF_SIZE);
            if (rbytes <= 0)
                err_ended = true;
            else
                writePackage(pids, buf, true);
        }
    }

    for (auto &[k, v] : pids)
        v.close();
    return 0;
}

void sendPacket(bool &, char *, char *);

int childProcess(char *program, char *argv[])
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

    // SELECT
    fd_set read_fds;
    int select_fd = 5;
    fcntl(FD_OUT, F_SETFL, O_NONBLOCK);
    fcntl(FD_ERR, F_SETFL, O_NONBLOCK);

    char buf[BUF_SIZE];
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
            sendPacket(out_ended, buf, program);
        }
        if (!err_ended && FD_ISSET(FD_ERR, &read_fds))
        {
            sendPacket(err_ended, buf, program);
        }
    }
    return 0;
}

void sendPacket(bool &input_ended, char *buf, char *program_name)
{
    auto now = chrono::system_clock::now();
    auto currentTime = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

    string pref = BEGIN_OUT + string("\n") + to_string(getpid()) + "\n" + program_name + '\n' + to_string(currentTime) + "\n", suf = END_OUT + string("\n");

    int rbytes = read(FD_OUT, buf + pref.size(), BUF_SIZE);
    if (rbytes <= 0)
        input_ended = true;
    else
    {
        memcpy(buf, pref.c_str(), pref.size());                      // copy prefix
        memcpy(buf + rbytes + pref.size(), suf.c_str(), suf.size()); // copy suffix
        write(STDOUT_FILENO, buf, pref.size() + rbytes + suf.size());
    }
}

int main(int, char *argv[])
{
    char *env_var = getenv(ENV_NAME);
    if (!env_var)
    {
        setenv(ENV_NAME, argv[0], 1);

        // create pipe ROOT <- LISTENER 1
        int pipe_fd_out[2], pipe_fd_err[2];
        pipe(pipe_fd_out);
        pipe(pipe_fd_err);

        pid_t pid = fork();
        if (pid != 0)
        {
            // ROOT <- CHILD
            close(pipe_fd_out[1]);
            close(pipe_fd_err[1]);
            dup2(pipe_fd_out[0], FD_OUT);
            dup2(pipe_fd_err[0], FD_ERR);

            int exit_code = rootProcess();
            unsetenv(ENV_NAME);
            return exit_code;
        }
        else
        {
            // CHILD -> ROOT
            close(pipe_fd_out[0]);
            close(pipe_fd_err[0]);
            dup2(pipe_fd_out[1], STDOUT_FILENO);
            dup2(pipe_fd_err[1], STDERR_FILENO);
        }
    }
    int exit_code = childProcess(argv[1], argv + 1);
    return exit_code;
}
