#ifndef PACKAGE_HEADER_HPP
#define PACKAGE_HEADER_HPP

#include <unistd.h>
#include "const.hpp"

struct package_header {
    unsigned int header_key;
    pid_t parent_pid;
    pid_t pid;
    int64_t time;
};

#endif // PACKAGE_HEADER_HPP
