#ifndef __CALLBACK_DISPATCH_H__
#define __CALLBACK_DISPATCH_H__

#include <map>
#include "nulltype.h"

namespace CallBackDispatcher
{
	template <typename R, typename K, class Owner, typename P1 = NullType, typename P2 = NullType, typename P3 = NullType,
		typename P4 = NullType, typename P5 = NullType, typename P6 = NullType, typename P7 = NullType, typename P8 = NullType>
	class ClassCallBackDispatcher;
	template <typename R, typename K, class Owner, typename P1, typename P2>
	class ClassCallBackDispatcher<R, K, Owner, P1, P2>
	{
	public:
		typedef R(Owner::*CallBackFunction)(P1& p1, P2& p2);
		ClassCallBackDispatcher(Owner& refOwner) :m_refOwner(refOwner) {}
		inline bool RegistFunction(K k, CallBackFunction pCallBackFunction)
		{
			if (m_mapCallBackFuns.find(k) != m_mapCallBackFuns.end())
			{
				return false;
			}
			m_mapCallBackFuns[k] = pCallBackFunction;
			return true;
		}
		inline CallBackFunction GetFunction(K k)
		{
			if (m_mapCallBackFuns.find(k) == m_mapCallBackFuns.end())
			{
				return false;
			}
			return m_mapCallBackFuns[k];
		}
		inline R Dispatch(CallBackFunction pFun, P1& p1, P2& p2)
		{
			return (m_refOwner.*pFun)(p1, p2);
		}
	protected:
	private:
		std::map<K, CallBackFunction> m_mapCallBackFuns;
		Owner& m_refOwner;
	};

	template <typename R, typename K, class Owner, typename P1>
	class ClassCallBackDispatcher<R, K, Owner, P1>
	{
	public:
		typedef R(Owner::*CallBackFunction)(P1& p1);
		ClassCallBackDispatcher(Owner& refOwner) :m_refOwner(refOwner) {}
		inline bool RegistFunction(K k, CallBackFunction pCallBackFunction)
		{
			if (m_mapCallBackFuns.find(k) != m_mapCallBackFuns.end())
			{
				return false;
			}
			m_mapCallBackFuns[k] = pCallBackFunction;
			return true;
		}
		inline CallBackFunction GetFunction(K k)
		{
			if (m_mapCallBackFuns.find(k) == m_mapCallBackFuns.end())
			{
				return false;
			}
			return m_mapCallBackFuns[k];
		}
		inline R Dispatch(CallBackFunction pFun, P1& p1)
		{
			return (m_refOwner.*pFun)(p1);
		}
	protected:
	private:
		std::map<K, CallBackFunction> m_mapCallBackFuns;
		Owner& m_refOwner;
	};

	template <typename R, typename K, class Owner>
	class ClassCallBackDispatcher<R, K, Owner>
	{
	public:
		typedef R(Owner::*CallBackFunction)();
		ClassCallBackDispatcher(Owner& refOwner) :m_refOwner(refOwner) {}
		inline bool RegistFunction(K k, CallBackFunction pCallBackFunction)
		{
			if (m_mapCallBackFuns.find(k) != m_mapCallBackFuns.end())
			{
				return false;
			}
			m_mapCallBackFuns[k] = pCallBackFunction;
			return true;
		}
		inline CallBackFunction GetFunction(K k)
		{
			if (m_mapCallBackFuns.find(k) == m_mapCallBackFuns.end())
			{
				return false;
			}
			return m_mapCallBackFuns[k];
		}
		inline R Dispatch(CallBackFunction pFun)
		{
			return (m_refOwner.*pFun)();
		}
	protected:
	private:
		std::map<K, CallBackFunction> m_mapCallBackFuns;
		Owner& m_refOwner;
	};
}




#endif // !__CALLBACK_DISPATCH_H__
