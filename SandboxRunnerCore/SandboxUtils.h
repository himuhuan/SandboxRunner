#ifndef SANDBOX_UTILS_H
#define SANDBOX_UTILS_H
#include <cstdio>

/**
 * @brief Split a string by a delimiter
 * @return the number of parts
 * @remarks This function is not thread-safe
 */
[[nodiscard]] int SplitString(char *str, const char *delimiter, char **result, int maxCount);

#endif //! SANDBOX_UTILS_H
