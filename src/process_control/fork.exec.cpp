#include <unistd.h>
#include <iostream>

using namespace std;

int main(void)
{
    pid_t pid;
    pid = fork();
    if (pid > 0)
    {
        cout << "I am parent, my pid = " << getpid() << endl;
    }
    else if (pid == 0)
    {
        cout << "I am child, my pid = " << getpid() << endl;
    }
    return 0;
}