#ifndef _LZ_H
#define _LZ_H


#define _CRT_SECURE_NO_WARNINGS

#include "main.h"
#include <list>

#define BITS_OVERFLOW 17U  //max 14 bits =  bytes to parent index

union outByte {
	unsigned long long num;
	unsigned int index[sizeof(long long) / sizeof(int)];
	unsigned char bytes[sizeof(long long)];
};

void encode(std::string);
void decode(std::string);

#endif // !_LZ_H
