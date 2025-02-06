#ifndef STRING_H
#define STRING_H

#include "platform.h"

typedef buffer string;

typedef struct struct_string_list
{
    string *Strings;
    u32 Count;
} string_list;

#endif
