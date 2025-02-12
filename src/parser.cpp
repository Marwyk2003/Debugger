#include <chrono>
#include <fstream>
#include <map>
#include <string>
#include <cstring>
#include <iomanip>
#include <string>
#include <algorithm>
#include <pwd.h>

#include "const.hpp"
#include "log_writer.hpp"
#include "package_header.hpp"
#include "parser.hpp"

using namespace std;

void parse_buffer(map<string, ofstream>& streamMap, map<string, string>& dataMap, char* buf, bool isError, int end) {
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

    static bool firstOccurence = true;

    if (head.type == 0) {
        if (streamMap.find(pid) == streamMap.end()) {
            if (dataMap.find(pid) == dataMap.end()) {
                dataMap[pid] = get_file_name(time, line);
            }
            string file_name = dataMap[pid];

            if (firstOccurence){
                firstOccurence = false;
                registerLink(time, pid, line, file_name);
            }
            
            string path = string(DEFAULT_PATH) + file_name;
            streamMap[pid].open(path, ios::out | ios::trunc);
            writeHeader(streamMap[pid], line);
            writeLink(streamMap[ppid], time, pid, line, file_name);
        }
        ofstream& s = streamMap[pid];
        writeLine(s, line, time, isError);
    } else {
        string filename = head.type == 1 ? line : dataMap[pid]; 
        writeLink(streamMap[ppid], time, pid, "link", filename);
    }
}


string get_file_name(string time, string line){
    struct passwd *pw = getpwuid(getuid());
    char* dir = pw->pw_dir;
    char* user_name = pw->pw_name;

    long long milliseconds = stoll(time);
    auto duration = chrono::milliseconds(milliseconds);
    auto time_point = chrono::system_clock::time_point(duration);
    time_t time_t = chrono::system_clock::to_time_t(time_point);
    ostringstream oss;
    oss << put_time(localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    string time_string = oss.str();
    
    erase(line, '\n');

    replace(line.begin(), line.end(), '.', '_');
    replace(line.begin(), line.end(), '/', '_');

    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    string file_name = "/" + time_string + "_" + string(user_name) + "@" + hostname + "_" + line + ".html";
    replace(file_name.begin(), file_name.end(), ' ', '_');

    return file_name;
}
