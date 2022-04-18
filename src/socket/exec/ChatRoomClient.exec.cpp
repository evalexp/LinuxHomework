#include "../lib/UI.lib.h"
#include "../lib/Util.lib.h"
#include "../lib/Socket.lib.h"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <thread>

// 设计五客户端

using namespace std;

UI ui;
Socket *_socket;
thread *han = NULL;
string filepath;
string filedata;
string _filename;

void error(const string &msg)
{
    ui.showMessage("Error: " + msg);
    if (han != NULL)
    {
        pthread_cancel(han->native_handle());
        han->join();
    }
    han = NULL;
}

bool checkConnect()
{
    if (_socket == NULL)
    {
        ui.showMessage("Error: Connect to a server firse");
        return false;
    }
    return true;
}

void sec_send(const string &msg)
{
    if (!checkConnect())
        return;
    try
    {
        _socket->_send(msg);
    }
    catch (SocketException &e)
    {
        error(e.what());
    }
}

void echo(const string &msg)
{
    ui.showMessage(msg);
}

void handler()
{
    while (1)
    {
        pthread_testcancel();
        if (_socket == NULL)
        {
            break;
        }
        string msg = _socket->_recv();
        // 回显命令
        if (Util::startWith(msg, "/echo "))
        {
            echo(Util::replace(msg, "/echo ", ""));
        }
        // 文件发送请求
        else if (Util::startWith(msg, "/file "))
        {
            string arg = Util::replace(msg, "/file ", "");
            string filename = "";
            for (int i = arg.size() - 1; i >= 0 & arg[i] != ' '; i--)
                filename = arg[i] + filename;
            string user = Util::replace(arg, " " + filename, "");
            string uid = "";
            bool record = false;
            for (int i = 0; i < user.size() && user[i] != ')'; i++)
            {
                if (record)
                    uid += user[i];
                if (user[i] == '(')
                    record = true;
            }
            // 记录文件名
            _filename = filename;
            echo("Notice: User " + user + " want to send you file " + filename + ", type \"/answer " + uid + " accept\" to aceept it");
            echo("Notice: If you want to reject it, just type \"/answer " + uid + " reject\"");
        }
        else if (Util::startWith(msg, "/answer "))
        {
            if (Util::endWith(msg, "accept"))
            {
                // 发送文件 "/data "占了六个字符空间，为了安全，每次只读1000个字符
                ifstream in(filepath);
                char buff[1024];
                if (in.good())
                {
                    sec_send("/filesend");
                    // 每次发送间隔1秒，避免多条命令结合
                    this_thread::sleep_for(chrono::seconds(1));
                    while (!in.eof())
                    {
                        in.read(buff, 1000);
                        stringstream builder;
                        builder << "/data " << buff;
                        this_thread::sleep_for(chrono::seconds(1));
                        sec_send(builder.str());
                    }
                    this_thread::sleep_for(chrono::seconds(1));
                    sec_send("/fileend");
                    filepath = "";
                    ui.showMessage("Notice: Your file sent completely");
                }
            }else{
                ui.showMessage("Notice: Your file send request is rejected");
            }
        }else if(Util::startWith(msg, "/filesend")){
            // 文件接收
            filedata = "";
            ui.showMessage("Notice: File recving...");
        }
        else if(Util::startWith(msg, "/data ")){
            // 保持文件内容
            filedata += Util::replace(msg, "/data ", "");
        }
        else if(Util::startWith(msg, "/fileend")){
            // 接收完毕
            ofstream out("/home/coder/project/evalexp/recv/" + _filename);
            if(out.good()){
                // 写入文件系统
                out.write(filedata.c_str(), filedata.size());
                ui.showMessage("Notice: File save to /home/coder/project/evalexp/recv/" + _filename);
            }
            _filename = "";
            filedata = "";
        }
    }
}

void connect(const string &arg)
{
    string ip = "";
    for (int i = 0; i < arg.size() && arg[i] != ' '; i++)
        ip += arg[i];
    string port = Util::replace(arg, ip + " ", "");
    unsigned int iport = atoi(port.c_str());
    if (iport == 0)
    {
        ui.showMessage("Error: port could not be less than zero");
        return;
    }
    try
    {
        _socket = new Socket(ip, iport);
        _socket->_connect();
        ui.showMessage("Info: Connected to server");
        han = new thread(handler);
    }
    catch (const SocketException &e)
    {
        error(e.what());
    }
}

bool checkFileExist(const string &path, string &filename)
{
    ifstream in(path);
    if (in.good())
    {
        filename = "";
        for (int i = path.size() - 1; i >= 0 && path[i] != '/'; i--)
        {
            filename = path[i] + filename;
        }
        return true;
    }
    return false;
}

void sendFile(const string &arg)
{
    string suid = "";
    for (int i = 0; i < arg.size() && arg[i] != ' '; i++)
        suid += arg[i];
    string path = Util::replace(arg, suid + " ", "");
    string filename;
    if (checkFileExist(path, filename))
    {
        filepath = path;
        sec_send("/file " + suid + " " + filename);
    }
    else
    {
        ui.showMessage("Error: file not exists");
    }
}

int main(int argc, char const *argv[])
{
    // loop
    while (1)
    {
        string input = ui.getInput();
        string arg;
        switch (Util::resolveOperation(input, arg))
        {
        case Type::CONNECT:
            connect(arg);
            break;
        case Type::LOGIN:
            sec_send("/login " + arg);
            break;
        case Type::SEND:
            sec_send("/send " + arg);
            ui.showMessage("SELF: " + arg);
            break;
        case Type::SENDTO:
            sec_send("/sendto " + arg);
            ui.showMessage("[Private]Send to" + arg);
            break;
        case Type::JOIN:
            sec_send("/join " + arg);
            break;
        case Type::SENDFILE:
            sendFile(arg);
            ui.showMessage("[Private]Your file send request is sent");
            break;
        case Type::ANSWER:
            sec_send(arg);
            ui.showMessage("Notice: Your answer is sent");
            break;
        case Type::HELP:
            ui.help();
            break;
        case Type::EXIT:
            pthread_cancel(han->native_handle());
            han->join();
            exit(0);
        }
    }

    return 0;
}
