#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace std;

string ENV_NAME = "DEBUG";
uint BUF_SIZE = 1024;
string SPECIAL_PREF = "_META_\n";
string SPECIAL_SUF = "\n";

int rootProcess()
{
    // read input, create html etc.

    char buf[BUF_SIZE];
    int rbytes;
    while ((rbytes = read(STDIN_FILENO, buf, BUF_SIZE)))
    {
        if (rbytes < 0)
            break;
        write(STDOUT_FILENO, buf, rbytes);
    }

    return 0;
}

int childProcess(char *program, char *argv[])
{
    // create pipe LISTENER <- PROGRAM
    int pipe_fd[2];
    pipe(pipe_fd);

    pid_t pid = fork();
    if (pid == 0)
    {
        // PROGRAM -> LISTENER
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);

        execvp(program, argv);
    }

    // LISTENER <- PROGRAM
    close(pipe_fd[1]);
    dup2(pipe_fd[0], STDIN_FILENO);

    int pref_size = SPECIAL_PREF.size(), suf_size = SPECIAL_SUF.size();
    char buf[BUF_SIZE + pref_size + suf_size];
    int rbytes;
    while ((rbytes = read(STDIN_FILENO, buf + pref_size, BUF_SIZE)))
    {
        if (rbytes < 0)
            break;

        // TODO: Skip already marked data

        memcpy(buf, SPECIAL_PREF.c_str(), pref_size);
        memcpy(buf + rbytes + pref_size, SPECIAL_SUF.c_str(), suf_size);
        write(STDOUT_FILENO, buf, pref_size + rbytes + suf_size);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    char *env_var = getenv(ENV_NAME.c_str());
    if (!env_var)
    {
        putenv(argv[0]);

        // create pipe ROOT <- LISTENER 1
        int pipe_fd[2];
        pipe(pipe_fd);

        pid_t pid = fork();
        if (pid != 0)
        {
            // ROOT <- CHILD
            close(pipe_fd[1]);
            dup2(pipe_fd[0], STDIN_FILENO);

            uint exit_code = rootProcess();
            unsetenv(ENV_NAME.c_str());
            return exit_code;
        }
        else
        {
            // CHILD -> ROOT
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
        }
    }

    uint exit_code = childProcess(argv[1], argv + 1);
    return exit_code;
}