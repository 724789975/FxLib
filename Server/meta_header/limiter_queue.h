/**
* \file	limiter_queue.h
* \author  	������ 724789975@qq.com
* \date 	2018��8��02�� 23ʱ04��26�� CST
* \brief 	һ����������
*
* ��Ҫ�����ڶ������� ��ֹ��������Ҫִ�еĻص�̫�� �������
* ԭ����ÿִ֡�й̶��������¼� ���� ������һ֡��ִ��ʱ�� ������֡���¼�ִ������
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

	//����Ǽ�¼�¼��ڶ����е�λ�� ������ٵ��Ļ� ֱ�ӰѶ����е�ָ�븳�վͿ�����
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


// NUM ��СֵΪ1
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

////һ���������� <ִ��ʱ���ж�����(Ҳ��map��key), key�����й���, �������ʱ��(����)> ������Ҫ���й���� ��ʱ�Ȳ�ʵ����
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

//�������ڶ��߳�
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

//�������ڶ��߳�
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
