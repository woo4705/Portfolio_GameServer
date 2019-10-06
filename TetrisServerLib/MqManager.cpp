#include "MqPacket.h"
#include "MqManager.h"

namespace TetrisServerLib {
	// MqBase
	MqBase::MqBase()
	{
	}

	MqBase::~MqBase()
	{
	}
		
	int MqBase::Connect(char const* hostname, const int port, const char* userID, const char* userPW, const char* vhost)
	{
		m_pConnState = amqp_new_connection();

		m_pSocket = amqp_tcp_socket_new(m_pConnState);
		if (m_pSocket == nullptr) {
			return 1;
		}

		int status = amqp_socket_open(m_pSocket, hostname, port);
		if (status) {
			return 2;
		}

		auto loginRet = amqp_login(m_pConnState, vhost, 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, userID, userPW);
		if (loginRet.reply_type != AMQP_RESPONSE_NORMAL)
		{
			return 3;
		}

		amqp_channel_open(m_pConnState, m_ChannelNum);
		auto openChannelRet = amqp_get_rpc_reply(m_pConnState);
		if (openChannelRet.reply_type != AMQP_RESPONSE_NORMAL)
		{
			return 5;
		}

		return 0;
	}

	void MqBase::Init(amqp_channel_t channelNum, const char* queueName)
	{
		m_ChannelNum = channelNum;
		m_QueueName = queueName;
	}

	void MqBase::Destory()
	{
		amqp_channel_close(m_pConnState, m_ChannelNum, AMQP_REPLY_SUCCESS);
		amqp_connection_close(m_pConnState, AMQP_REPLY_SUCCESS);
		amqp_destroy_connection(m_pConnState);
	}


	// MqSender
	MqSender::MqSender()
	{
	}

	MqSender::~MqSender()
	{
	}
		
	bool MqSender::Send(const size_t dataSize, char* pData)
	{
		amqp_bytes_t message_bytes;
		message_bytes.len = dataSize;
		message_bytes.bytes = pData;
		
		auto result = amqp_basic_publish(m_pConnState, m_ChannelNum, amqp_cstring_bytes(""),
			amqp_cstring_bytes(m_QueueName.c_str()), 0, 0, NULL,
			message_bytes);

		if (result < 0)
		{
			return false;
		}

		return true;
	}




	// MqReceiver
	MqReceiver::MqReceiver()
	{
		m_ChannelNum = 2;
	}

	MqReceiver::~MqReceiver()
	{
	}

	int MqReceiver::BindAndComsume()
	{
		amqp_table_t amqpEmptyTable; amqpEmptyTable.num_entries = 0;
		amqp_bytes_t amqpEmptyBytes; amqpEmptyBytes.len = 0;
		
		amqp_queue_declare_ok_t* r = amqp_queue_declare(
			m_pConnState, m_ChannelNum, amqp_cstring_bytes(m_QueueName.c_str()), 0, 0, 0, 1, amqpEmptyTable);
		auto rpcGetRet = amqp_get_rpc_reply(m_pConnState);
		if (rpcGetRet.reply_type != AMQP_RESPONSE_NORMAL)
		{
			return 10;
		}				


		amqp_basic_consume(m_pConnState, m_ChannelNum, amqp_cstring_bytes(m_QueueName.c_str()), amqpEmptyBytes, 0, 1, 0,
			amqpEmptyTable);

		auto bindRet = amqp_get_rpc_reply(m_pConnState);
		if (rpcGetRet.reply_type != AMQP_RESPONSE_NORMAL)
		{
			return 12;
		}

		return 0;
	}


	// MqManager
	MqManager::MqManager()
	{
	}

	MqManager::~MqManager()
	{
	}

	int MqManager::Init(MqConfig config, std::function<void(char*)> mqMessageInsert)
	{
		int result = 0;
				
		m_Sender.Init(1, config.SendToMatchQueueName.c_str());

		result = m_Sender.Connect(config.Hostname.c_str(), config.Port, config.UserID.c_str(), config.UserPW.c_str(), config.VHost.c_str());
		if (result != 0)
		{
			return result;
		}

		m_Receiver.Init(2, config.ReceivFromMatchQueueName.c_str());

		result = m_Receiver.Connect(config.Hostname.c_str(), config.Port, config.UserID.c_str(), config.UserPW.c_str(), config.VHost.c_str());
		if (result != 0)
		{
			return result;
		}

		result = m_Receiver.BindAndComsume();
		if (result != 0)
		{
			return result;
		}
		
		
		MqMessageInsert = mqMessageInsert;

		m_IsReceiveThreadRun = true;
		m_ReceiveThread = std::thread(&MqManager::ThreadFunc, this);
		return 0;
	}

	void MqManager::Stop()
	{
		m_IsReceiveThreadRun = false;

		m_Receiver.Destory();
		m_Sender.Destory();		

		if (m_ReceiveThread.joinable())
		{
			m_ReceiveThread.join();
		}
	}

	bool MqManager::Send(const size_t dataSize, char* pData)
	{
		auto result = m_Sender.Send(dataSize, pData);
		return result;
	}

	void MqManager::ThreadFunc()
	{
		while (m_IsReceiveThreadRun)
		{
			amqp_rpc_reply_t res;
			amqp_envelope_t envelope;

			amqp_maybe_release_buffers(m_Receiver.GetAmqpConnection());

			res = amqp_consume_message(m_Receiver.GetAmqpConnection(), &envelope, NULL, 0);

			if (AMQP_RESPONSE_NORMAL != res.reply_type) {
				break;
			}

			auto pMQBody = new char[envelope.message.body.len];
			CopyMemory(pMQBody, envelope.message.body.bytes, envelope.message.body.len);

			MqMessageInsert(pMQBody);

			amqp_destroy_envelope(&envelope);
		}
	}
}