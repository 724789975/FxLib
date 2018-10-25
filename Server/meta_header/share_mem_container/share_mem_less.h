#ifndef __ShareMemLess_H__
#define __ShareMemLess_H__

namespace ShareMemory
{
	template<typename T>
	class Less
	{
	public:
		bool operator() (const T& refLeft, const T& refRight)
		{
			return refLeft < refRight;
		}
	};

	template<typename T, class KeyCompare = Less<T> >
	class Equal
	{
	public:
		bool operator() (const T& refLeft, const T& refRight)
		{
			return (!KeyCompare()(refLeft, refRight)) && (KeyCompare()(refRight, refLeft));
		}
	};
};
#endif // !__ShareMemLess_H__
