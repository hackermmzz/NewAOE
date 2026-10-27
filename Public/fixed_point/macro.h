#pragma once

#include"Public/util/util.h"

#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__) || defined(__LP64__)
#define MMZZ_X64
#else
#define MMZZ_X86
#endif



