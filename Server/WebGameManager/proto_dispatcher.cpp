#include "proto_dispatcher.h"
#include "google/protobuf/message.h"

namespace CallBackDispatcher
{
	//template <typename OwnerType>
	//bool ProtoDispatcherBase<OwnerType>::Dispatch(const std::string& refszName, const unsigned char* pData, unsigned int dwSize, OwnerType* pOwner /*= NULL*/)
	//{
	//	const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(refszName);
	//	CallbackMap::iterator it = m_mapMessageCallBack.find(pDescriptor);
	//	if (it == m_mapMessageCallBack.end())
	//	{
	//		return false;
	//	}
	//	const google::protobuf::Message *pPrototype =
	//		google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
	//	if (!pPrototype)
	//	{
	//		return false;
	//	}
	//	google::protobuf::Message* pMsg = pPrototype->New();
	//	if (!pMsg)
	//	{
	//		return false;
	//	}
	//	if (!pMsg->ParseFromArray(pData, dwSize))
	//	{
	//		return false;
	//	}
	//	it->second->exec(pOwner, pMsg);
	//	delete pMsg;
	//	pMsg = NULL;
	//	return true;
	//}

	bool GetProtoMessage(google::protobuf::Message** ppMsg, const google::protobuf::Descriptor** ppDescriptor, const std::string& refszName, const unsigned char* pData, unsigned int dwSize)
	{
		*ppDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(refszName);
		if (*ppDescriptor == NULL)
		{
			return false;
		}

		const google::protobuf::Message *pPrototype =
			google::protobuf::MessageFactory::generated_factory()->GetPrototype(*ppDescriptor);
		if (!pPrototype)
		{
			return false;
		}
		*ppMsg = pPrototype->New();
		if (!(*ppMsg))
		{
			return false;
		}
		if (!(*ppMsg)->ParseFromArray(pData, dwSize))
		{
			delete *ppMsg;
			return false;
		}

		return true;
	}
}

