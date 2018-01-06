#ifndef __UTILITY_H__
#define __UTILITY_H__

unsigned char HashToIndex(const char* pData, unsigned int dwLen);
unsigned char HashToIndex(short wData);
unsigned char HashToIndex(unsigned short wData);
unsigned char HashToIndex(int dwData);
unsigned char HashToIndex(unsigned int dwData);
unsigned char HashToIndex(long long qwData);
unsigned char HashToIndex(unsigned long long qwData);


#endif	//!__UTILITY_H__
