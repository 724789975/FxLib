#pragma once
class BBB
{
	public:
#ifdef _WIN32
	__declspec(dllexport) 
#endif	//!_WIN32
	BBB();
	int b;
};
