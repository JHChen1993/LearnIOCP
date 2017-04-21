#pragma once
#include <WinSock2.h>
class JHSocketBase
{
public:
	JHSocketBase();
	~JHSocketBase();
	void InitSocket();
	void ExitInitSocket();
	bool JHCreateSocket(char* ip,unsigned short nPort,SOCKET* pSocket);
protected:
	SOCKET m_hSocket;
};

class JHClientSocket :public JHSocketBase{
public:
	JHClientSocket();
	~JHClientSocket();
	/// <����>
	void Send();
	/// <���ûص�>
	void SetCallBack();
	/// <����>
	void ReConnect();
};