#pragma once
#include <string>
#include <vector>
#include <curses.h>

using namespace std;

class UI{
public:
    UI();
    ~UI();
    void showMessage(const string& msg);
    string getInput();
    void inputShow(const string& msg);
    void help();
    string getInputBoxMessage();
private:
    vector<string> messages;
    string inputBoxMessage;
    char buff[1024] = {0};
    char inputResult[1024] = {0};
    WINDOW* messageBox;
    WINDOW* inputBox;
    void refreshMessageBox();
    void refreshInputBox();
};