#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        cout << "Which program you want to run?" << endl;
        exit(0);
    }
    // call other program
    // reshape argv, delete self program name
    char **p_argv = argv + 1;
    pid_t pid;
    // create subprocess to execve
    pid = fork();
    // parent process, print notice
    if (pid > 0)
    {
        cout << "Ready to parse var, call subprocess..." << endl;
        cout << "Parse start =======>" << endl;
        int status;
        // wait for subprocess return
        wait(&status);
        if (status >= 0)
        {
            // normal return
            cout << "Parse End <=======" << endl;
        }
        // error, return the error code to shell
        exit(status);
    }
    else if (pid == 0)
    {
        char *path = argv[1];
        // inject .so file
        char *p_env[] = {"LD_PRELOAD=/home/coder/project/evalexp/hook.so", NULL};
        if (execve(path, p_argv, p_env) == -1)
        {
            perror("Execve error");
        }
    }
    return 0;
}
