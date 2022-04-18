#include <iostream>
#include <string>
#include "../lib/Socket.lib.h"
#include "../lib/Util.lib.h"
#include <algorithm>

// 设计五服务端

namespace act
{
    enum ActionType
    {
        LOGIN = 0,
        JOIN = 1,
        SEND = 3,
        SENDTO = 4,
        FILE_ASK = 5,
        FILE_REAL_SEND = 6,
        FILE_SEND_END = 7,
        FILE_ANSWER = 8,
        FILE_DATA = 9,
        None = -1
    };
}

using namespace std;

class MyHandler : SocketAsyncHandler
{
public:
    void handle(const sockaddr_in &addr, const int &fd, const string &msg, const int &clientId);
    MyHandler();

private:
    unordered_map<int, int> id2fd;
    unordered_map<int, string> id2nickname;
    unordered_map<int, vector<int>> channelid2userlist;
    unordered_map<int, int> id2channel;
    unordered_map<act::ActionType, function<void(MyHandler *_this, const string &arg, const int &fd, const int &id)>> action;
    unordered_map<int, bool> fileSendMode;
    unordered_map<int, int> id2filesendfd;

    act::ActionType resolveAction(const string &msg, string &arg);
    void registerAction(act::ActionType action, function<void(MyHandler *_this, const string &arg, const int &fd, const int &id)> func);
    bool response(const string &msg, const int &fd);
    bool getUid(unsigned int &iuid, string &suid, const string &arg, const int &fd);

    static void login(MyHandler *_this, const string &arg, const int &fd, const int &id);
    static void join(MyHandler *_this, const string &arg, const int &fd, const int &id);
    static void broadcast(MyHandler *_this, const string &arg, const int &fd, const int &id);
    static void sendto(MyHandler *_this, const string &arg, const int &fd, const int &id);
    static void fileAsk(MyHandler *_this, const string &arg, const int &fd, const int &id);
    static void fileAnswer(MyHandler *_this, const string &arg, const int &fd, const int &id);
    static void filedata(MyHandler *_this, const string &arg, const int &fd, const int &id);
};

bool MyHandler::getUid(unsigned int &iuid, string &suid, const string &arg, const int &fd)
{
    // 从参数表中获取接收方的uid
    for (int i = 0; i < arg.size() && arg[i] != ' '; i++)
        suid += arg[i];
    iuid = atoi(suid.c_str());
    stringstream buff;
    // 找不到用户
    if (this->id2fd.find(iuid) == this->id2fd.end())
    {
        buff << "/echo Server: user whose uid = " << iuid << " not found";
        this->response(buff.str(), fd);
        return false;
    }
    return true;
}

void MyHandler::login(MyHandler *_this, const string &arg, const int &fd, const int &id)
{
    if (arg.size() > 10)
    {
        _this->response("/echo Sorry, the nickname must be less than 10 characters long", fd);
    }
    else
    {
        _this->id2nickname[id] = arg;
        stringstream buffer;
        buffer << "/echo Server: Hello, " << arg << ", your uid is " << id;
        _this->response(buffer.str(), fd);
    }
}

void MyHandler::join(MyHandler *_this, const string &arg, const int &fd, const int &id)
{
    unsigned int channel = atoi(arg.c_str());
    vector<int> *v = &_this->channelid2userlist[_this->id2channel[id]];
    for (size_t i = 0; i < v->size(); i++)
    {
        if (v->at(i) == id)
        {
            v->erase(v->begin() + i);
            break;
        }
    }
    _this->channelid2userlist[channel].push_back(id);
    _this->id2channel[id] = channel;
    stringstream buffer;
    buffer << "/echo Server: Switch to channel " << channel;
    _this->response(buffer.str(), fd);
}

