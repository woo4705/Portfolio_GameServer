#pragma once

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


namespace TetrisServerLib {
	enum class MQ_GAMT_DATA_ID
	{
		REQ_SERVER_INFO = 301,
		RES_SERVER_INFO = 302,

		NTF_MATCHING_INFO = 306,

		NTF_UN_USED_ROOM = 308,		
	};



	const int MAX_MQ_SENDER_LEN = 8;

	struct MQBinaryHead
	{
		UINT16 ID = 0;
		char Sender[MAX_MQ_SENDER_LEN] = { 0, };

		int HeadDecode(char* buffer)
		{
			auto pos = 0;

			ID = ReadUInt16(buffer, pos);
			pos += 2;

			auto len = ReadUInt16(buffer, pos);
			pos += 2;

			ReadBuf(buffer, pos, len, Sender);
			pos += len;

			return pos;
		}

		int HeadEncode(char* buffer)
		{
			auto pos = 0;

			WriteUInt16(buffer, pos, ID);
			pos += 2;

			auto senderByteLen = (UINT16)strlen(Sender);
			WriteUInt16(buffer, pos, senderByteLen);
			pos += 2;

			WriteBuf(buffer, pos, senderByteLen, Sender);
			pos += senderByteLen;

			return pos;
		}


		void SetHeader(const UINT16 id, const INT32 serverIndex)
		{
			ID = id;
			sprintf_s(Sender, MAX_MQ_SENDER_LEN, "GA%d", serverIndex);
		}

	
		INT16 ReadInt16(char* pData, const INT32 pos)
		{
			INT16 value;
			memcpy(&value, pData + pos, 2);
			return value;
		}
		void WriteInt16(char* pData, const INT32 pos, const INT16 value)
		{
			memcpy(pData + pos, &value, 2);
		}

		UINT16 ReadUInt16(char* pData, const INT32 pos)
		{
			UINT16 value;
			memcpy(&value, pData+pos, 2);
			return value;
		}
		void WriteUInt16(char* pData, const INT32 pos, const UINT16 value)
		{
			memcpy(pData + pos, &value, 2);			
		}

		INT32 ReadInt32(char* pData, const INT32 pos)
		{
			INT32 value;
			memcpy(&value, pData + pos, 4);
			return value;
		}
		void WriteInt32(char* pData, const INT32 pos, const INT32 value)
		{
			memcpy(pData + pos, &value, 4);
		}

		void ReadBuf(char* pData, const INT32 pos, const INT32 length, char* pOutBuffer)
		{
			UINT16 value;
			memcpy(pOutBuffer, pData + pos, length);
		}
		void WriteBuf(char* pData, const INT32 pos, const INT32 length, char* pOutBuffer)
		{
			memcpy(pData + pos, pOutBuffer, length);
		}
	};


	struct MQReqGAServerInfo : MQBinaryHead
	{
		INT32 StartRoomNumber = -1;
		INT32 MaxRoomCount = 0;
		char ServerIP[32] = { 0, };
		UINT16 ServerPort = 0;

		INT32 Encode(char* buffer)
		{
			auto pos = HeadEncode(buffer);

			WriteInt32(buffer, pos, StartRoomNumber);
			pos += 4;

			WriteInt32(buffer, pos, MaxRoomCount);
			pos += 4;

			auto ipLen = (UINT16)strlen(ServerIP);
			WriteUInt16(buffer, pos, ipLen);
			pos += 2;

			WriteBuf(buffer, pos, ipLen, ServerIP);
			pos += ipLen;

			WriteUInt16(buffer, pos, ServerPort);
			pos += 2;

			return pos;
		}
	};

	struct MQResGAServerInfo : MQBinaryHead
	{
		INT16 Result = 0;

		void Decode(char* buffer)
		{
			auto pos = HeadDecode(buffer);

			Result = ReadInt16(buffer, pos);
			pos += 2;
		}
	};


	struct MQNtfGAMatchingInfo : MQBinaryHead
	{
		char UserID1[32] = { 0, };
		char UserID2[32] = { 0, };
		INT32 RoomNumber = 0;
				
		void Decode(char* buffer)
		{
			auto pos = HeadDecode(buffer);

			auto len = ReadUInt16(buffer, pos);
			pos += 2;
			ReadBuf(buffer, pos, len, UserID1);
			pos += len;

			len = ReadUInt16(buffer, pos);
			pos += 2;
			ReadBuf(buffer, pos, len, UserID2);
			pos += len;

			RoomNumber = ReadInt32(buffer, pos);
			pos += 4;
		}
	};


	struct MQNtfGAUnUsedRoom : MQBinaryHead
	{
		INT32 RoomNumber = -1;
		
		INT32 Encode(char* buffer)
		{
			auto pos = HeadEncode(buffer);

			WriteInt32(buffer, pos, RoomNumber);
			pos += 4;

			return pos;
		}
	};
}