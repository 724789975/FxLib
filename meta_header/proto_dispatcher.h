#ifndef __PROTO_DISPATCHER_H__
#define __PROTO_DISPATCHER_H__
#include <functional>
#include <map>

namespace google
{
	namespace protobuf
	{
		class Message;
		class Descriptor;
	}
}

namespace ProtoBufDispatch {

	class NullType;

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
}

#define	declare_msg_func(msg_type)	\
	void on_##msg_type(const msg::msg_type &rev)

#define reg_msg_func(dispatcher, handler_classname, msg_type)	\
	(dispatcher).func_reg< msg::msg_type >(std::bind(&handler_classname::on_##msg_type, this, _1))


//#define DISPATCHER_FUNC_HAS_OWNER(dispatcher, msg_type, owner_type, p_instance, func)
	//(dispatcher).func_reg< msg::msg_type >(Fir::CmdCallback< msg::msg_type, owner_type >::Function((p_instance), &func));

#endif
