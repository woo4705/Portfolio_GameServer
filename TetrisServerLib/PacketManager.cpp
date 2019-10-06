#define _CRT_SECURE_NO_WARNINGS
#include "Packet.h"
#include "PacketManager.h"
#include <utility>
#include <cstring>

#include "MqPacket.h"
using namespace Common;

namespace TetrisServerLib
{
	void PacketManager::Init(NetLib::IOCPServerNet* pIOCPServer, UserManager* pUserManager, RoomManager* pRoomManager) {

		m_RecvFuntionDictionary = std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION>();

		m_RecvFuntionDictionary[(int)Common::PACKET_ID::LOGIN_REQUEST] = &PacketManager::ProcessLogin;
		m_RecvFuntionDictionary[(int)PACKET_ID::ROOM_ENTER_REQUEST] = &PacketManager::ProcessEnterRoom;
		m_RecvFuntionDictionary[(int)PACKET_ID::ROOM_LEAVE_REQUEST] = &PacketManager::ProcessLeaveRoom;
		m_RecvFuntionDictionary[(int)PACKET_ID::ROOM_CHAT_REQUEST] = &PacketManager::ProcessRoomChatMessage;
		m_RecvFuntionDictionary[(int)PACKET_ID::GAME_SYNC_REQUEST] = &PacketManager::ProcessGameSyncPacket;
		m_RecvFuntionDictionary[(int)PACKET_ID::GAME_START_REQUEST] = &PacketManager::ProcessGameStartPacket;
		m_RecvFuntionDictionary[(int)PACKET_ID::GAME_END_REQUEST] = &PacketManager::ProcessGameEndPacket;

		m_pIOCPServer = pIOCPServer;
		m_pUserManager = pUserManager;
		m_pRoomManager = pRoomManager;
	}



	void PacketManager::ProcessRecvPacket(INT32 connectionIndex, char* pBuf, INT16 copySize) {
		PACKET_HEADER* pHeader = reinterpret_cast<PACKET_HEADER*>(pBuf);

		auto iter = m_RecvFuntionDictionary.find(pHeader->PacketId);
		if (iter != m_RecvFuntionDictionary.end())
		{
			(this->*(iter->second))(connectionIndex, pBuf, copySize);
		}
	}



	void PacketManager::ProcessLogin(const INT32 connIndex, char* pBuf, INT16 copySize) { //TODO ���� Ŭ���� ������ �ֱ�
		LOGIN_REQUEST_PACKET* pLoginReqPacket = reinterpret_cast<LOGIN_REQUEST_PACKET*>(pBuf);

		char* userID = pLoginReqPacket->UserID;
		printf("requested user id = %s\n", userID);

		LOGIN_RESPONSE_PACKET loginResPacket;
		loginResPacket.PacketId = (UINT16)PACKET_ID::LOGIN_RESPONSE;
		loginResPacket.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);

		if (m_pUserManager->GetCurrentUserCnt() >= m_pUserManager->GetMaxUserCnt()) { //�����ڼ��� �ִ���� �����ؼ� ���ӺҰ�
			loginResPacket.Result = (UINT16)ERROR_CODE::LOGIN_USER_USED_ALL_OBJ;

			m_pIOCPServer->SendPacket(connIndex, &loginResPacket, sizeof(LOGIN_RESPONSE_PACKET));
			return;
		}

		//���⿡�� �̹� ���ӵ� �������� Ȯ���ϰ�, ���ӵ� ������� �����Ѵ�.
		if (m_pUserManager->FindUserByID(userID) == -1) { //�������� �ƴ� �������
			//������ �����ϴ� ��ü�� �ִ´�.
			m_pUserManager->AddUser(userID, connIndex);
			loginResPacket.Result = (UINT16)ERROR_CODE::NONE;
		}
		else { //�������� �������� ���и� ��ȯ�Ѵ�.
			loginResPacket.Result = (UINT16)ERROR_CODE::LOGIN_USER_ALREADY;
			m_pIOCPServer->SendPacket(connIndex, &loginResPacket, sizeof(LOGIN_RESPONSE_PACKET));
			return;
		}

