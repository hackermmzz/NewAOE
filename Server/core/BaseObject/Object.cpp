#include "Object.h"

U64 Object::globalSNIndex=1;	//给全局对象分配SN

Object::Object()
{
    SN=globalSNIndex++;
}