void MyHandler::broadcast(MyHandler *_this, const string &arg, const int &fd, const int &id)
{
    vector<int> *channelList = &_this->channelid2userlist[_this->id2channel[id]];
    int uid;
    int broadcast_fd;
    string nickname;
    for (int i = 0; i < channelList->size(); i++)
    {
        // 取广播组内的uid
        uid = channelList->at(i);
        if (uid != id)
        {
            broadcast_fd = _this->id2fd[uid];
            nickname = _this->id2nickname[uid];
            stringstream buff;
            printf("Broadcast to %d\n", uid);
            // 向广播组内用户分别发送消息
            buff << "/echo " << _this->id2nickname[id] << "(" << id << "): " << arg;
            if (!_this->response(buff.str(), broadcast_fd))
            {
                // 发送失败，说明socket异常，关闭socket并且清理该客户端的存储信息
                close(broadcast_fd);
                channelList->erase(channelList->begin() + i);
                _this->id2channel.erase(uid);
                _this->id2fd.erase(uid);
                _this->id2nickname.erase(uid);
            }
        }
    }
}

void MyHandler::sendto(MyHandler *_this, const string &arg, const int &fd, const int &id)
{
    // 从参数表中获取接收方的uid
    string suid = "";
    unsigned int iuid;
    stringstream buff;
    if (_this->getUid(iuid, suid, arg, fd))
    {
        // 找到接收方了，向接收方发送消息
        buff << "/echo [Private]" << _this->id2nickname[iuid] << "(" << id << "): " << Util::replace(arg, suid + " ", "");
        _this->response(buff.str(), _this->id2fd[iuid]);
    }
}

void MyHandler::fileAsk(MyHandler *_this, const string &arg, const int &fd, const int &id)
{
    // 从参数表中获取接收方的uid
    string suid = "";
    unsigned int iuid;
    if (_this->getUid(iuid, suid, arg, fd))
    {
        int target_fd = _this->id2fd[iuid];
        if (_this->id2filesendfd.end() != find_if(_this->id2filesendfd.begin(), _this->id2filesendfd.end(), [target_fd](const unordered_map<int, int>::value_type &mvt)
                                                  { return mvt.second == target_fd; }))
        {
            // 其它用户正在传输
            _this->response("/echo Server: Target user not valid now, try it later", fd);
            cout << "Target fd found " << target_fd << endl;
            return;
        }
        // 暂存接收方fd
        _this->id2filesendfd[id] = _this->id2fd[iuid];
        stringstream buff;
        buff << "/file " << _this->id2nickname[id] << "(" << id << ")" << Util::replace(arg, suid, "");
        cout << buff.str() << endl;
        _this->response(buff.str(), _this->id2fd[iuid]);
    }
    else
    {
        stringstream buff;
        buff << "/echo Server: Could not find user whose uid = " << iuid;
        _this->response(buff.str(), fd);
    }
}

void MyHandler::fileAnswer(MyHandler *_this, const string &arg, const int &fd, const int &id)
{
    // 从参数表中获取接收方的uid
    string suid = "";
    unsigned int iuid;
    if (_this->getUid(iuid, suid, arg, fd))
    {
        // 转发answer
        if (!Util::endWith(arg, "accept"))
        {
            cout << "reject" << endl;
            _this->id2filesendfd.erase(iuid);
        }
        _this->response("/answer " + arg, _this->id2fd[iuid]);
    }
}

void MyHandler::filedata(MyHandler *_this, const string &arg, const int &fd, const int &id)
{
    if (_this->fileSendMode[id])
    {
        _this->response(arg, _this->id2filesendfd[id]);
    }
    else
    {
        _this->response("/echo Server: raw data must be file data, please use /file to request first", fd);
    }
}

bool MyHandler::response(const string &msg, const int &fd)
{
    if (send(fd, msg.c_str(), msg.size(), 0) <= 0)
    {
        printf("[Error] Failed to response to sockfd : %d", fd);
        return false;
    }
    else
    {
        return true;
    }
}

