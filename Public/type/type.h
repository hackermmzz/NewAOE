#pragma once
#include <stdint.h>

/*
定义程序使用的基础类型，不准使用除本文件定义外的任何其他基础类型
比如int,只能使用I32，而不是int
这是为了统一类型以及参数优化 
对跨平台也有绝对的优势
*/


//必须确保float类型为4字节,double为8字节
static_assert(sizeof(float) == 4, "float must be 4 bytes");
static_assert(sizeof(double) == 8, "double must be 8 bytes");


//带符号
using I8=int8_t;
using I16=int16_t;
using I32=int32_t;
using I64=int64_t;


//不带符号
using U8=uint8_t;
using U16=uint16_t;
using U32=uint32_t;
using U64=uint64_t;


//其他类型
using Byte=unsigned char;
using Float32=float;
using Float64=double;
