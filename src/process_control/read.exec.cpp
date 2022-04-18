#include <iostream>
#include <stdio.h>
#include <algorithm>

using namespace std;

void readFile(const char *file, const int start, const int end)
{
    FILE *src = fopen(file, "r");
    if (src != NULL)
    {
        int i = 1;
        char buffer[1024];
        char c;
        while (!feof(src))
        {
            fgets(buffer, sizeof(buffer), src);
            if (i >= start && i <= end)
            {
                printf("%s", buffer);
            }
            if (i > end)
            {
                break;
            }
            i++;
        }
        printf("\n");
        fclose(src);
    }
    else
    {
        cout << "File Not found." << endl;
        exit(-1);
    }
}

int main(int argc, char const *argv[], char const *envp[])
{
    if (argc == 4)
    {
        int start = atoi(argv[2]);
        int end = atoi(argv[3]);
        if (start == 0 || end == 0)
        {
            cout << "Sorry, unsupport arg." << endl;
            return -1;
        }
        readFile(argv[1], start, end);
    }
    else
    {
        cout << "Sorry, unsupport arg." << endl;
        return -1;
    }
    return 0;
}
