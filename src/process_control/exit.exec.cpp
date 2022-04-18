#include <unistd.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

int main(int argc, char const *argv[])
{
    pid_t pid;
    pid = fork();
    if (pid > 0){
        cout << "[Parent Process] Ready to show exit() result." << endl;
        printf("Hello, \n");
        printf("This is a test program. \n");
        sleep(1);
        exit(1);
    }else if(pid == 0){
        cout << "[Child Process] Ready to show exit() result." << endl;
        printf("Hello, \n");
        printf("This is a test program.");
        _exit(2);
    }
    return 0;
}
