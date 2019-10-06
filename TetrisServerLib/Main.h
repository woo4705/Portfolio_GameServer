#pragma once

#include <memory>
#include <concurrent_queue.h>
#include "../TetrisServerLib/Define.h"
#include "PacketManager.h"

namespace TetrisServerLib 
{
	class MqManager;

	class Main
	{
	public:
		Main();
		~Main();

		int Init(GameServerConfig serverConfig);		
		void Run();	
		void Stop();
		
		NetLib::IOCPServerNet* GetIOCPServer() { return  m_pIOCPServer.get(); }

	private:
		void InitMq(GameServerConfig serverConfig);
		void SendGameServerInfoToMq(GameServerConfig serverConfig);
		void ProcessMqMessage();
		void ProcessNotifyMatching(char* pMessage);

		std::unique_ptr<NetLib::IOCPServerNet> m_pIOCPServer;
		std::unique_ptr<UserManager> m_pUserManager;
		std::unique_ptr<PacketManager> m_pPacketManager;
		std::unique_ptr<RoomManager> m_pRoomManager;
		std::unique_ptr<MqManager> m_pMqManager;

		Concurrency::concurrent_queue<char*> m_MqDataQueue;

		int max_packet_size;
		int max_connection_count;
		int post_message_thread_cnt;
		int MaxRoomCnt;

		bool m_IsRun = false;
	};

}