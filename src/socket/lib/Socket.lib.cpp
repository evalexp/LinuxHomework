#include "./Socket.lib.h"

Socket::Socket(string addr, int port)
{
    this->addr = addr;
    this->port = port;

    // 初始化socket信息
    this->_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sockfd == -1)
    {
        throw SocketException("Socket initial failed");
    }
    this->_sock_addr.sin_family = AF_INET;
    this->_sock_addr.sin_port = htons(this->port);
    in_addr address;
    if (!inet_aton(this->addr.c_str(), &address))
    {
        throw SocketException("Bad address");
    }
    this->_sock_addr.sin_addr = address;
    memset(this->_sock_addr.sin_zero, 0, 8);
}

void Socket::serveAsync(SocketAsyncHandler *handler)
{
    // 绑定socket
    if (bind(this->_sockfd, (const sockaddr *)&this->_sock_addr, sizeof(sockaddr)) == -1)
    {
        throw SocketException("Bind address error, check your address and port.");
    }
    // 开始监听
    if (listen(this->_sockfd, this->_server_backlog) == -1)
    {
        throw SocketException("Listen error");
    }
    printf("Server start at %s:%d \n", this->addr.c_str(), this->port);
    // 线程池维护pool size个线程处理事务
    for (int i = 0; i < this->poolSize; i++)
    {
        this->threads.push_back(thread(Socket::asyncHandle, this, handler));
    }
    // 任务递交线程，使用Lambda表达式创建的线程
    this->maintainer = new thread([this]()
                                  {
    socklen_t addrLen;
    // 创建epoll 根节点
    int epfd = epoll_create(1);
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = this->_sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, this->_sockfd, &event);
    epoll_event all[2000];
    while (1)
    {
        pthread_testcancel();
        int ret = epoll_wait(epfd, all, sizeof(all) / sizeof(all[0]), -1);
        if (ret == -1)
        {
            printf("[Fatal Error] Epoll Wait Error\n");
            break;
        }
        else if (ret == 0)
        { // 无变化
            continue;
        }
        else
        { // 有变化了
            for (int i = 0; i < ret; i++)
            {
                int fd = all[i].data.fd;
                if (fd == this->_sockfd && all[i].events & EPOLLIN)
                {
                    // socket描述符的改变，服务端说明有新连接
                    sockaddr_in clientAddr;
                    addrLen = sizeof(sockaddr);
                    int clientfd = accept(this->_sockfd, (sockaddr *)&clientAddr, &addrLen);
                    if (clientfd == -1)
                    {
                        printf("[Error] Connection with %s failed\n", inet_ntoa(clientAddr.sin_addr));
                        continue;
                    }
                    else
                    {
                        printf("[Info] Connection with %s success\n", inet_ntoa(clientAddr.sin_addr));
                        this->clientId++;
                    }
                    // 添加fd到addr、clientId的映射并挂载fd到节点上
                    this->fd2addr[clientfd] = clientAddr;
                    this->fd2id[clientfd] = this->clientId;
                    printf("");
                    event.events = EPOLLIN;
                    event.data.fd = clientfd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &event);
                }
                else if (all[i].events & EPOLLIN)
                {
                    // fd != this->_sockfd不是socket描述符的变化，说明是子节点发生的改变，此时为客户端状态改变，及发送了信息过来
                    char buff[1024] = {0};
                    int recvLen = recv(fd, buff, 1024, 0);
                    if (recvLen < 0)
                    {
                        // 接收异常，清除信息
                        printf("[Error] Failed to recv from %s\n", inet_ntoa(this->fd2addr[fd].sin_addr));
                        close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &all[i]);
                        this->fd2addr.erase(fd);
                        this->fd2id.erase(fd);
                        continue;
                    }
                    else if (recvLen == 0)
                    {
                        // 客户端关闭连接，同样清除信息
                        printf("[Info] Client %s close the connection\n", inet_ntoa(this->fd2addr[fd].sin_addr));
                        close(fd);
                        this->fd2addr.erase(fd);
                        this->fd2id.erase(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &all[i]);
                    }
                    else
                    {
                        // 否则将接收到的信息转为任务，交由线程池处理
                        Task newtask;
                        newtask.msg = buff;
                        newtask.clientfd = fd;
                        {
                            // 临界区递交任务
                            lock_guard<mutex> _lg(this->lock);
                            this->tasks.push(newtask);
                        }
                    }
                }
            }
        }
    } });
}

