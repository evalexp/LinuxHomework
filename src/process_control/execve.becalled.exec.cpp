#include <unistd.h>
#include <iostream>
using namespace std;
int main(int argc, char const *argv[], char const *envp[])
{
    cout << "[Becalled Program] My pid = " << getpid() << endl;
    cout << "Arg count = " << argc << endl;
    for (size_t i = 0; i < argc; i++)
    {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    for (size_t i = 0; envp[i]; i++)
    {
        printf("envp[%d] = %s\n", i, envp[i]);
    }
    return 0;
}
