#include "utils.h"

String stringMask(const String &str, char mask)
{
    String masked;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        masked += mask;
    }
    return masked;
}