void Socket::setQueryInterval(const int& interval){
    if(internal <=0 || interval >= 1000){
        throw SocketException("Interval illegal");
    }else{
        this->queryInterval = interval;
    }
}

void Socket::stopAsyncServe(){
    for(int i = 0; i < this->threads.size(); i++){
        printf("Stop thread %d\n", i);
        pthread_cancel(this->threads[i].native_handle());
    }
    pthread_cancel(this->maintainer->native_handle());
    close(this->_sockfd);
    for(int i = 0; i < this->threads.size();i++){
        printf("Join thread %d\n", i);
        this->threads[i].join();
    }
    this->maintainer->join();
}

void Socket::asyncHandle(Socket *_this, SocketAsyncHandler *handler)
{
    while (1)
    {
        pthread_testcancel();
        Task handleTask;
        {
            // 访问临界区资源，使用互斥锁 lock_guard会自动释放
            lock_guard<mutex> _lg(_this->lock);
            if (_this->tasks.empty())
            {
                // 为了避免空任务时占用过大的CPU资源，必须设置轮询间隔，单位为毫秒
                this_thread::sleep_for(chrono::microseconds(_this->queryInterval));
                continue;
            }
            else
            {
                // 取出一项任务
                handleTask = _this->tasks.front();
                _this->tasks.pop();
            }
        }
        // 退出临界区，使用handler处理
        handler->handle(_this->fd2addr[handleTask.clientfd], handleTask.clientfd, handleTask.msg, _this->fd2id[handleTask.clientfd]);
    }
}

void Socket::setPoolsize(const int &poolsize)
{
    if (poolsize <= 0 || poolsize > 100)
    {
        throw SocketException("Too many threads.");
    }
    else
    {
        this->poolSize = poolsize;
    }
}

void Socket::serve(SocketHandler *handler)
{
    // 绑定socket
    if (bind(this->_sockfd, (const sockaddr *)&this->_sock_addr, sizeof(sockaddr)) == -1)
    {
        throw SocketException("Bind address error, check your address and port.");
    }
    // 开始监听
    if (listen(this->_sockfd, this->_server_backlog) == -1)
    {
        throw SocketException("Listen error");
    }
    socklen_t addrLen;
    int recvLen;
    printf("Server start at %s:%d \n", this->addr.c_str(), this->port);
    // 循环处理接收到的请求
    while (1)
    {
        addrLen = sizeof(sockaddr);
        sockaddr_in clientAddr;
        int clientfd = accept(this->_sockfd, (sockaddr *)&clientAddr, &addrLen);
        if (clientfd != -1)
        {
            this->clientId++;
            // 使用新线程利用Handler进行消息处理
            this->threads.push_back(thread(Socket::handle, handler, clientAddr, clientfd, this->clientId));
        }
    }
}

void Socket::handle(SocketHandler *handler, sockaddr_in clientAddr, int clientfd, int clientID)
{
    handler->handle(clientAddr, clientfd, clientID);
}

void Socket::communicate(SocketHandler *handler)
{
    this->_connect();
    printf("Connect to server %s\n", this->addr.c_str());
    handler->handle(this->_sock_addr, this->_sockfd, 0);
}

void Socket::_connect(){
    if(connect(this->_sockfd, (const sockaddr*)&this->_sock_addr, sizeof(sockaddr)) == -1){
        throw SocketException("Connect error, check your ip or port");
    }
}

string Socket::_recv(){
    char buff[1024];
    int recvLen = recv(this->_sockfd, buff, 1024, 0);
    if(recvLen <= 0){
        throw SocketException("Connection close");
        close(this->_sockfd);
    }else{
        buff[recvLen] = '\0';
        return buff;
    }

}

void Socket::_send(const string& msg){
    if(send(this->_sockfd, msg.c_str(), msg.size(), 0) <= 0){
        throw SocketException("Connection lost");
    }
}