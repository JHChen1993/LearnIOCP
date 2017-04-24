#pragma once
//==================================================================================================================
// ��˾�������ٰ����缼�����޹�˾
// ���ߣ�cjh
// ʱ�䣺2017.04.23
// ��������װSocketͨ���࣬���ڴ�����Ŀ
//==================================================================================================================

#include <WinSock2.h>
#include <windows.h>
#include <functional>
#include <list>
#define PACKHEADLEN 8

/// <�ص���Ϣ����>
enum CallBackType
{
	cbt_ret, /// <send ����������>
	cbt_tot, /// <send ��ʱ������δ����>
	cbt_err  /// <�������>
};

#pragma pack(push)
#pragma pack(1)
/// <����/�ص�����Ϣ>
struct PackInfo{
	PackInfo() :mainCode(0), chilCode(0), nLen(0), pdata(0){}
	char mainCode;
	char chilCode;
	unsigned int nLen;
	void* pdata;
};
/// <��ͷ>
struct PackHead
{
	PackHead() :syns1(0x33), syns2(0x44), mcode(0), ccode(0), nlen(PACKHEADLEN){}
	char syns1;
	char syns2;
	char mcode;
	char ccode;
	unsigned int nlen;
};
/// <�ѷ��Ͱ���Ϣ>
struct SDPackTimeout
{
	bool operator== (const SDPackTimeout& tmpack){ return this->mcode == tmpack.mcode && this->ccode == tmpack.ccode; }
	char mcode;
	char ccode;
	DWORD tickcount;
};
#pragma  pack(pop)

//====================================================socket ��װ����=============================================================
// InitSocket��ExitInitSocket �ֱ��ڳ��������ͽ�βʱ�����һ�μ��ɣ���ʼ���׽���
// IsValid �����ж��׽����Ƿ���Ч
// SetCallBack �����׽��ֽ�������֮��Ļص�����
// Send ��������
//================================================================================================================================
class JHSocketBase
{
public:
	JHSocketBase(const char* ip,unsigned short nPort);
	~JHSocketBase();
	/// <��ʼ��>
	static void InitSocket();
	static void ExitInitSocket();
	/// <��Ч�Ķ���>
	bool IsValid(){ return _is_valid; }
	/// <���ûص�>
	void SetCallBack(const std::function<void(CallBackType, PackInfo)>& func_cb){ _func_cb = func_cb; };
	/// <����>
	void Send(const PackInfo& pack_dt);
protected:
	/// <�����׽��ֲ�����>
	bool _JHCreateSocket(SOCKET* pSocket);
	/// <����>
	bool _JHConnect(char* ip, unsigned short nPort);
	/// <����>
	void _ReConnect();
	/// <�ж��Ƿ��з�������>
	bool _HasSendData();
	/// <����������>
	void _SendHeartAlive();
	/// <�鿴�Ƿ��з��͵İ�δ����>
	void _ClearupPackInfo();
	/// <֪ͨ �������,�ҹر��׽���>
	void _NotiySocketError();
	/// <֪ͨ ���Ͱ���ʱ>
	void _NotiyPackTimeout(char mcode,char ccode);
	/// <֪ͨ ���ݰ�����>
	void _NotiyPackRet(const PackInfo& pack_ret);
	/// <��������>
	bool _Read();
	/// <��������>
	bool _Write();
protected:
	char _ip[32]; /// <ip>
	unsigned short _port; /// <�˿�>
	SOCKET _mSocket; /// <socket>
	bool _is_valid; /// <�Ƿ���Ч>
	std::list<PackInfo> _send_buf; /// <���ͻ���>
	std::list<SDPackTimeout> _tmout_buf; /// <����Э�� �Ƿ񷵻��ж�>
	std::function<void(CallBackType, const PackInfo&)> _func_cb; /// <�ص�����>
};

//================================================����ͻ����߼��Ľ��պ����ݽ���==================================================
// ����ReConnect �ض�����������������Ҫ�ȴ�ԭ���̹߳رղ�����socket���߳�
// 
//================================================================================================================================
class JHClientSocket :public JHSocketBase{
public:
	JHClientSocket(const char* ip, unsigned short nPort);
	~JHClientSocket();
	/// <����>
	void ReConnect();
private:
	/// <���� ��������>
	static DWORD CALLBACK SocketIOProc(LPVOID pParam); 
	/// <�������� ����send��ʱ����>
	static DWORD CALLBACK CustomerProc(LPVOID pParam); 
	/// <�ر��̣߳����������>
	void _ThreadClose();
private:
	HANDLE _sockio_hand;
	HANDLE _custom_hand;
};
class JHUploadSoket :public JHSocketBase
{
public:
	JHUploadSoket(const char* ip, unsigned short nPort);
	~JHUploadSoket();

private:

};
