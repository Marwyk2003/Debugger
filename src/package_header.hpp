#ifndef PACKAGE_HEADER_HPP
#define PACKAGE_HEADER_HPP

#include <unistd.h>
#include "const.hpp"

struct package_header
{
    int header = HEADER_CONST;
    int parent_pid;
    int pid;
    int64_t time;
};

#endif // PACKAGE_HEADER_HPP