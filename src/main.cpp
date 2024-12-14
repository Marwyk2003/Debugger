#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <limits.h>
#include <functional>

#include "parser.hpp"
#include "fds_listner.hpp"
#include "child.hpp"
#include "const.hpp"

using namespace std;

int rootProcess()
{
    map<string, ofstream> pids;
    pids.clear();

    auto proc = [&pids](int end, char* buf, int fd)-> void {
        write(STDOUT_FILENO, buf, end);
        return;
    };

    listen_on_fds(proc);

    for (auto& [k, v] : pids)
        v.close();
    return 0;
}

int main(int, char* argv[])
{
    char* env_var = getenv(ENV_NAME);
    if (!env_var)
    {

        char path[PATH_MAX];
        int count = readlink("/proc/self/exe", path, PATH_MAX);
        if (count > 0) {
            path[count] = 0;
            setenv(ENV_NAME, path, 1);
        }
        else {
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
