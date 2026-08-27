#ifndef LOCATION_PARSE_INT_PARAMETER_H
#define LOCATION_PARSE_INT_PARAMETER_H

#include <charconv>
#include <cstring>
#include <system_error>

inline bool ParseIntParameter(const char *buffer, int &out)
{
    if (buffer == nullptr || *buffer == '\0') {
        return false;
    }
    int value = 0;
    const char *last = buffer + std::strlen(buffer);
    auto result = std::from_chars(buffer, last, value);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

#endif
