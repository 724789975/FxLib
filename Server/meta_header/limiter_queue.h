/**
* \file	limiter_queue.h
* \author  	邓晓波 724789975@qq.com
* \date 	2018年8月02日 23时04分26秒 CST
* \brief 	一个限流队列
*
* 主要是用于队列限流 防止队列中需要执行的回调太多 造成阻塞
* 原理是每帧执行固定数量的事件 或者 根据上一帧的执行时间 调整本帧的事件执行数量
*
*/
#ifndef __LimiterQueue_H__
#define __LimiterQueue_H__

#include <map>

class CEventBase
{
public:
	CEventBase():m_ppCallBack(NULL){}
	virtual ~CEventBase()
	{
		if (m_ppCallBack)
		{
			*m_ppCallBack = NULL;
		}
	}
	virtual bool Tick(double qwTick) = 0;

	void SetPP(CEventBase** ppCallBack)
	{
		m_ppCallBack = ppCallBack;
	}

	//这个是记录事件在队列中的位置 如果销毁掉的话 直接把队列中的指针赋空就可以了
	CEventBase** m_ppCallBack;
};

template<typename T>
class CEventCallBack : public CEventBase
{
	typedef bool (T::*TICKFUNC)(double);
public:
	CEventCallBack(){}
	CEventCallBack(T* obj, TICKFUNC fun) :m_pObj(obj), m_pFun(fun) { }
	void Init(T* obj, TICKFUNC fun) { m_pObj = obj; m_pFun = fun; }
	virtual ~CEventCallBack() { };
	bool Tick(double qwTick) { return (m_pObj->*m_pFun)(qwTick); };
private:
	T*          m_pObj;
	TICKFUNC    m_pFun;
};


// NUM 最小值为1
template<class T, unsigned int NUM>
class CEventCaller : public CEventCaller<T, NUM - 1>
{
public:
	typedef bool (T::*TICKFUNC)(double);
public:
	~CEventCaller() { }

	CEventCallBack<T>& MakeEvent(T* p, TICKFUNC fun)
	{
		m_oCallBack.Init(p, fun);
		return m_oCallBack;
	}
	CEventCallBack<T>& GetEvent()
	{
		return m_oCallBack;
	}

protected:
	CEventCallBack<T> m_oCallBack;
};

template<typename T>
class CEventCaller<T, 1>
{
public:
	typedef bool (T::*TICKFUNC)(double);
public:
	~CEventCaller() { }

	CEventCallBack<T>& MakeEvent(T* p, TICKFUNC fun)
	{
		m_oCallBack.Init(p, fun);
		return m_oCallBack;
	}
	CEventCallBack<T>& GetEvent()
	{
		return m_oCallBack;
	}

protected:
	CEventCallBack<T> m_oCallBack;
};

////一个限流队列 <执行时的判断条件(也是map的key), key的排列规则, 最大运行时间(毫秒)> 这种需要排列规则的 暂时先不实现了
//template<typename C, typename CO, int dwMaxRunTime>
//class TLimiterQueue
//{
//public:
//	TLimiterQueue()
//	{
//	}
//
//	~TLimiterQueue()
//	{
//	}
//
//	CCallBackBase** PushEvent(CCallBackBase* pCallBack, C c)
//	{
//		std::multimap<C, CCallBackBase*, CO>::iterator it = m_mmapEvents.insert(c, pCallBack);
//		pCallBack->SetPP(&(it->second));
//		return &(it->second);
//	}
//
//private:
//	std::multimap<C, CCallBackBase*, CO> m_mmapEvents;
//};

//不适用于多线程
template<typename C, int dwMaxNum = 0>
class TLimiterQueue
{
public:
	TLimiterQueue()
	{
	}

	~TLimiterQueue()
	{
	}

	virtual bool CheckEvent(C c) = 0;

	CEventBase** PushEvent(C c, CEventBase* pCallBack)
	{
		std::multimap<C, CEventBase*, CO>::iterator it = m_mmapEvents.insert(c, pCallBack);
		pCallBack->SetPP(&(it->second));
		return &(it->second);
	}
	
	void Proc(double qwTick)
	{
		for (int i = 0; i < dwMaxNum; i++)
		{
			if (m_mmapEvents.size() == 0)
			{
				return;
			}

			if (!CheckEvent(m_mmapEvents.begin()->first))
			{
				return;
			}

			CEventBase* pCB = m_mmapEvents.begin()->second;
			if (pCB)
			{
				m_mmapEvents.erase(m_mmapEvents.begin());
				pCB->Tick(qwTick);
			}
		}
	}

private:
	std::multimap<C, CEventBase*> m_mmapEvents;
};

//不适用于多线程
template<typename C>
class TLimiterQueue<C, 0>
{
public:
	TLimiterQueue()
	{
	}

	~TLimiterQueue()
	{
	}

	virtual bool CheckEvent(C c) = 0;

	CEventBase** PushEvent(C c, CEventBase* pCallBack)
	{
		std::multimap<C, CEventBase*>::iterator it = m_mmapEvents.insert(std::make_pair(c, pCallBack));
		pCallBack->SetPP(&(it->second));
		return &(it->second);
	}

	void Proc(int dwMaxNum, double qwTick)
	{
		for (int i = 0; i < dwMaxNum; i++)
		{
			if (m_mmapEvents.size() == 0)
			{
				return;
			}

			if (!CheckEvent(m_mmapEvents.begin()->first))
			{
				return;
			}

			CEventBase* pCB = m_mmapEvents.begin()->second;
			if (pCB)
			{
				m_mmapEvents.erase(m_mmapEvents.begin());
				pCB->Tick(qwTick);
			}
		}
	}

private:
	std::multimap<C, CEventBase*> m_mmapEvents;
};

#endif // !__LimiterQueue_H__
