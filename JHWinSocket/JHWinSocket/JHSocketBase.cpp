#include "stdafx.h"
#include "JHSocketBase.h"
#pragma comment(lib,"ws2_32.lib")

JHSocketBase::JHSocketBase()
{
}

JHSocketBase::~JHSocketBase()
{
}

void JHSocketBase::InitSocket()
{
	WSAData wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
}
void JHSocketBase::ExitInitSocket()
{
	WSACleanup();
}
bool JHSocketBase::JHCreateSocket(char* ip, unsigned short nPort, SOCKET* pSocket)
{
	SOCKET hSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hSock == INVALID_SOCKET){
		return false;
	}
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = inet_addr(ip);
	/// <connect>
	for (int i = 0; i < 5; ++i){
		int nRet = connect(hSock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
		if (nRet == 0){
			break;
		}
		/// <connectÊ§°Ü>
		if (i == 4){
			closesocket(hSock);
			return false;
		}
		Sleep(1000);
	}
	*pSocket = hSock;
	return true;
}