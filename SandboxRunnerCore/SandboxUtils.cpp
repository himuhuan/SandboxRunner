#include "SandboxUtils.h"

#include <cstring>

int SplitString(char *str, const char *delimiter, char **result, int maxCount)
{
    int count = 0;
    result[count] = strtok(str, delimiter);
    while (result[count] != nullptr && count < maxCount)
    {
        result[++count] = strtok(nullptr, delimiter);
    }
    return count;
}