		m_pIOCPServer->SendPacket(connIndex, &loginResPacket, sizeof(LOGIN_RESPONSE_PACKET));
		//�����Ҵ��� �ϸ� ���� �Լ����� RingBuffer�� ������ ������. ���� �����Ҵ�ó�� ������ �ʿ䰡 ������

		//�ش� Ŭ���̾�Ʈ�� ����� ���ǿ� �α��� ������ �����Ѵ�.

	}



	void PacketManager::ProcessEnterRoom(INT32 connIndex, char* pBuf, INT16 copySize) {
		ROOM_ENTER_REQUEST_PACKET* pRoomEnterReqPacket = reinterpret_cast<ROOM_ENTER_REQUEST_PACKET*>(pBuf);
		User* reqUser = m_pUserManager->GetUserByConnIdx(connIndex);

		if (!reqUser || reqUser == nullptr) {
			return;
		}

		int roomNum = pRoomEnterReqPacket->RoomIndex;
		Room* enterRoom = m_pRoomManager->GetRoomByIndex(roomNum);

		if (enterRoom == nullptr) {
			return;
		}


		ROOM_ENTER_RESPONSE_PACKET roomEnterResPacket;
		roomEnterResPacket.PacketId = (UINT16)PACKET_ID::ROOM_ENTER_RESPONSE;
		roomEnterResPacket.PacketLength = sizeof(ROOM_ENTER_RESPONSE_PACKET);

		if ( reqUser->GetUserId().compare( enterRoom->GetAuthUser(0) ) == 0 ) {
			strcpy(roomEnterResPacket.RivalUserID ,enterRoom->GetAuthUser(1).c_str() );
		}
		else if (reqUser->GetUserId().compare(enterRoom->GetAuthUser(1)) == 0) {
			strcpy(roomEnterResPacket.RivalUserID, enterRoom->GetAuthUser(0).c_str());
		}
		else {
			(UINT16)ERROR_CODE::ENTER_ROOM_NOT_FINDE_USER;
			m_pIOCPServer->SendPacket(connIndex, &roomEnterResPacket, sizeof(ROOM_ENTER_RESPONSE_PACKET));
			return;
		}



		if (m_pRoomManager != nullptr) {
			roomEnterResPacket.Result = m_pRoomManager->EnterUser(pRoomEnterReqPacket->RoomIndex, reqUser);
		}
		else {
			roomEnterResPacket.Result = (UINT16)ERROR_CODE::USER_MGR_INVALID_USER_INDEX;
		}

		m_pIOCPServer->SendPacket(connIndex, &roomEnterResPacket, sizeof(ROOM_ENTER_RESPONSE_PACKET));
		printf("Response Packet Sended");

	}


	void PacketManager::ProcessLeaveRoom(INT32 connIndex, char* pBuf, INT16 copySize) {
		ROOM_LEAVE_REQUEST_PACKET* pRoomLeaveReqPacketet = reinterpret_cast<ROOM_LEAVE_REQUEST_PACKET*>(pBuf);
		ROOM_LEAVE_RESPONSE_PACKET roomLeaveResPacket;
		roomLeaveResPacket.PacketId = (UINT16)PACKET_ID::ROOM_LEAVE_RESPONSE;
		roomLeaveResPacket.PacketLength = sizeof(ROOM_LEAVE_RESPONSE_PACKET);

		User* reqUser = m_pUserManager->GetUserByConnIdx(connIndex);
		UINT16 roomNum = reqUser->GetCurrentRoom();

		if (roomNum < 0 || roomNum > m_pRoomManager->GetMaxRoomCount()) {//������ ChatRoom�� ������ �ʰų� ���� �߸��� ���
			roomLeaveResPacket.Result = (UINT16)ERROR_CODE::CHAT_ROOM_INVALID_ROOM_INDEX;
			return;
		}
		//TODO Room���� UserList��ü�� �� Ȯ���ϱ�
		m_pRoomManager->LeaveUser(roomNum, reqUser);
		roomLeaveResPacket.Result = (UINT16)ERROR_CODE::NONE;
		m_pIOCPServer->SendPacket(connIndex, &roomLeaveResPacket, sizeof(ROOM_LEAVE_RESPONSE_PACKET));

	}


	void PacketManager::ProcessRoomChatMessage(INT32 connIndex, char* pBuf, INT16 copySize) {
		ROOM_CHAT_REQUEST_PACKET* pRoomChatReqPacketet = reinterpret_cast<ROOM_CHAT_REQUEST_PACKET*>(pBuf);
		ROOM_CHAT_RESPONSE_PACKET roomChatResPacket;
		roomChatResPacket.PacketId = (UINT16)PACKET_ID::ROOM_CHAT_RESPONSE;
		roomChatResPacket.PacketLength = sizeof(ROOM_CHAT_RESPONSE_PACKET);

		User* reqUser = m_pUserManager->GetUserByConnIdx(connIndex);
		UINT16 roomNum = reqUser->GetCurrentRoom();

		if (roomNum < 0 || roomNum > m_pRoomManager->GetMaxRoomCount()) {//������ ChatRoom�� ������ �ʰų� ���� �߸��� ���
			roomChatResPacket.Result = (UINT16)ERROR_CODE::CHAT_ROOM_INVALID_ROOM_INDEX;
			return;
		}
		//TODO Room���� UserList��ü�� �� Ȯ���ϱ�
		Room* chatRoom = m_pRoomManager->GetRoomByIndex(roomNum);

		chatRoom->NotifyChat(connIndex, reqUser->GetUserId().c_str(), pRoomChatReqPacketet->Message);
		roomChatResPacket.Result = (INT16)ERROR_CODE::NONE;
		//TODO NofifyChat�� �������� �з��ϸ� �׿� ���� ERROR_CODE���� ������Ŷ�� Result�� ������.
		m_pIOCPServer->SendPacket(connIndex, &roomChatResPacket, sizeof(ROOM_CHAT_RESPONSE_PACKET));

	}

	void PacketManager::ProcessGameSyncPacket(INT32 connIndex, char* pBuf, INT16 copySize) {
		GAME_SYNCHRONIZE_REQUEST_PACKET* pGameSyncReqPacket = reinterpret_cast<GAME_SYNCHRONIZE_REQUEST_PACKET*>(pBuf);
		GAME_SYNCHRONIZE_NOTIFY_PACKET GameSyncNotifyPacket;

		if (copySize != 29) { //�������� �� �ִ� ũ��� �� ��Ŷ�� �´´ٸ� ������Ŷ
			return;
		}
		User* reqUser = m_pUserManager->GetUserByConnIdx(connIndex);
		UINT16 roomNum = reqUser->GetCurrentRoom();

		if (roomNum < 0 || roomNum > m_pRoomManager->GetMaxRoomCount()) {
			return;
		}

		Room* gameRoom = m_pRoomManager->GetRoomByIndex(roomNum);

		for (int i = 0; i < 6; i++) {
			GameSyncNotifyPacket.EventRecordArr[i] = pGameSyncReqPacket->EventRecordArr[i];
		}

		GameSyncNotifyPacket.PacketId = (INT16)PACKET_ID::GAME_SYNC_NOTIFY;
		GameSyncNotifyPacket.PacketLength = sizeof(GAME_SYNCHRONIZE_NOTIFY_PACKET);
		GameSyncNotifyPacket.Score = pGameSyncReqPacket->Score;
		GameSyncNotifyPacket.Line = pGameSyncReqPacket->Line;
		GameSyncNotifyPacket.Level = pGameSyncReqPacket->Level;


		gameRoom->SendToAllUser(
			(UINT16)PACKET_ID::GAME_SYNC_NOTIFY, 
			sizeof(GAME_SYNCHRONIZE_NOTIFY_PACKET), 
			&GameSyncNotifyPacket, 
			connIndex,
			true
		);

	}

	void PacketManager::ProcessGameStartPacket(INT32 connIndex, char* pBuf, INT16 copySize)
	{
		GAME_START_REQUEST_PACKET* pGameStartReqPacket = reinterpret_cast<GAME_START_REQUEST_PACKET*>(pBuf);
		GAME_START_RESPONSE_PACKET GameStartResPacket;

		User* reqUser = m_pUserManager->GetUserByConnIdx(connIndex);
		UINT16 roomNum = reqUser->GetCurrentRoom();

		if (roomNum < 0 || roomNum > m_pRoomManager->GetMaxRoomCount()) {//������ ChatRoom�� ������ �ʰų� ���� �߸��� ���
			return;
		}
		//TODO Room���� UserList��ü�� �� Ȯ���ϱ�
		Room* gameRoom = m_pRoomManager->GetRoomByIndex(roomNum);
		if (gameRoom->GetCurrentUserCount() == 2 ) {
			GameStartResPacket.Result = (INT16)ERROR_CODE::NONE;
			GAME_START_NOTIFY_PACKET GameStartNotifyPacket;
			GameStartNotifyPacket.PacketId = (INT16)PACKET_ID::GAME_START_NOTIFY;
			GameStartNotifyPacket.PacketLength = sizeof(GAME_START_NOTIFY_PACKET);
			gameRoom->SendToAllUser((UINT16)PACKET_ID::GAME_START_NOTIFY, sizeof(GAME_START_NOTIFY_PACKET), &GameStartNotifyPacket, connIndex, false);
		}
		else {
			//����� ���� �˸��� �ʽ��ϴ� ������Ŷ ������
		}


	}



	void PacketManager::ProcessGameEndPacket(INT32 connIndex, char* pBuf, INT16 copySize) {
		GAME_END_REQUEST_PACKET* pGameEndReqPacket = reinterpret_cast<GAME_END_REQUEST_PACKET*>(pBuf);
		GAME_END_RESPONSE_PACKET GameEndResPacket;

		User* reqUser = m_pUserManager->GetUserByConnIdx(connIndex);
		UINT16 roomNum = reqUser->GetCurrentRoom();

		if (roomNum < 0 || roomNum > m_pRoomManager->GetMaxRoomCount()) {//������ GameRoom�� ������ �ʰų� ���� �߸��� ���
			return;
		}
		//TODO Room���� UserList��ü�� �� Ȯ���ϱ�
		Room* gameRoom = m_pRoomManager->GetRoomByIndex(roomNum);
		if (gameRoom->GetCurrentUserCount() > 0) { //������ 2�ε� �׽�Ʈ������ 0�س���
			GameEndResPacket.Result = (INT16)ERROR_CODE::NONE;
			GAME_END_NOTIFY_PACKET GameEndNotifyPacket;
			GameEndNotifyPacket.PacketId = (INT16)PACKET_ID::GAME_END_NOTIFY;
			GameEndNotifyPacket.PacketLength = sizeof(GAME_END_NOTIFY_PACKET);
		
			gameRoom->SendToAllUser((UINT16)PACKET_ID::GAME_END_NOTIFY, sizeof(GAME_END_NOTIFY_PACKET), &GameEndNotifyPacket, connIndex, true);
		}

		char data[128] = { 0, };

		MQNtfGAUnUsedRoom ntfUnusedRoom;
		ntfUnusedRoom.RoomNumber = roomNum;
		ntfUnusedRoom.SetHeader((UINT16)MQ_GAMT_DATA_ID::NTF_UN_USED_ROOM, connIndex);
		auto size = ntfUnusedRoom.Encode(data);

		m_SendMQDataFunc(size, data);
	}





	void PacketManager::ClearConnectionInfo(INT32 connIndex) {
		User* reqUser = m_pUserManager->GetUserByConnIdx(connIndex);

		if (reqUser->GetDomainState() == User::DOMAIN_STATE::ROOM) {
			INT16 roomNum = reqUser->GetCurrentRoom();
			m_pRoomManager->LeaveUser(roomNum, reqUser);
		}

		if (reqUser->GetDomainState() != User::DOMAIN_STATE::NONE) {
			m_pUserManager->DeleteUserInfo(reqUser);
		}
	}




}