#ifndef __UTILITY_H__
#define __UTILITY_H__

unsigned char HashToIndex(char* pData, unsigned int dwLen);
unsigned char HashToIndex(short wData);
unsigned char HashToIndex(unsigned short wData);
unsigned char HashToIndex(int dwData);
unsigned char HashToIndex(unsigned int dwData);
unsigned char HashToIndex(long long qwData);
unsigned char HashToIndex(unsigned long long qwData);

void SetIndex(double qwIndex);
bool CheckIndex(unsigned char btIndex);

#endif
