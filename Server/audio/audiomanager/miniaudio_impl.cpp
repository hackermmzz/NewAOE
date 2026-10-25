// Apple mobile targets require Objective-C++. Their implementation is emitted
// by miniaudio_impl.mm instead; every other target uses this C++ translation
// unit.
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if !defined(__APPLE__) || !defined(TARGET_OS_IPHONE) || TARGET_OS_IPHONE != 1
#define MINIAUDIO_IMPLEMENTATION
#endif

#include "Public/third-party/miniaudio/miniaudio.h"


