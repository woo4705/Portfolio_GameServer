#pragma once

#include "Packet.h"
#include <string>


namespace TetrisServerLib
{
	class User
	{
	public:
		enum class DOMAIN_STATE {
			NONE = 0,
			LOGIN = 1,
			ROOM = 2
		};


		User() {}
		~User() {}

		void Init(const int index);

		int SetLogin(int conn_idx, char* login_id);
		void EnterRoom(int roomIndex);
		void Clear();
		void SetDomainState(DOMAIN_STATE val);

		INT16 GetCurrentRoom() {
			return m_RoomIndex;
		}

		INT32 GetNetConnIdx() {
			return m_NetConnIdx;
		}

		std::string GetUserId() {
			return  m_UserId;
		}

		DOMAIN_STATE GetDomainState() {
			return m_CurDomainState;
		}

		//�� ��ü���� �������� Pooling�Ѵ�



	private:
		//TODO type�� window���� ���ǵȰ����� (INT16 INT32 INT64) �� ������ ������ header�� window.h
		//�̰��� cross �÷����� �ƴ϶� windows������ ����ϱ⿡ ���� type. ���� ũ�ν��÷����� ���ٸ� c++���� ���ǵ� Ÿ���� ��

		INT32 m_UserIndex = -1;
		INT32 m_SessionIndex = -1;
		INT16 m_RoomIndex = -1;

		std::string m_UserId;
		bool m_IsConfirm = false;
		std::string AuthToken;

		INT32 m_NetConnIdx = -1;

		DOMAIN_STATE m_CurDomainState = DOMAIN_STATE::NONE;

		//public�� �� ����. protected�� Ư���� ��� �ƴϸ� �Ⱦ��°� ����. public>protect>private������



	};

}