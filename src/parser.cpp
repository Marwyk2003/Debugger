#include <chrono>
#include <fstream>
#include <map>
#include <string>
#include <cstring>

#include "const.hpp"
#include "log_writer.hpp"
#include "package_header.hpp"

using namespace std;

void parse_buffer(map<string, ofstream>& pidMap, char* buf, bool isError, int end, const string& debugger_path) {
    string pid, ppid, name, time;
    package_header head;
    char tmp[BUF_SIZE];

    memcpy(&head, buf, sizeof(package_header));
    memcpy(tmp, buf + sizeof(package_header), end - sizeof(package_header));
    tmp[end - sizeof(package_header)] = 0;
    string line(tmp);
    pid = to_string(head.pid);
    ppid = to_string(head.parent_pid);
    time = to_string(head.time);

    if (head.type == 0) {
        if (pidMap.find(pid) == pidMap.end()) {
            string path = debugger_path + "/result_" + pid + ".html";
            pidMap[pid].open(path, ios::out | ios::trunc);
            writeHeader(pidMap[pid], line);
            writeLink(pidMap[ppid], time, pid, line);
        }
        ofstream& s = pidMap[pid];
        writeLine(s, line, time, isError);
    } else {
        writeLink(pidMap[ppid], time, pid, "link");
    }
}
