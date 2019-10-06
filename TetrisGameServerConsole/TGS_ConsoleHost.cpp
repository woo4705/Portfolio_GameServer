// TGS_ConsoleHost.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"mswsock.lib")

#include <iostream>

#include "../TetrisServerLib/flags.h"
#include "../TetrisServerLib/Main.h"


TetrisServerLib::GameServerConfig ParseConfig(int argc, char* argv[]);
bool ParseMQConfig(int argc, char* argv[], TetrisServerLib::GameServerConfig* gameServerConfig);


int main(int argc, char* argv[])
{
	//NetLib::NetConfig netConfig;
	
	//netConfig.m_PortNumber = 11020; //bat파일로 받아들이기
	//netConfig.m_WorkThreadCount = 4;
	//netConfig.m_MaxRecvOverlappedBufferSize = 600; //RingBuffer크기와 패킷버퍼 크기가 같아서 약간의 크기만 수신받아도 오류걸려서 임의로 수정함
	//netConfig.m_MaxSendOverlappedBufferSize = 600;
	//netConfig.m_MaxRecvConnectionBufferCount = 1024;
	//netConfig.m_MaxSendConnectionBufferCount = 1024;
	//netConfig.m_MaxPacketSize = 1024;
	//netConfig.m_MaxConnectionCount = 1024;
	//netConfig.m_MaxMessagePoolCount = 1024;
	//netConfig.m_ExtraMessagePoolCount = 128;
	//netConfig.m_PerformancePacketMillisecondsTime = 0;
	//netConfig.m_PostMessagesThreadsCount = 1;
	
	auto serverConfig = ParseConfig(argc, argv);
	//InitMq()
	NetLib::LogFuncPtr = [](const int eLogInfoType, const char* szLogMsg) {
		UNREFERENCED_PARAMETER(eLogInfoType);
		printf("%s\r\n", szLogMsg);
	};

	TetrisServerLib::Main main;
	main.Init(serverConfig);

	std::thread logicThread([&]() {
		main.Run(); }
	);

	
	std::cout << "Server quits when you press (quit) !!!" << std::endl;

	while (true)
	{
		std::string inputCmd;
		std::getline(std::cin, inputCmd);
		
		if (inputCmd == "quit")
		{
			break;
		}
	}

	//std::cout << "press any key to exit...";
	//getchar();

	main.Stop();
	logicThread.join();

    std::cout << "Hello World!\n";

	//Main Thread를 만들고 실행
}


