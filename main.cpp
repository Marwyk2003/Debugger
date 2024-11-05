#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <chrono>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;

#define ENV_NAME "DEBUG"
#define BUF_SIZE 1024
#define FD_OUT 3
#define FD_ERR 4
#define BEGIN_OUT "B_OUT"
#define END_OUT "E_OUT"

string program_name;
ofstream result;

void writePackage(char*,bool);

int rootProcess()
{
    result.open("result.html", ios::out | ios::app);
    // read input, create html etc.

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
                writePackage(buf, false);
        }

        if (!err_ended && FD_ISSET(FD_ERR, &read_fds))
        {
            memset(buf, 0, sizeof(buf));
            rbytes = read(FD_ERR, buf, BUF_SIZE);
            if (rbytes <= 0)
                err_ended = true;
            else
                writePackage(buf, true);
        }
    }
    result.close();
    return 0;
}

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
            auto now = chrono::system_clock::now();
            auto currentTime = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

            string pref = BEGIN_OUT + string("\n") + to_string(currentTime) + "\n", suf = END_OUT + string("\n");

            rbytes = read(FD_OUT, buf + pref.size(), BUF_SIZE);
            if (rbytes <= 0)
                out_ended = true;
            else
            {
                memcpy(buf, pref.c_str(), pref.size());                      // copy prefix
                memcpy(buf + rbytes + pref.size(), suf.c_str(), suf.size()); // copy suffix
                write(STDOUT_FILENO, buf, pref.size() + rbytes + suf.size());
            }
        }

        if (!err_ended && FD_ISSET(FD_ERR, &read_fds))
        {
            auto now = chrono::system_clock::now();
            auto currentTime = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

            string pref = BEGIN_OUT + string("\n") + to_string(currentTime) + "\n", suf = END_OUT + string("\n");

            rbytes = read(FD_ERR, buf + pref.size(), BUF_SIZE);
            if (rbytes <= 0)
                err_ended = true;
            else
            {
                memcpy(buf, pref.c_str(), pref.size());                      // copy prefix
                memcpy(buf + rbytes + pref.size(), suf.c_str(), suf.size()); // copy suffix
                write(STDERR_FILENO, buf, pref.size() + rbytes + suf.size());
            }
        }
    }
    result.close();
    return 0;
}

void writeHeader(string program_name){
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    result << R"(<!DOCTYPE html>
<html lang="pl-PL">
<head>
<meta charset="UTF-8" />
<link rel="stylesheet" href="styles.css" />
<title>)";
    result << program_name << "\n";
    result << R"(</title>
</head>
<body>
<div class="head">
<div class="info">
<span class="info-title">command:</span> <span class="info-value info-value-path">)" + program_name + R"(</span>
</div>
<div class="info">
<span class="info-title">start time:</span> <span class="info-value">)";
result << put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
result << R"(</span>
</div>
<div class="info">
<span class="info-title">last entry:</span> <span class="info-value"></span>
</div>
</div>
<div class="line"></div>
<div class="entries">
<table>
<tboby>
)";
    result.flush();
}

void writeLine(string line, string timeStr, bool isError){
    long long milliseconds = stoll(timeStr);
    auto duration = std::chrono::milliseconds(milliseconds);
    auto time_point = std::chrono::system_clock::time_point(duration);
    time_t time= std::chrono::system_clock::to_time_t(time_point);

    result << "<tr><td class=\"entry-time\">";
    result << put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    if(isError)
        result << "</td><td>&nbsp;</td><td class=\"entry-log entry-stderr\">";
    else
        result << "</td><td>&nbsp;</td><td class=\"entry-log entry-stdout\">";
    result << line;
    result << "</td></tr>\n";
    result.flush();
}

void writePackage(char * buf, bool isError){
    istringstream stream(buf);
    string line;
    string time;

    while (getline(stream, line)) {
        if(line == BEGIN_OUT){
            getline(stream, time);
            continue;
        }
        if(line == END_OUT){
            continue;
        }
        writeLine(line, time, isError);
    }
}

int main(int argc, char *argv[])
{

    result.open("result.html", ios::out | ios::trunc);
    writeHeader(argv[1]);
    result.close();
    char *env_var = getenv(ENV_NAME);
    if (!env_var)
    {
        putenv(argv[0]);

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
