#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <limits.h>

#include "parser.hpp"
#include "child.hpp"
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
                parse_buffer(pids, buf, false);
        }

        if (!err_ended && FD_ISSET(FD_ERR, &read_fds))
        {
            memset(buf, 0, sizeof(buf));
            rbytes = read(FD_ERR, buf, BUF_SIZE);
            if (rbytes <= 0)
                err_ended = true;
            else
                parse_buffer(pids, buf, true);
        }
    }

    for (auto &[k, v] : pids)
        v.close();
    return 0;
}

int main(int, char *argv[])
{
    char *env_var = getenv(ENV_NAME);
    if (!env_var)
    {

        char path[PATH_MAX];
        int count = readlink("/proc/self/exe", path, PATH_MAX);
        if(count > 0){
            path[count] = 0;
            setenv(ENV_NAME, path, 1);
        }else{
            setenv(ENV_NAME, argv[0], 1);
        }

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
