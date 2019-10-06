#pragma once
#include <unordered_map>
#include <functional>

#include "RoomManager.h"

namespace TetrisServerLib
{
	class PacketManager {
	public:
		PacketManager() {};
		~PacketManager() {};
		void Init(NetLib::IOCPServerNet* pIOCPServer, UserManager* pUserManager, RoomManager* pRoomManager);

		void ProcessRecvPacket(INT32 connectionIndex, char* pBuf, INT16 copySize);
		void ProcessLogin(INT32 connIndex, char* pBuf, INT16 copySize);
		void ProcessEnterRoom(INT32 connIndex, char* pBuf, INT16 copySize);
		void ProcessLeaveRoom(INT32 connIndex, char* pBuf, INT16 copySize);
		void ProcessRoomChatMessage(INT32 connIndex, char* pBuf, INT16 copySize);
		void ProcessGameSyncPacket(INT32 connIndex, char* pBuf, INT16 copySize);
		void ProcessGameStartPacket(INT32 connIndex, char* pBuf, INT16 copySize);
		void ProcessGameEndPacket(INT32 connIndex, char* pBuf, INT16 copySize);

		void ClearConnectionInfo(INT32 connIndex);
		
		void SetSendMQFunc(std::function<void(int, char*)> sendMQDataFunc) 
		{
			m_SendMQDataFunc = sendMQDataFunc;
		}


	private:
		typedef void(PacketManager::* PROCESS_RECV_PACKET_FUNCTION)(INT32, char*, INT16);
		std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION> m_RecvFuntionDictionary;

		NetLib::IOCPServerNet* m_pIOCPServer;
		UserManager* m_pUserManager;
		RoomManager* m_pRoomManager;

		//int->������ ũ��/char*->����Ʈ�迭(������)
		std::function<void(int, char*)> m_SendMQDataFunc;
	};
}
