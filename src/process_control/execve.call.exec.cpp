#include <unistd.h>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
    char* const path = "/home/coder/project/evalexp/out/bin/exec/execve.becalled.bin";
    char* const program_argv[] = {path, "-test", "for a test", NULL};
    char* const program_env[] = {"LD_PRELOAD=./preload.so", NULL};
    cout << "[Call Program] My pid is " << getpid() << endl;
    cout << " ----- Now Execve -----" << endl;
    if(execve(path, program_argv, program_env) == -1){
        perror("Execve Error");
    }
    return 0;
}
