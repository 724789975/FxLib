#ifndef __PROTO_DISPATCHER_H__
#define __PROTO_DISPATCHER_H__
#include <functional>
#include <map>
#include <string>
#include "meta/meta.h"
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

	template <typename MsgType, typename OwnerType = Meta::Null>
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
	class ProtoMessageCallback<MsgType, Meta::Null> : public ProtobufCallback<Meta::Null>
	{
	public:
		typedef std::function<void(const MsgType&)>	Function;
		ProtoMessageCallback(const Function& refCallBack) : m_fpCallBack(refCallBack) {}
		virtual void Exec(Meta::Null* pOwner, const google::protobuf::Message* pMsg) const
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

	template <typename OwnerType = Meta::Null>
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
	class ProtoDispatcher<Meta::Null> : public ProtoDispatcherBase<Meta::Null>
	{
	public:
		ProtoDispatcher(const std::string szName) : ProtoDispatcherBase<Meta::Null>(szName) {}
		template <typename MsgType>
		void FuncReg(const typename ProtoMessageCallback<MsgType>::Function& refFunc)
		{
			ProtoDispatcherBase<Meta::Null>::m_mapMessageCallBack[MsgType::descriptor()] =
				new ProtoMessageCallback<MsgType>(refFunc);
		}
	};


	bool GetProtoMessage(google::protobuf::Message** ppMsg, const google::protobuf::Descriptor** ppDescriptor, const std::string& refszName, const unsigned char* pData, unsigned int dwSize);

	template <class Owner, typename P1>
	class ProtoCallBackDispatch : public ClassCallBackDispatcher<bool, const google::protobuf::Descriptor*, Owner, P1&, google::protobuf::Message&>
	{
		typedef ClassCallBackDispatcher<bool, const google::protobuf::Descriptor*, Owner, P1&, google::protobuf::Message&> BaseType;
	public:
		ProtoCallBackDispatch(Owner& refOwner)
			:BaseType(refOwner)
		{}

		inline bool Dispatch(const std::string& refszName, const unsigned char* pData, unsigned int dwSize, Owner* pOwner, P1& refP1)
		{
			google::protobuf::Message* pMsg = NULL;
			const google::protobuf::Descriptor* pDescriptor = NULL;
			if (!GetProtoMessage(&pMsg, &pDescriptor, refszName, pData, dwSize))
			{
				return false;
			}
			typename BaseType::CallBackFunction pFun = BaseType::GetFunction(pDescriptor);
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
