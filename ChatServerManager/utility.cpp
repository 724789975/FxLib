#include "utility.h"

#include <set>

unsigned char HashToIndex(char* pData, unsigned int dwLen)
{
	unsigned char btHash = 0;
	for (unsigned int i = 0; i < dwLen; i++)
	{
		btHash += pData[i];
	}
	return btHash;
}

unsigned char HashToIndex(short wData)
{
	return HashToIndex((char*)&wData, sizeof(wData));
}

unsigned char HashToIndex(unsigned short wData)
{
	return HashToIndex((char*)&wData, sizeof(wData));
}

unsigned char HashToIndex(int dwData)
{
	return HashToIndex((char*)&dwData, sizeof(dwData));
}

unsigned char HashToIndex(unsigned int dwData)
{
	return HashToIndex((char*)&dwData, sizeof(dwData));
}

unsigned char HashToIndex(long long qwData)
{
	return HashToIndex((char*)&qwData, sizeof(qwData));
}

unsigned char HashToIndex(unsigned long long qwData)
{
	return HashToIndex((char*)&qwData, sizeof(qwData));
}

//-------------------------------------------