MyHandler::MyHandler()
{
    this->registerAction(act::ActionType::LOGIN, MyHandler::login);
    this->registerAction(act::ActionType::JOIN, MyHandler::join);
    this->registerAction(act::ActionType::SEND, MyHandler::broadcast);
    this->registerAction(act::ActionType::SENDTO, MyHandler::sendto);
    this->registerAction(act::ActionType::FILE_ASK, MyHandler::fileAsk);
    this->registerAction(act::ActionType::FILE_REAL_SEND, [](MyHandler *_this, const string &arg, const int &fd, const int &id)
                         { if(_this->id2filesendfd.find(id) != _this->id2filesendfd.end()){
                            _this->fileSendMode[id] = true;
                            _this->response("/filesend", _this->id2filesendfd[id]);
                         }else{
                             _this->response("/echo Server: Sorry, please ask first", fd);
                         } });
    this->registerAction(act::ActionType::FILE_SEND_END, [](MyHandler *_this, const string &arg, const int &fd, const int &id)
                         { _this->fileSendMode.erase(id);
                         _this->response("/fileend", _this->id2filesendfd[id]);
                         _this->id2filesendfd.erase(id); });
    this->registerAction(act::ActionType::FILE_ANSWER, MyHandler::fileAnswer);
    this->registerAction(act::ActionType::FILE_DATA, MyHandler::filedata);
    this->registerAction(act::ActionType::None, [](MyHandler *_this, const string &arg, const int &fd, const int &id)
                         { _this->response("/echo Server: No command detected", fd); });
}

void MyHandler::registerAction(act::ActionType action, function<void(MyHandler *_this, const string &arg, const int &fd, const int &id)> func)
{
    if (this->action.find(action) == this->action.end())
    {
        this->action[action] = func;
    }
    else
    {
        throw "Action exist";
    }
}

void MyHandler::handle(const sockaddr_in &addr, const int &fd, const string &msg, const int &clientId)
{
    if (this->id2fd.find(clientId) == this->id2fd.end())
        this->id2fd[clientId] = fd;
    if (this->id2nickname.find(clientId) == this->id2nickname.end())
        this->id2nickname[clientId] = "Anonymous";
    if (this->id2channel.find(clientId) == this->id2channel.end())
    {
        this->id2channel[clientId] = 0;
        this->channelid2userlist[0].push_back(clientId);
    }
    string arg;
    act::ActionType act = this->resolveAction(msg, arg);
    this->action[act](this, arg, fd, clientId);
}

act::ActionType MyHandler::resolveAction(const string &msg, string &arg)
{
    if (Util::startWith(msg, "/login "))
    {
        arg = Util::replace(msg, "/login ", "");
        return act::ActionType::LOGIN;
    }
    else if (Util::startWith(msg, "/join "))
    {
        arg = Util::replace(msg, "/join ", "");
        return act::ActionType::JOIN;
    }
    else if (Util::startWith(msg, "/send "))
    {
        arg = Util::replace(msg, "/send ", "");
        return act::ActionType::SEND;
    }
    else if (Util::startWith(msg, "/sendto "))
    {
        arg = Util::replace(msg, "/sendto ", "");
        return act::ActionType::SENDTO;
    }
    else if (Util::startWith(msg, "/file "))
    {
        arg = Util::replace(msg, "/file ", "");
        return act::ActionType::FILE_ASK;
    }
    else if (Util::startWith(msg, "/filesend"))
    {
        arg = msg;
        return act::ActionType::FILE_REAL_SEND;
    }
    else if (Util::startWith(msg, "/answer "))
    {
        arg = Util::replace(msg, "/answer ", "");
        return act::ActionType::FILE_ANSWER;
    }
    else if (Util::startWith(msg, "/fileend"))
    {
        return act::ActionType::FILE_SEND_END;
    }
    else if (Util::startWith(msg, "/data "))
    {
        arg = msg;
        return act::ActionType::FILE_DATA;
    }
    else
    {
        arg = msg;
        return act::ActionType::None;
    }
}

int main(int argc, char const *argv[])
{
    try
    {
        Socket socket("0.0.0.0", 12345);
        MyHandler *handler = new MyHandler();
        socket.setPoolsize(10);
        socket.setQueryInterval(1);
        socket.serveAsync((SocketAsyncHandler *)handler);
        string s;
        while (1)
        {
            getline(cin, s);
            if (Util::startWith(s, "/exit"))
            {
                socket.stopAsyncServe();
                break;
            }
        }
    }
    catch (SocketException &e)
    {
        cout << "Socket Error : " << e.what() << endl;
    }
    return 0;
}
