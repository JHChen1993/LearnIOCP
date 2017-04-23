// JHWinSocket.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "JHSocketBase.h"

int _tmain(int argc, _TCHAR* argv[])
{
	JHClientSocket::InitSocket();
	JHClientSocket theScoket("192.168.0.1", 7000);
	theScoket.SetCallBack([](CallBackType type,PackInfo pkinfo){
		::MessageBox(NULL, _T("xx"), _T("caption"), MB_OK);
	});
	JHClientSocket::ExitInitSocket();
	return 0;
}