TetrisServerLib::GameServerConfig ParseConfig(int argc, char* argv[]) 
{
	auto serverConfig = TetrisServerLib::GameServerConfig();

	const flags::args args(argc, argv);
	const auto port = args.get<UINT16>("port"); 
	if (!port) { 
		std::cerr << "No Port. :(" << std::endl;
		return serverConfig;
	} 
	std::cout << "Port: " << *port << std::endl;


	const auto WorkThreadCount = args.get<INT32>("WorkThreadCount");
	if (!WorkThreadCount) {
		std::cerr << "No WorkThreadCount. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "WorkThreadCount: " << *WorkThreadCount << std::endl;


	const auto MaxRecvOverlappedBufferSize = args.get<INT32>("MaxRecvOverlappedBufferSize");
	if (!MaxRecvOverlappedBufferSize) {
		std::cerr << "No MaxRecvOverlappedBufferSize. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxRecvOverlappedBufferSize: " << *MaxRecvOverlappedBufferSize << std::endl;


	const auto MaxSendOverlappedBufferSize = args.get<INT32>("MaxSendOverlappedBufferSize");
	if (!MaxSendOverlappedBufferSize) {
		std::cerr << "No MaxSendOverlappedBufferSize. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxSendOverlappedBufferSize: " << *MaxSendOverlappedBufferSize << std::endl;


	const auto MaxRecvConnectionBufferCount = args.get<INT32>("MaxRecvConnectionBufferCount");
	if (!MaxRecvConnectionBufferCount) {
		std::cerr << "No MaxRecvConnectionBufferCount. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxRecvConnectionBufferCount: " << *MaxRecvConnectionBufferCount << std::endl;


	const auto MaxSendConnectionBufferCount = args.get<INT32>("MaxSendConnectionBufferCount");
	if (!MaxSendConnectionBufferCount) {
		std::cerr << "No MaxSendConnectionBufferCount. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxSendConnectionBufferCount: " << *MaxSendConnectionBufferCount << std::endl;


	const auto MaxPacketSize = args.get<INT32>("MaxPacketSize");
	if (!MaxPacketSize) {
		std::cerr << "No MaxPacketSize. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxSendConnectionBufferCount: " << *MaxSendConnectionBufferCount << std::endl;


	const auto MaxConnectionCount = args.get<INT32>("MaxConnectionCount");
	if (!MaxSendConnectionBufferCount) {
		std::cerr << "No MaxConnectionCount. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxConnectionCount: " << *MaxConnectionCount << std::endl;


	const auto MaxMessagePoolCount = args.get<INT32>("MaxMessagePoolCount");
	if (!MaxMessagePoolCount) {
		std::cerr << "No MaxMessagePoolCount. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "MaxMessagePoolCount: " << *MaxMessagePoolCount << std::endl;


	const auto ExtraMessagePoolCount = args.get<INT32>("ExtraMessagePoolCount");
	if (!ExtraMessagePoolCount) {
		std::cerr << "No ExtraMessagePoolCount. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "ExtraMessagePoolCount: " << *ExtraMessagePoolCount << std::endl;


	if (ParseMQConfig(argc, argv, &serverConfig) == false) {
		return serverConfig;
	}


	const auto MyIPAddress = args.get <std::string>("MyIPAddress");
	if (!MyIPAddress) {
		std::cerr << "No MyIPAddress. :(" << std::endl;
		return serverConfig;
	}
	std::cout << "No MyIPAddress: " << *MyIPAddress << std::endl;

	serverConfig.m_PortNumber = *port;
	serverConfig.m_WorkThreadCount = *WorkThreadCount;
	serverConfig.m_MaxRecvOverlappedBufferSize = *MaxRecvOverlappedBufferSize;
	serverConfig.m_MaxSendOverlappedBufferSize = *MaxSendOverlappedBufferSize;
	serverConfig.m_MaxRecvConnectionBufferCount = *MaxRecvConnectionBufferCount;
	serverConfig.m_MaxSendConnectionBufferCount = *MaxSendConnectionBufferCount;
	serverConfig.m_MaxPacketSize = *MaxPacketSize;
	serverConfig.m_MaxConnectionCount = *MaxConnectionCount;
	serverConfig.m_MaxMessagePoolCount = *MaxMessagePoolCount;
	serverConfig.m_ExtraMessagePoolCount = *ExtraMessagePoolCount;
	serverConfig.m_PerformancePacketMillisecondsTime = 0;
	serverConfig.m_PostMessagesThreadsCount = 1;

	serverConfig.m_MyIPAddress = *MyIPAddress;

	return serverConfig;
}



bool ParseMQConfig(int argc, char* argv[], TetrisServerLib::GameServerConfig* gameServerConfig) {
	//gameServerConfig.MqConfig.Hostname
	
	TetrisServerLib::MqConfig* mqConfig = &(gameServerConfig->MqConfig);
	
	const flags::args args(argc, argv);
	const auto HostName = args.get<std::string>("MQHostname");
	if (!HostName) {
		std::cerr << "No MQHostname. :(" << std::endl;
		return false;
	}
	std::cout << "MQHostname: " << *HostName << std::endl;


	const auto Port = args.get<INT32>("MQPort");
	if (!Port) {
		std::cerr << "No MQPort. :(" << std::endl;
		return false;
	}
	std::cout << "MQPort: " << *Port << std::endl;


	const auto UserID = args.get<std::string>("MQUserID");
	if (!UserID) {
		std::cerr << "No MQUserID. :(" << std::endl;
		return false;
	}
	std::cout << "MQUserID: " << *UserID << std::endl;


	const auto UserPW = args.get<std::string>("MQUserPW");
	if (!UserPW) {
		std::cerr << "No MQUserPW. :(" << std::endl;
		return false;
	}
	std::cout << "MQUserPW: " << *UserPW << std::endl;


	const auto VHost = args.get<std::string>("VHost");
	if (!VHost) {
		std::cerr << "No VHost. :(" << std::endl;
		return false;
	}
	std::cout << "VHost: " << *VHost << std::endl;


	const auto SendQueueName = args.get<std::string>("SendQueueName");
	if (!SendQueueName) {
		std::cerr << "No SendQueueName. :(" << std::endl;
		return false;
	}
	std::cout << "SendQueueName: " << *SendQueueName << std::endl;


	const auto ReceiveQueueName = args.get<std::string>("ReceiveQueueName");
	if (!ReceiveQueueName) {
		std::cerr << "No ReceiveQueueName. :(" << std::endl;
		return false;
	}
	std::cout << "ReceiveQueueName: " << *ReceiveQueueName << std::endl;


	const auto MQStartRoomNum = args.get<INT32>("MQStartRoomNum");
	if (!MQStartRoomNum) {
		std::cerr << "No MQStartRoomNum. :(" << std::endl;
		return false;
	}
	std::cout << "MQStartRoomNum: " << *MQStartRoomNum << std::endl;


	const auto MQMaxRoomCnt = args.get<INT32>("MQMaxRoomCnt");
	if (!MQMaxRoomCnt) {
		std::cerr << "No MQMaxRoomCnt. :(" << std::endl;
		return false;
	}
	std::cout << "MQMaxRoomCnt: " << *MQMaxRoomCnt << std::endl;


	mqConfig->Hostname = *HostName;
	mqConfig->Port = *Port;
	mqConfig->UserID = *UserID;
	mqConfig->UserPW = *UserPW;
	mqConfig->VHost = *VHost;
	mqConfig->SendToMatchQueueName = *SendQueueName;
	mqConfig->ReceivFromMatchQueueName = *ReceiveQueueName;

	mqConfig->MQStartRoomNum = *MQStartRoomNum;
	mqConfig->MQMaxRoomCnt = *MQMaxRoomCnt;

	return true;

}