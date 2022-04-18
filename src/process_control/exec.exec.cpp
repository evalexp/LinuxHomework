#include <unistd.h>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
    cout << "Prepare to enter a new process" << endl;
    execl("/bin/ls", "ls", "-l", NULL);
    cout << "Can you see me?" << endl;
    return 0;
}
