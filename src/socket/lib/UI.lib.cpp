#include <curses.h>
#include <panel.h>
#include <iostream>
#include "./UI.lib.h"

/**
 * @brief init ui
 */
UI::UI()
{
    initscr();
    this->messageBox = newwin(20, 100, 0, 0);
    this->inputBox = newwin(5, 100, 20, 0);
    cbreak();
    box(messageBox, 0, 0);
    box(inputBox, 0, 0);
    this->inputBoxMessage = "Welcome to FAST CHAT. Type /help to list help.";
    this->refreshMessageBox();
    this->refreshInputBox();
}

/**
 * @brief Destroy UI
 */
UI::~UI()
{
    endwin();
}

/**
 * @brief print a message on the screen
 * 
 * @param msg the message to show
 */
void UI::showMessage(const string &msg)
{
    this->messages.push_back(msg);
    while(this->messages.size() > 100) {
        this->messages.erase(this->messages.begin());
    }
    this->refreshMessageBox();
    this->refreshInputBox();
}

/**
 * @brief refresh message box
 * 
 */
void UI::refreshMessageBox()
{
    wclear(this->messageBox);
    box(this->messageBox, 0, 0);
    int i = 0;
    if (this->messages.size() > 18)
    {
        i = this->messages.size() - 18;
    }
    int index = 1;
    for (; i < this->messages.size(); i++)
    {
        wmove(this->messageBox, index++, 1);
        waddstr(this->messageBox, messages[i].c_str());
    }
    wrefresh(this->messageBox);
}

/**
 * @brief refresh input box
 * 
 */
void UI::refreshInputBox()
{
    wclear(this->inputBox);
    box(this->inputBox, 0, 0);
    wmove(this->inputBox, 1, 1);
    waddstr(this->inputBox, this->inputBoxMessage.c_str());
    wmove(this->inputBox, 3, 1);
    waddstr(this->inputBox, "> ");
    wmove(this->inputBox, 3, 3);
    waddstr(this->inputBox, buff);
    wrefresh(this->inputBox);
}

/**
 * @brief override input method, for interrupting
 * 
 * @return string the input
 */
string UI::getInput()
{
    char c;
    int index = 0;
    while(true){
        c = wgetch(this->inputBox);
        if(c == 8 || c == 127){
            if(index != 0 ){
                index--;
                this->buff[index] = '\0';
            }
        }else if(c == '\n'){
            break;
        }else{
            this->buff[index++] = c;
            this->buff[index] = '\0';
        }
        this->refreshInputBox();
    }
    for(int i = 0 ; i < index + 1; i++){
        this->inputResult[i] = this->buff[i];
    }
    this->buff[0] = '\0';
    this->refreshInputBox();
    return this->inputResult;
}

/**
 * @brief show a notice on the top of input box
 * 
 * @param msg 
 */
void UI::inputShow(const string &msg)
{
    this->inputBoxMessage = msg;
    this->refreshInputBox();
}

/**
 * @brief print help message on the messagebox
 * 
 */
void UI::help()
{
    this->showMessage("------------------------------------------------------");
    this->showMessage("Help: ");
    this->showMessage("use /connect [ip] [port] to connect a server");
    this->showMessage("use /login [uid] to login as user [uid]");
    this->showMessage("use /join [channel] to join a channel");
    this->showMessage("use /send to send a message to channel");
    this->showMessage("uee /sendto [uid] [msg] to send a message to the user");
    this->showMessage("use /file [uid] [filename] to send a file to the user");
    this->showMessage("use /echo to echo a sentence");
    this->showMessage("use /exit to exit this program");
    this->showMessage("------------------------------------------------------");
}

string UI::getInputBoxMessage(){
    return this->inputBoxMessage;
}