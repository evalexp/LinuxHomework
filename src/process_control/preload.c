#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief 
 * 
 * @param strRes 
 * @param from 
 * @param to 
 * @return int 
 */
int StrReplace(char strRes[], char from[], char to[])
{
    int flag = 0;
    char *Ptr = NULL;
    char *middle_flag = strstr(strRes, from);
    if (middle_flag == NULL)
    {
        return flag;
    }

    int len = strlen(middle_flag);
    Ptr = (char *)malloc(len * sizeof(char));
    if (NULL == Ptr)
    {
        return flag;
    }
    strcpy(Ptr, middle_flag + (strlen(from)));
    if (middle_flag != NULL)
    {
        *middle_flag = '\0';
        strcat(strRes, to);
        strcat(strRes, Ptr);
        free(Ptr);
        flag = 1;
    }
    return flag;
}

char *fgets(char *s, int n, FILE *f)
{
    typeof(fgets) *func;
    func = dlsym(RTLD_NEXT, "fgets");
    va_list valist;
    char *result = (*func)(s, n, f);
    char cwd[1024];
    struct passwd *pwd = getpwuid(getuid());
    getcwd(cwd, sizeof(cwd));
    while (strstr(result, "$user") != NULL)
    {
        StrReplace(result, "$user", pwd->pw_name);
    }
    while (strstr(result, "$home") != NULL)
    {
        StrReplace(result, "$home", pwd->pw_dir);
    }
    while (strstr(result, "$pwd") != NULL)
    {
        StrReplace(result, "$pwd", cwd);
    }
    return result;
}
