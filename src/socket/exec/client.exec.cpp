#include "../lib/Socket.lib.h"
#include "../lib/Util.lib.h"

// 设计四客户端

class MyHandler : SocketHandler
{
public:
    void handle(sockaddr_in serverAddr, int serverfd, int _);

private:
    sockaddr_in _server_addr;
    int _server_fd;
    void parentProcess();
    void childProcess();
};

void MyHandler::handle(sockaddr_in serverAddr, int serverfd, int _)
{
    this->_server_addr = serverAddr;
    this->_server_fd = serverfd;
    pid_t pid = fork();
    if (pid > 0)
        this->parentProcess();
    else if (pid == 0)
        this->childProcess();
}

void MyHandler::parentProcess()
{
    int recvLen;
    char buff[1024];
    while (1)
    {
        recvLen = recv(this->_server_fd, buff, 1024, 0);
        if (recvLen <= 0)
        {
            printf("Connection close.");
            close(this->_server_fd);
            exit(-1);
        }
        else
        {
            buff[recvLen] = '\0';
            printf("Server : %s\n", buff);
        }
    }
}

void MyHandler::childProcess()
{
    while (1)
    {
        string input;
        getline(cin, input);
        input = Util::trim(input);
        // detect file command
        if (Util::startWith(input, "/file1 "))
        {
            string path = Util::replace(input, "/file ", "");
            string content;
            try
            {
                // load file
                string content = Util::getFileContent(path);
                Util::transformToSendStruct(content);
            }
            catch (const char *msg)
            {
                printf("[Error] %s\n", msg);
                continue;
            }
            // send command to server
            send(this->_server_fd, input.c_str(), input.length(), 0);
            char buff[1024];
            int recvLen = recv(this->_server_fd, buff, 1024, 0);
            if (recvLen > 0)
            {
                buff[recvLen] = '\0';
                string response = buff;
                // server accept to recv file
                if (Util::startWith(response, "/accept"))
                {
                    string path = Util::replace(input, "/file ", "");
                    try
                    {
                        string content = Util::getFileContent(path);
                        Util::transformToSendStruct(content);
                        // send it
                        if (send(this->_server_fd, content.c_str(), content.size(), 0) <= 0)
                        {
                            printf("[Error] Failed to send to server\n");
                        }
                        else
                        {
                            printf("[Info] Send file success\n");
                        }
                    }
                    catch (const char *msg)
                    {
                        printf("[Error] %s\n", msg);
                    }
                }
                else
                {
                    printf("Server reject to recv file\n");
                }
            }
            else
            {
                printf("[Error] Could not recv response\n");
            }
        }
        // other, just send
        else if (send(this->_server_fd, input.c_str(), input.length(), 0) <= 0)
        {
            close(this->_server_fd);
            exit(-1);
        }
    }
}

int main(int argc, char **argv)
{
    try
    {
        Socket socket("127.0.0.1", 12345);
        MyHandler *handler = new MyHandler();
        socket.communicate((SocketHandler *)handler);
    }
    catch (SocketException &e)
    {
        cout << "Socket Error : " << e.what() << endl;
    };
    return 0;
}