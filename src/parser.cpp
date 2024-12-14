#include <chrono>
#include <fstream>
#include <map>

#include "const.hpp"
#include "log_writer.hpp"

using namespace std;

void parse_buffer(map<string, ofstream>& pidMap, char* buf, bool isError) {
    istringstream stream(buf);
    string line;
    string pid, ppid, name, time;

    while (getline(stream, line)) {
        if (line == BEGIN_OUT) {
            ppid = pid;
            getline(stream, pid);
            getline(stream, name);
            getline(stream, time);
            continue;
        }
        if (line == END_OUT)
            continue;

        if (pidMap.find(pid) == pidMap.end()) {
            pidMap[pid].open("result_" + pid + ".html", ios::out | ios::trunc);
            writeHeader(pidMap[pid], name);
            writeLink(pidMap[ppid], time, pid, name);
        }
        ofstream& s = pidMap[pid];
        writeLine(s, line, time, isError);
    }
}
