#pragma once

#include "windows.h"
#include "boost\lexical_cast.hpp"

typedef long long _int;
typedef unsigned long long _uint;

#define inttostr(a) boost::lexical_cast<std::string>(a)

wchar_t *makewstr(const char *s);

