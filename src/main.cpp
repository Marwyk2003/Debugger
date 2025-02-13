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
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>

#include "parser.hpp"
#include "fds_listner.hpp"
#include "package_header.hpp"
#include "child.hpp"
#include "const.hpp"
#include "colors.hpp"
#include "utilz.hpp"
#include "call_handler.hpp"

using namespace std;

static void fillLastEntryAndExit(map<string, ofstream>& pids, map<string, string>& dataMap) {
    // TODO: make js script to handle it
    for (auto& [k, _] : pids) {
        if (k == "0") continue;

        ifstream html(string(DEFAULT_PATH) + dataMap[k]);
        if (!html) continue;
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

        ofstream res(string(DEFAULT_PATH) + dataMap[k]);
        res << content;
        res.flush();
        res.close();
    }
}

int rootProcess(pid_t child_pid) {
    map<string, ofstream> streamMap;
    map<string, string> dataMap;
    streamMap.clear();
    dataMap.clear();

    char* static_filename = getenv(ENV_STATIC_FILENAME);
    if (static_filename) {
        dataMap[to_string(child_pid)] = static_filename;
    }

    auto proc = [&streamMap, &dataMap](int size, char* buf, int fd) {
        if (size == sizeof(package_header)) return; // empty log early return;
        parse_buffer(streamMap, dataMap, buf, fd == STDERR_FILENO, size);
        if (fd == STDERR_FILENO) write(fd, RED, 6);
        write(fd, buf + sizeof(package_header), size - sizeof(package_header));
        if (fd == STDERR_FILENO) write(fd, RESET, 5);
    };

    listen_on_fds(proc);

    for (auto& [k, v] : streamMap) {
        v.close();
    }

    fillLastEntryAndExit(streamMap, dataMap);
    return 0;
}

int main(int, char* argv[]) {
    if (string(argv[1]) == "--callhandler") {
        callhandlerProcess(argv[2], argv + 2); // ends with exec
    }

    char* debug_var = getenv(ENV_DEBUG);

    if (!debug_var) {
        char path[PATH_MAX];
        int count = readlink("/proc/self/exe", path, PATH_MAX);
        if (count > 0) {
            path[count] = 0;
            setenv(ENV_DEBUG, path, 1);
            string callhandler_var = string(path) + " --callhandler";
            setenv(ENV_CALLHANDLER, callhandler_var.c_str(), 1);
        } else {
            setenv(ENV_DEBUG, argv[0], 1);
            string callhandler_var = string(argv[0]) + " --callhandler";
            setenv(ENV_CALLHANDLER, callhandler_var.c_str(), 1);
        }

        mode_t old_mask = umask(0); // just in case
        string debugger_path = DEFAULT_PATH;
        mkdir(debugger_path.c_str(), 0777); // TODO think about permissions, maybe too loose
        char info_dir[] = TMP_INFO_DIR_PATH;
        mkdir(info_dir, 0777);
        umask(old_mask);

        deleteContentOfDir(string(TMP_INFO_DIR_PATH));
        createStyles(debugger_path);
        createIndex(debugger_path);
        

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

            int exit_code = rootProcess(pid);
            unsetenv(ENV_DEBUG);
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
