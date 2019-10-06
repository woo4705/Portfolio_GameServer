#pragma once

#include <thread>
#include <functional>
#include <string>

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include "Define.h"


namespace TetrisServerLib 
{
	class MqBase
	{
	public:
		MqBase();
		~MqBase();
		
		void Init(amqp_channel_t channelNum, const char* queueName);
		
		void Destory();

		int Connect(char const* hostname, const int port, const char* userID, const char* userPW, const char* vhost);

		amqp_connection_state_t GetAmqpConnection() { return m_pConnState; }
		

	protected:
		amqp_connection_state_t m_pConnState = nullptr;
		amqp_socket_t* m_pSocket = nullptr;
		amqp_channel_t m_ChannelNum = 0;
		std::string m_QueueName;
	};


	class MqSender : public MqBase
	{
	public:
		MqSender();
		~MqSender();

		bool Send(const size_t dataSize, char* pData);

	private:
		
	};

	class MqReceiver : public MqBase
	{
	public:
		MqReceiver();
		~MqReceiver();

		int BindAndComsume();

	};


	
	class MqManager
	{
	public:
		MqManager();
		~MqManager();

		int Init(MqConfig config, std::function<void(char*)> mqMessageInsert);

		void Stop();

		bool Send(const size_t dataSize, char* pData);

		
	private:
		void ThreadFunc();



		MqSender m_Sender;
		MqReceiver m_Receiver;

		std::function<void(char*)> MqMessageInsert;

		bool m_IsReceiveThreadRun = false;
		std::thread m_ReceiveThread;
	};
}