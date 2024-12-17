#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <chrono>

#include "src/package_header.hpp"

using namespace std::chrono;

int main(int, char* argv[]) {
    pid_t pid = getpid();

    auto now = system_clock::now();
    auto currentTime = duration_cast<milliseconds>(now.time_since_epoch()).count();

    unsetenv("DEBUG");
    setenv("DEBUG_STATIC_PID", std::to_string(pid).c_str(), 1);

    package_header head;
    head.type = 1;
    head.header_key = HEADER_CONST;
    head.pid = pid;
    head.parent_pid = 0;
    head.time = currentTime;

    write(STDOUT_FILENO, &head, sizeof(package_header));
    write(STDOUT_FILENO, "\n", 1);

    execvp(argv[1], argv + 1);
}
