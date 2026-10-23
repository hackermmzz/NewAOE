#pragma once

#include "Public/type/type.h"
#include <string>
#include <cassert>
#include <functional>
#include <cstdint>

/************************************强制内联宏******************************** */
#if defined(_MSC_VER)
#    define FORCE_INLINE __forceinline
#    define NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#    define FORCE_INLINE inline __attribute__((always_inline))
#    define NO_INLINE __attribute__((noinline))
#else
// 未知编译器，退化为普通inline，不保证强制内联
#    define FORCE_INLINE inline
#    define NO_INLINE
#endif


/***********************************断言宏********************************** */
#define ENABLE_ASSERT 1
#if ENABLE_ASSERT
#define Assert(status,msg) do{assert( (!(status)) && msg );}while(false)
#else 
#define Assert(status,msg) {}
#endif


/**************************************工具函数******************************** */
U64 GetTimeMS();


std::string UUID();

bool wait_for(U32 seconds,const std::function<bool()>&cond);




uint64_t GetCurrentPid();