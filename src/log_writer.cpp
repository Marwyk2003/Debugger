#include<iomanip>
#include <pwd.h>
#include <unistd.h>

#include "const.hpp"
#include "utilz.hpp"
#include "log_writer.hpp"

using namespace std;

void writeHeader(ofstream& result, string program_name) {
    auto now = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(now);
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
<span class="info-title">command:</span> <span class="info-value info-value-path">)" +
program_name + R"(</span>
</div>
<div class="info">
<span class="info-title">start time:</span> <span class="info-value">)";
    result << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
    result << R"(</span>
</div>
<div class="info">
<span class="info-title">last entry:</span> <span class="info-value"></span>
</div>
<div class="info">
<span class="info-title exitno">exit ?</span>
</div>
<div class="line"></div>
<div class="entries">
<table>
<tboby>
)";
    result.flush();
}

void writeLine(ofstream& result, string line, string timeStr, bool isError) {
    long long milliseconds = stoll(timeStr);
    auto duration = chrono::milliseconds(milliseconds);
    auto time_point = chrono::system_clock::time_point(duration);
    time_t time = chrono::system_clock::to_time_t(time_point);

    result << "<tr><td class=\"entry-time\">";
    result << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
    if (isError)
        result << "</td><td>&nbsp;</td><td class=\"entry-log entry-stderr\">";
    else
        result << "</td><td>&nbsp;</td><td class=\"entry-log entry-stdout\">";
    result << line;
    result << "</td></tr>\n";
    result.flush();
}

void writeLink(ofstream& result, string timeStr, string pid, string name, string file_name) {
    long long milliseconds = stoll(timeStr);
    auto duration = chrono::milliseconds(milliseconds);
    auto time_point = chrono::system_clock::time_point(duration);
    time_t time = chrono::system_clock::to_time_t(time_point);

    string debugger_path = getDebuggerPath();

    result << "<tr><td class=\"entry-time\">";
    result << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
    result << "</td><td>&nbsp;</td><td class=\"entry-log entry-link\"><a href=" << debugger_path + "/all_logs" + file_name << ">" << name << " </a></td></tr>\n";
}

void registerLink(string timeStr, string pid, string name, string file_name) {
    long long milliseconds = stoll(timeStr);
    auto duration = chrono::milliseconds(milliseconds);
    auto time_point = chrono::system_clock::time_point(duration);
    time_t time = chrono::system_clock::to_time_t(time_point);

    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    struct passwd *pw = getpwuid(getuid());
    char* user_name = pw->pw_name;

    string debugger_path = getDebuggerPath();

    ofstream res(debugger_path + "/" + hostname + "/" + user_name  + "/index.html", std::ios::app);
    res << "<tr><td class=\"entry-time\">";
    res << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
    res << "</td><td>&nbsp;</td><td class=\"entry-log entry-link\"><a href=" << debugger_path + "/all_logs" + file_name << ">" << name << " </a></td></tr>\n";
    res.flush();
    res.close();
}
