#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <limits.h>
#include <functional>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>

#include "parser.hpp"
#include "fds_listner.hpp"
#include "package_header.hpp"
#include "child.hpp"
#include "const.hpp"
#include "colors.hpp"
#include "utilz.hpp"

using namespace std;

static void fillLastEntryAndExit(map<string, ofstream>& pids) {
    // TODO: make js script to handle it
    for (auto& [k, _] : pids) {
        if (k == "0") continue;
        ifstream html(string(DEFAULT_DEBUG_OUTPUT_DIR) + "/result_" + k + ".html");
        stringstream buff;
        buff << html.rdbuf();
        html.close();
        string content = buff.str();
        
        buff = stringstream();
        ifstream lastEntry(string(TMP_INFO_DIR_PATH) + "/lastentry_" + k);
        buff << lastEntry.rdbuf();
        lastEntry.close();
        string lastEntryValue = buff.str();

        buff = stringstream();
        ifstream exitCode(string(TMP_INFO_DIR_PATH) + "/exitcode_" + k);
        buff << exitCode.rdbuf();
        exitCode.close();
        string exitCodeValue = buff.str();

        if (exitCodeValue != "-1") {
            size_t pos = 0;
            string exitSpanOld = R"(<span class="info-title exitno">exit ?</span>)";
            string exitSpanNew = (exitCodeValue == "0") ? R"(<span class="info-title exitok">exit 0</span>)"
                                                        : R"(<span class="info-title exiterr">exit )" + exitCodeValue + R"(</span>)";
            pos = content.find(exitSpanOld, pos);
            content.replace(pos, exitSpanOld.length(), exitSpanNew);
        }

        size_t pos = 0;
        string lastEntryOld = R"(<span class="info-title">last entry:</span> <span class="info-value"></span>)";
        string lastEntryNew = R"(<span class="info-title">last entry:</span><span class="info-value"> )" + lastEntryValue + R"(</span>)";
        pos = content.find(lastEntryOld, pos);
        content.replace(pos, lastEntryOld.length(), lastEntryNew);
        
        ofstream res(string(DEFAULT_DEBUG_OUTPUT_DIR) + "/result_" + k + ".html");
        res << content;
        res.flush();
        res.close();
    }
}

int rootProcess() {
    map<string, ofstream> pids;
    pids.clear();

    auto proc = [&pids](int size, char* buf, int fd) {
        if (size == sizeof(package_header)) return; // empty log early return;
        parse_buffer(pids, buf, fd == STDERR_FILENO, size);
        if (fd == STDERR_FILENO) write(fd, RED, 6);
        write(fd, buf + sizeof(package_header), size - sizeof(package_header));
        if (fd == STDERR_FILENO) write(fd, RESET, 5);
        };

    listen_on_fds(proc);

    for (auto& [k, v] : pids) {
        v.close();
    }

    fillLastEntryAndExit(pids);
    return 0;
}

int main(int, char* argv[]) {
    char* env_var = getenv(ENV_NAME);
    if (!env_var) {
        char path[PATH_MAX];
        int count = readlink("/proc/self/exe", path, PATH_MAX);
        if (count > 0) {
            path[count] = 0;
            setenv(ENV_NAME, path, 1);
        } else {
            setenv(ENV_NAME, argv[0], 1);
        }

        mode_t old_mask = umask(0); // just in case
        char dir[] = DEFAULT_DEBUG_OUTPUT_DIR;
        mkdir(dir, 0777); // TODO think about permissions, maybe to loose
        char info_dir[] = TMP_INFO_DIR_PATH;
        mkdir(info_dir, 0777);
        umask(old_mask);

        deleteContentOfDir(string(TMP_INFO_DIR_PATH));
        createStyles(string(DEFAULT_DEBUG_OUTPUT_DIR));
        createIndex(string(DEFAULT_DEBUG_OUTPUT_DIR));

        // create pipe ROOT <- LISTENER 1
        int pipe_fd_out[2], pipe_fd_err[2];
        pipe(pipe_fd_out);
        pipe(pipe_fd_err);

        pid_t pid = fork();
        if (pid != 0) {
            // ROOT <- CHILD
            close(pipe_fd_out[1]);
            close(pipe_fd_err[1]);
            dup2(pipe_fd_out[0], FD_OUT);
            dup2(pipe_fd_err[0], FD_ERR);

            int exit_code = rootProcess();
            unsetenv(ENV_NAME);
            return exit_code;
        } else {
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
