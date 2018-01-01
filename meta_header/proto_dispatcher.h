#ifndef __PROTO_DISPATCHER_H__
#define __PROTO_DISPATCHER_H__
#include <functional>
#include <map>
#include "nulltype.h"
#include "callback_dispatch.h"

namespace google
{
	namespace protobuf
	{
		class Message;
		class Descriptor;
	}
}

namespace CallBackDispatcher
{
	template <typename OwnerType>
	class ProtobufCallback
	{
	public:
		ProtobufCallback() {}
		virtual ~ProtobufCallback() {};
		virtual void Exec(OwnerType* pOwner, const google::protobuf::Message* pMsg) const = 0;
	private:
		ProtobufCallback(const ProtobufCallback& ref) {}
		const ProtobufCallback& operator= (const ProtobufCallback& ref) {};
	};

	template <typename MsgType, typename OwnerType = NullType>
	class ProtoMessageCallback : public ProtobufCallback<OwnerType>
	{
	public:
		typedef std::function<void(OwnerType*, const MsgType&)>	Function;
		ProtoMessageCallback(const Function& refCallBack) : m_fpCallBack(refCallBack) {}
		virtual void Exec(OwnerType* pOwner, const google::protobuf::Message* pMsg) const
		{
			if (pOwner && pMsg)
			{
				m_fpCallBack(pOwner, *(const MsgType* )pMsg);
			}
		}
	private:
		Function m_fpCallBack;
	};

	template <typename MsgType>
	class ProtoMessageCallback<MsgType, NullType> : public ProtobufCallback<NullType>
	{
	public:
		typedef std::function<void(const MsgType&)>	Function;
		ProtoMessageCallback(const Function& refCallBack) : m_fpCallBack(refCallBack) {}
		virtual void Exec(NullType* pOwner, const google::protobuf::Message* pMsg) const
		{
			if (pMsg)
			{
				m_fpCallBack(*(const MsgType*)pMsg);
			}
		}
	private:
		Function m_fpCallBack;
	};

	template <typename OwnerType>
	class ProtoDispatcherBase
	{
	public:
		ProtoDispatcherBase(const std::string szName) : m_szName(szName) {}
		virtual ~ProtoDispatcherBase()
		{
			typename CallbackMap::iterator it = m_mapMessageCallBack.begin();
			for (; it != m_mapMessageCallBack.end(); ++it)
			{
				delete it->second;
				it->second = NULL;
			}
			m_mapMessageCallBack.clear();
		}
		bool Dispatch(const std::string& refszName, const unsigned char* pData, unsigned int dwSize, OwnerType* pOwner = NULL);
	protected:
		typedef std::map<const google::protobuf::Descriptor*, ProtobufCallback<OwnerType>* >	CallbackMap;
		CallbackMap	m_mapMessageCallBack;
	private:
		std::string m_szName;
	};

	template <typename OwnerType = NullType>
	class ProtoDispatcher : public ProtoDispatcherBase<OwnerType>
	{
	public:
		ProtoDispatcher(const std::string szName) : ProtoDispatcherBase<OwnerType>(szName) {}
		template <typename MsgType>
		void FuncReg(const typename ProtoMessageCallback<MsgType, OwnerType>::Function& refFunc)
		{
			ProtoDispatcherBase<OwnerType>::m_mapMessageCallBack[MsgType::descriptor()] =
				new ProtoMessageCallback<MsgType, OwnerType>(refFunc);
		}
	};

	template <>
	class ProtoDispatcher<NullType> : public ProtoDispatcherBase<NullType>
	{
	public:
		ProtoDispatcher(const std::string szName) : ProtoDispatcherBase<NullType>(szName) {}
		template <typename MsgType>
		void FuncReg(const typename ProtoMessageCallback<MsgType>::Function& refFunc)
		{
			ProtoDispatcherBase<NullType>::m_mapMessageCallBack[MsgType::descriptor()] =
				new ProtoMessageCallback<MsgType>(refFunc);
		}
	};


	bool GetMessage(google::protobuf::Message** ppMsg, const google::protobuf::Descriptor** ppDescriptor, const std::string& refszName, const unsigned char* pData, unsigned int dwSize);

	template <class Owner, typename P1>
	class ProtoCallBackDispatch : public ClassCallBackDispatcher<bool, const google::protobuf::Descriptor*, Owner, P1&, google::protobuf::Message&>
	{
		typedef ClassCallBackDispatcher<bool, const google::protobuf::Descriptor*, Owner, P1&, google::protobuf::Message&> BaseType;
	public:
		ProtoCallBackDispatch(Owner& refOwner)
			:ClassCallBackDispatcher(refOwner)
		{}

		//使用静态函数的目的是为了头文件跟cpp文件的分离
		inline bool Dispatch(const std::string& refszName, const unsigned char* pData, unsigned int dwSize, Owner* pOwner, P1& refP1)
		{
			google::protobuf::Message* pMsg = NULL;
			const google::protobuf::Descriptor* pDescriptor = NULL;
			if (!GetMessage(&pMsg, &pDescriptor, refszName, pData, dwSize))
			{
				return false;
			}
			BaseType::CallBackFunction pFun = GetFunction(pDescriptor);
			if (pFun == NULL)
			{
				delete pMsg;
				pMsg = NULL;
				return false;
			}

			BaseType::Dispatch(pFun, refP1, *pMsg);
			delete pMsg;
			pMsg = NULL;
			return true;
		}
	protected:
	private:
	};

	
}

#endif
