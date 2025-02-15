#include <iomanip>
#include <pwd.h>
#include <unistd.h>
#include <fstream>

#include "const.hpp"
#include "utilz.hpp"
#include "log_writer.hpp"

using namespace std;

void writeHeader(ofstream& result, string program_name, string pid) {
    auto now = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(now);
    result << R"(
<!DOCTYPE html>
<html lang="pl-PL">
<head>
<meta charset="UTF-8" />
<link rel="stylesheet" href="styles.css" />
<title>)" << program_name << "\n"<< R"(
</title>
</head>
<body>
<div class="head">
<div class="info">
<span class="info-title">command:</span>
<span class="info-value info-value-path">)" << program_name << R"(</span>
</div>
<div class="info">
<span class="info-title">start time:</span>
<span class="info-value">)" << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S") << R"(</span>
</div>
<div class="info">
<span class="info-title">last entry:</span> <span class="info-value" id="last_entry"></span>
</div>
<div class="info">
<span class="info-title"><span class="exitno" id="exit_code_wrapper">exit <span id="exit_code"></span textContent="?"></span></span>
</div>
<script src="./)" << pid << R"(.js"></script>
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

    string debugger_path = getOutputPath();

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

    string debugger_path = getOutputPath();

    ofstream res(debugger_path + "/" + hostname + "/" + user_name  + "/index.html", std::ios::app);
    res << "<tr><td class=\"entry-time\">";
    res << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
    res << "</td><td>&nbsp;</td><td class=\"entry-log entry-link\"><a href=" << debugger_path + "/all_logs" + file_name << ">" << name << " </a></td></tr>\n";
    res.flush();
    res.close();
}

void createJsFile(pid_t pid, int exit_code) {
    string debugger_path = getOutputPath();
    string path = debugger_path + "/all_logs/" + to_string(pid) + ".js";

    auto now = chrono::system_clock::now(); // last entry
    auto time = chrono::system_clock::to_time_t(now);

    fstream js_file;
    js_file.open(path, ios::out | ios::trunc);
    js_file << R"(document.getElementById("exit_code").textContent=")" << exit_code << R"(";)"
            << R"(document.getElementById("last_entry").textContent=")" << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S") << R"(";)";
            
    if (exit_code) js_file << R"(document.getElementById("exit_code_wrapper").className="exiterr")";
    else js_file << R"(document.getElementById("exit_code_wrapper").className="exitok")";
}
