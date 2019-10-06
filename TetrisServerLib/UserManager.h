#pragma once
#include "User.h"
#include <unordered_map>


namespace TetrisServerLib
{
	class UserManager
	{

	public:
		UserManager() {};
		~UserManager() {};

		void Init();

		int GetCurrentUserCnt() { return current_user_cnt; } //TODO ����� �ڵ尡 ���� ������ �Լ� ���� ������ �ش�. �ٸ� ���ϵ� ���� �����ϱ�
		int GetMaxUserCnt() { return MAX_USER_CNT; }
		void IncreaseUserCnt() { current_user_cnt++; }
		void DecreaseUserCnt() {
			if (current_user_cnt > 0) {
				current_user_cnt--;
			}
		}

		int AddUser(char* userID, int conn_idx);
		int FindUserByID(char* userID);
		void DeleteUserInfo(User* deleteUser);

		User* GetUserByConnIdx(INT32 conn_idx);


	private:
		const int MAX_USER_CNT = 1024; //TODO ����� �� ���� �ϵ��ڵ��� �ƴ� �ܺο��� ���� �޵��� �Ѵ�
		int current_user_cnt = 0; //TODO ����� �ڵ� �꿡 �°� �̸� �ٲٱ�

		std::vector<User*> UserObjPool; //vector��
		std::unordered_map<std::string, int> UserDictionary;


	};
}