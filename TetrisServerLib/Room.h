#pragma once

#include "UserManager.h"

namespace TetrisServerLib
{
	class Room {
	public:
		Room() {}
		~Room() {}
		void Init(UINT16 roomNum, NetLib::IOCPServerNet* pIOCPServer);

		UINT16 GetMaxUserCount() { return MAX_USER_COUNT; }
		UINT16 GetCurrentUserCount() { return m_CurrentUserCount; }
		UINT16 GetRoomNumber() { return m_RoomNum; }

		UINT16 EnterUser(User* pUser);
		UINT16 LeaveUser(User* pUser);

		void SetAuthUserList(char* UserID1, char* UserID2);
		std::string GetAuthUser(int idx) { return m_AuthUserList[idx]; }

		void Update();
		void NotifyChat(INT32 connIndex, const char* UserID, const char* Msg);
		void SendToAllUser(const UINT16 packetID, const UINT16 dataSize, void* pData, const UINT32 passUserindex, bool exceptMe);

	private:
		UINT16 m_RoomNum = -1;

		std::vector<User*> m_UserList;
		//ToDo 룸에서 사용자를 미리넣고, Enter가 되면
		//1. 들어올 수 있는 유저인지(UserList에 있는 유저만)
		

		UINT16 MAX_USER_COUNT = 2; //1:1의 특성을 고려해서 2명으로 설정
		UINT16 m_CurrentUserCount = 0;
		std::string m_AuthUserList[2];


		NetLib::IOCPServerNet* m_pIOCPServer;

	};
}