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
	/// <发送>
	void Send();
	/// <设置回调>
	void SetCallBack();
	/// <重连>
	void ReConnect();
};