// UseWinSocket.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WinSock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")


int _tmain(int argc, _TCHAR* argv[])
{
	WSAData wsadt;
	WSAStartup(MAKEWORD(2,2), &wsadt);

	SOCKET hsock = socket(AF_INET, SOCK_STREAM, 0);
	if (hsock == INVALID_SOCKET){
		;
	}
	LPHOSTENT hostEnty;
	char hostname[1024] = { 0 };
	gethostname(hostname, 1024);
	hostEnty = gethostbyname(hostname);
	printf("%d.%d.%d.%d",
		(hostEnty->h_addr_list[0][0] & 0x00ff),
		(hostEnty->h_addr_list[0][1] & 0x00ff),
		(hostEnty->h_addr_list[0][2] & 0x00ff),
		(hostEnty->h_addr_list[0][3] & 0x00ff));
	char* ip = inet_ntoa(*(struct in_addr *)*hostEnty->h_addr_list);
	printf("%s", ip);
	unsigned long addr = inet_addr("219.239.241.145");
	hostEnty = gethostbyaddr((char*)&addr, 4, AF_INET);
	SOCKADDR_IN addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(6588);
	addr_in.sin_addr.S_un.S_addr = INADDR_ANY;
	auto nRet = bind(hsock, (SOCKADDR*)&addr_in, sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR){
		;
	}
	listen(hsock, SOMAXCONN);
	int nLen;
	int optlen = 4;// sizeof(nLen);
	getsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)&nLen, &optlen);
	getchar();
	WSACleanup();
	return 0;
}

