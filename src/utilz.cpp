#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <string>

#include <iostream>

#include "utilz.hpp"



void deleteContentOfDir(const std::string& path) {
    DIR* dir = opendir(path.c_str());

    struct dirent* entry;
    struct stat fileStat;

    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        std::string fullPath = path + "/" + entry->d_name;

        if (stat(fullPath.c_str(), &fileStat) == 0) {
            if (S_ISDIR(fileStat.st_mode)) {
                deleteContentOfDir(fullPath);
                rmdir(fullPath.c_str());
            } else {
                remove(fullPath.c_str());
            }
        }
    }
    closedir(dir);
}

void createStyles(const std::string& path) {
    std::ofstream styles(path + "/styles.css");
    styles << R"(body {
  background-color: #272822;
  margin: 0;
  padding: 0
}

table {
	font-size: 14px;
}

a:link {
  color: #9270ff;
  text-decoration: none;
}

a:visited {
  color: #7178ff;
  text-decoration: none;
}

div.line {
  border-bottom: 2px solid #00AAAA;
  position: absolute;
  width: 100%;
}

div.head {
  margin: 10px;
  padding: 0
}

div.info {
	font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
	font-size: 15px;
	font-style: normal;
	font-variant: normal;
	font-weight: 500;
	line-height: 22px;
}

span.info-title {
  font-style: italic;
  color: #00AAAA;
}

span.info-value {
  color:  #F8F8F2;
	 font-size: 15px;
}

a.info-value {
 color: #F8F8F2;
}

span.info-value-path {
	font-family: monospace;
	font-size: 15px;
	font-style: normal;
	font-variant: normal;
	font-weight: 100;
	line-height: 18px;
}

a.info-value-fromcall-ok {
  color: #00AA00;
}

a.info-value-fromcall-err {
  color: #BB1111
}

a.info-value-fromcall-wait {
  color: #BBBB11
}

span.exitok {
	color: #00AA00;
}

span.exiterr {
	color: #CC1111
}

span.exitno {
	color: #AAAA00;
}

div.headder {
	color:  #F8F8F2;
	 font-size: 25px;
}

div.entries {
	padding: 10px;
	font-family: monospace;
	font-size: 14px;
	font-style: normal;
	font-variant: normal;
	font-weight: 500;
	line-height: 18px;
}

table {
  border-collapse: collapse;
}

td.entry-time {
	font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
	font-size: 14px;
	font-style: normal;
	font-variant: normal;
	font-weight: 500;
	line-height: 18px;
	font-style: italic;
	color: #B8B8B2;
  text-align: right;
  width: 150px;
  display: inline-block;
}

td.entry-log {
  border-left: 1px solid #B8B8B2;
  padding-left: 10px;
}

td.entry-stdout {
  color:  #F8F8F2;
}

td.entry-stderr {
  color:  #CC5500;
}

td.entry-call-wait {
  color:  #BBBB11;
}

td.entry-call-ok {
  color:  #00AA00;
}

td.entry-call-err {
  color:  #BB1111;
}

a.entry-call {
	overflow: hidden;
	white-space: nowrap;
}

a.entry-call-ok {
  color: #00AA00;
}

a.entry-call-err {
  color: #BB1111
}

a.entry-call-wait {
  color: #BBBB11
}

span.entry-exit {
	font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
	font-size: 15px;
	font-style: normal;
	font-variant: normal;
	font-weight: 500;
	line-height: 22px;
	font-style: italic;
	padding-right: 0px;
})";
    
    styles.close();
}


void createIndex(const std::string& path) {
    std::ifstream fileCheck(path + "/index.html");    
    if (fileCheck.good()) return;

    std::ofstream index(path + "/index.html");
    index << R"(<!DOCTYPE html>
<html lang="pl-PL">
<head>
<meta charset="UTF-8" />
<link rel="stylesheet" href="styles.css" />
<title>test/test-basic.sh

</title>
</head>
<body>
<div class="head">
<div class="headder"> DEBUGGER </div>
<div class="info">
<span class="info-title">list of recent debugging logs</span>
</div>
<div class="line"></div>
<div class="entries">
<table>
<tboby>)";

    index.close();
}

std::string getDebugDir(){
    const char* home = std::getenv("HOME");
    return std::string(home) + "/debugger_logs";
}