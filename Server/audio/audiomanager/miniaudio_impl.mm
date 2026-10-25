// miniaudio's iOS/tvOS/watchOS Core Audio backend contains Objective-C syntax.
// Keep it in a dedicated Objective-C++ translation unit.
#if defined(__APPLE__)
#include <TargetConditionals.h>

#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE == 1
#define MINIAUDIO_IMPLEMENTATION
#include "Public/third-party/miniaudio/miniaudio.h"
#endif
#endif

