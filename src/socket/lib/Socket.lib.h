#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string>
#include <thread>
#include <vector>
#include <functional>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <sys/epoll.h>
#include <queue>
#include <mutex>
#include "./Util.lib.h"


using namespace std;

class SocketException : exception
{
public: 
    const char *what() const throw()
    {
        return this->msg.c_str();
    }
    SocketException(const string msg){
        this->msg = msg;
    }
private:
    string msg;
};

class SocketHandler{
public:
    virtual void handle(sockaddr_in addr, int fd, int clientId) = 0;
};

class SocketAsyncHandler{
public:
    virtual void handle(const sockaddr_in& addr, const int& fd, const string& msg, const int& clientId) = 0;
};

struct Task{
    string msg;
    int clientfd;
};

class Socket
{
public:
    Socket(string addr, int port);
    void serve(SocketHandler* handler);
    void communicate(SocketHandler* handler);
    void serveAsync(SocketAsyncHandler* handler);
    void setPoolsize(const int& poolsize);
    void setQueryInterval(const int& interval);
    void stopAsyncServe();
    void _connect();
    string _recv();
    void _send(const string& msg);
private:
    int port;
    string addr;
    sockaddr_in _sock_addr;
    int _sockfd;
    int _server_backlog = 10;
    int clientId = 0;
    unsigned char _recv_buff[1024];
    unsigned char _send_buff[1024];
    vector<thread> threads;
    unordered_map<int, sockaddr_in> fd2addr;
    unordered_map<int, int> fd2id;
    queue<Task> tasks;
    mutex lock;
    int poolSize = 10;
    thread* maintainer;
    int queryInterval = 100;

    static void handle(SocketHandler* handler, sockaddr_in clientAddr, int clientfd, int clientID);
    static void asyncHandle(Socket* _this, SocketAsyncHandler* handler);
};
