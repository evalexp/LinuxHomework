#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

using namespace std;

int main(int argc, char const *argv[])
{
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        cout << "[Child] process run ... " << endl;
        exit(100);
    }
    int status;
    pid_t ret;
    ret = wait(&status);
    if (ret < 0)
    {
        perror("Wait Error");
        exit(EXIT_FAILURE);
    }
    printf("Ret = %d, child pid = %d\n", ret, pid);
    if (WIFEXITED(status))
        printf("child exited normal exit status = %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("child exited abnormal signal number = %d\n", WTERMSIG(status));
    else if (WIFSTOPPED(status))
        printf("child stopped signal number = %d\n", WSTOPSIG(status));

    return 0;
}
