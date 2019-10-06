#pragma once
#include "Room.h"

namespace TetrisServerLib
{
	class RoomManager
	{
	public:
		void Init(NetLib::IOCPServerNet* pIOCPServer);
		UINT GetMaxRoomCount() { return MAX_ROOM_COUNT; }
		UINT GetCurrentRoomCount() { return m_CurrentRoomCount; }

		UINT16 EnterUser(int RoomIdx, User* pUser);
		UINT16 LeaveUser(int RoomIdx, User* pUser);

		Room* GetRoomByIndex(UINT16 index) { return m_RoomList[index]; } //TODO index�� �߸��� ���϶� ����ó�� ���ֱ�

		
	private:
		std::vector<Room*> m_RoomList;
		UINT16 MAX_ROOM_COUNT = 10; //TODO ����� ���� �ϵ� �ڵ� ���ϱ�
		UINT16 m_CurrentRoomCount = 0;
		NetLib::IOCPServerNet* m_pIOCPServer;//TODO ����� std::function�� ����Ͽ� �ʿ��� �κи� �����ϱ�

	};
}