#include "../lib/Socket.lib.h"
#include "../lib/Util.lib.h"

// 设计四服务端

using namespace std;

class MyHandler : SocketHandler
{
public:
    void handle(sockaddr_in clientAddr, int clientfd, int clientId);

private:
    sockaddr_in _client_addr;
    int _client_fd;
    int _client_id;
    string username = "";
    string filename = "";
    string filedata;
    void parentProcess();
    void childProcess();
    bool response(const string &msg);
};

void MyHandler::handle(sockaddr_in clientAddr, int clientfd, int clientId)
{
    this->_client_addr = clientAddr;
    this->_client_fd = clientfd;
    this->_client_id = clientId;
    pid_t pid = fork();
    if (pid > 0)
        this->parentProcess();
    else if (pid == 0)
        this->childProcess();
}

void MyHandler::parentProcess()
{
    // for sending the message
    if (send(this->_client_fd, "Hello, please login use command like /login Peter", 50, 0) <= 0)
    {
        printf("[Error] Failed to send welcome text to client");
    }
    while (1)
    {
        string input;
        getline(cin, input);
        printf("[Info] Send message to client %d : %s\n", this->_client_id, input.c_str());
        // 此处禁止使用this->response函数 父进程拿不到子进程的this->username
        if (send(this->_client_fd, input.c_str(), input.size(), 0) <= 0)
        {
            close(this->_client_fd);
            exit(-1);
        }
    }
}

void MyHandler::childProcess()
{
    printf("[Info] Connect from client %d, addr %s \n", this->_client_id, inet_ntoa(this->_client_addr.sin_addr));
    int recvLen;
    char buff[1024];
    while (1)
    {
        // recv message
        recvLen = recv(this->_client_fd, buff, 1024, 0);
        if (recvLen <= 0)
        {
            // error, exit
            printf("[Info] Close connection from client %d, addr %s \n", this->_client_fd, inet_ntoa(this->_client_addr.sin_addr));
            close(this->_client_fd);
            exit(-1);
        }
        else
        {
            buff[recvLen] = '\0';
            string mes = buff;
            // check if is login
            if (this->username == "" && !Util::startWith(mes, "/login "))
            {
                this->response("Hello, please login use command like /login Peter");
                continue;
            }
            // login action
            if (Util::startWith(mes, "/login "))
            {
                this->username = Util::replace(mes, "/login ", "");
                // keep the username
                string welcome = "Hello, " + this->username;
                if (!this->response(welcome))
                {
                    printf("[Error] Failed to response for login action as user : %s", this->username.c_str());
                }
            }
            // client want to send file
            else if (Util::startWith(mes, "/file "))
            {
                printf("%s want to send you a file, do you want to recv it ? \n[Type /accept to recv it, any other characters to reject]\n", this->username.c_str());
                // parse the filename
                string path = Util::replace(mes, "/file ", "");
                this->filename = path.substr(path.find_last_of('/') + 1, path.size() - path.find_last_of('/'));
            }
            // file recv action
            else if (Util::startWith(mes, "[File]"))
            {
                // to get the origin data, replace the flag to null
                this->filedata = Util::replace(mes, "[File]", "");
                string path = "/home/coder/project/evalexp/recv/" + this->filename;
                while(!Util::endWith(this->filedata, "[FileEOF]")){
                    recvLen = recv(this->_client_fd, buff, 1024, 0);
                    string data = buff;
                    this->filedata += data;
                }
                this->filedata = Util::replace(this->filedata, "[FileEOF]", "");
                ofstream out(path);
                if(out.good()){
                    out.write(this->filedata.c_str(), this->filedata.size());
                    printf("[Info] File save to %s\n", path.c_str());
                }
                out.close();
            }
            else
            {
                // just print
                printf("%s : %s\n", this->username.c_str(), buff);
            }
        }
    }
}

bool MyHandler::response(const string &msg)
{
    // send welcome
    if (Util::startWith(msg, "Hello, "))
    {
        return send(this->_client_fd, msg.c_str(), msg.size(), 0) > 0;
    }
    else
    {
        // require login
        if (this->username == "")
        {
            return send(this->_client_fd, "Sorry, login first.", 20, 0) > 0;
        }
        else
        {
            return send(this->_client_fd, msg.c_str(), msg.size(), 0) > 0;
        }
    }
}

int main(int argc, char const *argv[])
{
    // serve
    try
    {
        Socket socket("0.0.0.0", 12345);
        MyHandler *handler = new MyHandler();
        socket.serve((SocketHandler *)handler);
    }
    catch (SocketException &e)
    {
        cout << "Socket Error : " << e.what() << endl;
    }
    return 0;
}
