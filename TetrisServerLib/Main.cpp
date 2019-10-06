
#include "MqManager.h"
#include "MqPacket.h"
#include "Main.h"


namespace TetrisServerLib 
{
	Main::Main() 
	{
	}

	Main::~Main()
	{
	}

	int Main::Init(GameServerConfig serverConfig) 
	{

		m_pIOCPServer = std::make_unique<NetLib::IOCPServerNet>();
		
		auto netConfig = serverConfig.GetNetConfig();

		NetLib::E_NET_RESULT ServerStartResult = m_pIOCPServer->StartServer(netConfig);
		if (ServerStartResult != NetLib::E_NET_RESULT::Success) {
			printf("ServerStartError! ErrorCode: %d\n", (int)ServerStartResult);
			return -1;
		}

		m_pPacketManager = std::make_unique<PacketManager> ();
		m_pUserManager = std::make_unique<UserManager> ();
		m_pRoomManager = std::make_unique<RoomManager> ();

		m_pPacketManager->Init(m_pIOCPServer.get(), m_pUserManager.get(), m_pRoomManager.get());
		m_pUserManager->Init();
		m_pRoomManager->Init(m_pIOCPServer.get());

		max_packet_size = m_pIOCPServer->GetMaxPacketSize();
		max_connection_count = m_pIOCPServer->GetMaxConnectionCount();
		post_message_thread_cnt = m_pIOCPServer->GetPostMessagesThreadsCount();
		InitMq(serverConfig);
		
		return 0;
	}

	void Main::InitMq(GameServerConfig serverConfig)
	{
		m_pMqManager = std::make_unique<MqManager>();
		auto mqResult = m_pMqManager->Init(serverConfig.MqConfig,
			[&](char* pMpData)
			{
				m_MqDataQueue.push(pMpData);
			});

		m_pPacketManager->SetSendMQFunc([&](int dataSize, char* pMQData)
			{
				m_pMqManager->Send(dataSize, pMQData);
			});

		SendGameServerInfoToMq(serverConfig);
	}

	void Main::Run() 
	{
		m_IsRun = true;

		int MaxPacketSize = Common::MAX_PACKET_SIZE + 1;
		char* pBuf = new char[MaxPacketSize];
		ZeroMemory(pBuf, sizeof(char) * MaxPacketSize);
		INT32 waitTimeMillisec = 1;

		while (m_IsRun)
		{
			INT8 operationType = 0;
			INT32 connectionIndex = 0;
			INT16 copySize = 0;

			if (m_pIOCPServer->ProcessNetworkMessage(operationType, connectionIndex, pBuf, copySize, waitTimeMillisec))
			{
				switch (operationType)
				{
				case NetLib::OP_CONNECTION:
					printf("On Connect %d\n",connectionIndex);
					break;
				case NetLib::OP_CLOSE:
					m_pPacketManager->ClearConnectionInfo(connectionIndex);
					break;
				case NetLib::OP_RECV_PACKET:
					m_pPacketManager->ProcessRecvPacket(connectionIndex, pBuf, copySize);
					break;
				}
			}
			ProcessMqMessage();
		}
	}


	void Main::Stop()
	{
		m_IsRun = false;

		m_pIOCPServer->EndServer();
		m_pMqManager->Stop();
	}

	void Main::SendGameServerInfoToMq(GameServerConfig serverConfig)
	{
		MQReqGAServerInfo mqReqServerInfo;
		mqReqServerInfo.SetHeader((UINT16)MQ_GAMT_DATA_ID::REQ_SERVER_INFO, 0);
		mqReqServerInfo.StartRoomNumber = serverConfig.MqConfig.MQStartRoomNum;
		mqReqServerInfo.MaxRoomCount = serverConfig.MqConfig.MQMaxRoomCnt;
		strncpy_s(mqReqServerInfo.ServerIP, serverConfig.m_MyIPAddress.c_str(), serverConfig.m_MyIPAddress.length());
		mqReqServerInfo.ServerPort = serverConfig.m_PortNumber;

		char mqSendBuffer[256] = { 0, };
		auto sendSize = mqReqServerInfo.Encode(mqSendBuffer);
		m_pMqManager->Send(sendSize, mqSendBuffer);
	}

	void Main::ProcessMqMessage()
	{
		while (true)
		{
			char* pMessage = nullptr;
			if (m_MqDataQueue.try_pop(pMessage) == false)
			{
				return;
			}

			MQBinaryHead head;
			head.HeadDecode(pMessage);

			switch ((MQ_GAMT_DATA_ID)head.ID)
			{
			case MQ_GAMT_DATA_ID::RES_SERVER_INFO:
				{
					MQResGAServerInfo response;
					response.Decode(pMessage);

					printf("MQResGAServerInfo. result:%d\n", response.Result);
				}
				break;
			case MQ_GAMT_DATA_ID::NTF_MATCHING_INFO:
			{
				ProcessNotifyMatching(pMessage);
				printf("MQNtfGAMatchingInfo\n");
			}
			break;
			}
			
		}
	}


	void Main::ProcessNotifyMatching(char* pMessage)
	{
		MQNtfGAMatchingInfo packet;
		packet.Decode(pMessage);

		int RoomNum = packet.RoomNumber;

		if (RoomNum < 0 || RoomNum > m_pRoomManager->GetMaxRoomCount()) {
			printf("[Error] 매칭된 방의 번호가 유효하지 않습니다.");
			return;
		}

		Room* pMatchedRoom = m_pRoomManager->GetRoomByIndex(RoomNum);
		if (!pMatchedRoom) {
			printf("[Error] 매칭된 방이 유효하지 않습니다.");
			return;
		}

		printf("%d번방으로 매칭됨", RoomNum);
		pMatchedRoom->SetAuthUserList(packet.UserID1, packet.UserID2);
	}

}