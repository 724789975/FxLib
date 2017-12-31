#include "proto_dispatcher.h"
#include "google/protobuf/message.h"

namespace CallBackDispatcher
{
	template <typename OwnerType>
	bool ProtoDispatcherBase<OwnerType>::Dispatch(const std::string& refName, const unsigned char* pData, unsigned int dwSize, OwnerType* pOwner /*= NULL*/)
	{
		const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(refName);
		CallbackMap::iterator it = m_mapMessageCallBack.find(pDescriptor);
		if (it == m_mapMessageCallBack.end())
		{
			return false;
		}
		const google::protobuf::Message *pPrototype =
			google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
		if (!pPrototype)
		{
			return false;
		}
		google::protobuf::Message* pMsg = pPrototype->New();
		if (!pMsg)
		{
			return false;
		}
		if (!pMsg->ParseFromArray(pData, dwSize))
		{
			return false;
		}
		it->second->exec(pOwner, pMsg);
		delete pMsg;
		pMsg = NULL;
		return true;
	}
}

