#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

enum Type{
    LOGIN = 0,
    CONNECT = 1,
    JOIN = 2,
    QUIT = 3,
    SEND = 4,
    SENDTO = 5,
    SENDFILE = 6,
    EXIT = 7,
    HELP = 8,
    ECHO = 9,
    ANSWER = 10
};

class Util
{
public:
    static bool startWith(const string &checkStr, const string &startStr);
    static bool endWith(const string& checkStr, const string &endStr);
    static string replaceAll(const string& src, const string& oldstr, const string& newstr);
    static string replace(const string& src, const string& oldstr, const string& newstr);
    static Type resolveOperation(const string& input, string& arg);
    static string trim(const string& src);
    static string getFileContent(const string& path);
    static void transformToSendStruct(string& content);
private:
};
