#pragma once
//==================================================================================================================
// 公司：北京速帮网络技术有限公司
// 作者：cjh
// 时间：2017.04.23
// 描述：封装Socket通信类，用于传版项目
//==================================================================================================================

#include <WinSock2.h>
#include <windows.h>
#include <functional>
#include <list>
#define PACKHEADLEN 8

/// <回调消息类型>
enum CallBackType
{
	cbt_ret, /// <send 服务器返回>
	cbt_tot, /// <send 超时服务器未返回>
	cbt_err  /// <网络错误>
};

#pragma pack(push)
#pragma pack(1)
/// <发送/回调包信息>
struct PackInfo{
	PackInfo() :mainCode(0), chilCode(0), nLen(0), pdata(0){}
	char mainCode;
	char chilCode;
	unsigned int nLen;
	void* pdata;
};
/// <包头>
struct PackHead
{
	PackHead() :syns1(0x33), syns2(0x44), mcode(0), ccode(0), nlen(PACKHEADLEN){}
	char syns1;
	char syns2;
	char mcode;
	char ccode;
	unsigned int nlen;
};
/// <已发送包信息>
struct SDPackTimeout
{
	bool operator== (const SDPackTimeout& tmpack){ return this->mcode == tmpack.mcode && this->ccode == tmpack.ccode; }
	char mcode;
	char ccode;
	DWORD tickcount;
};
#pragma  pack(pop)

//====================================================socket 封装基类=============================================================
// InitSocket、ExitInitSocket 分别在程序启动和结尾时候调用一次即可，初始化套接字
// IsValid 用于判断套接字是否有效
// SetCallBack 设置套接字接收数据之后的回调函数
// Send 发送数据
//================================================================================================================================
class JHSocketBase
{
public:
	JHSocketBase(const char* ip,unsigned short nPort);
	~JHSocketBase();
	/// <初始化>
	static void InitSocket();
	static void ExitInitSocket();
	/// <有效的对象>
	bool IsValid(){ return _is_valid; }
	/// <设置回调>
	void SetCallBack(const std::function<void(CallBackType, PackInfo)>& func_cb){ _func_cb = func_cb; };
	/// <发送>
	void Send(const PackInfo& pack_dt);
protected:
	/// <创建套接字并连接>
	bool _JHCreateSocket(SOCKET* pSocket);
	/// <连接>
	bool _JHConnect(char* ip, unsigned short nPort);
	/// <重连>
	void _ReConnect();
	/// <判断是否有发送数据>
	bool _HasSendData();
	/// <发送心跳包>
	void _SendHeartAlive();
	/// <查看是否有发送的包未返回>
	void _ClearupPackInfo();
	/// <通知 网络错误,且关闭套接字>
	void _NotiySocketError();
	/// <通知 发送包超时>
	void _NotiyPackTimeout(char mcode,char ccode);
	/// <通知 数据包返回>
	void _NotiyPackRet(const PackInfo& pack_ret);
	/// <接收数据>
	bool _Read();
	/// <发送数据>
	bool _Write();
protected:
	char _ip[32]; /// <ip>
	unsigned short _port; /// <端口>
	SOCKET _mSocket; /// <socket>
	bool _is_valid; /// <是否有效>
	std::list<PackInfo> _send_buf; /// <发送缓冲>
	std::list<SDPackTimeout> _tmout_buf; /// <发送协议 是否返回判断>
	std::function<void(CallBackType, const PackInfo&)> _func_cb; /// <回调函数>
};

//================================================处理客户端逻辑的接收和数据解析==================================================
// 函数ReConnect 重定义了重连函数，需要等待原有线程关闭并创建socket和线程
// 
//================================================================================================================================
class JHClientSocket :public JHSocketBase{
public:
	JHClientSocket(const char* ip, unsigned short nPort);
	~JHClientSocket();
	/// <重连>
	void ReConnect();
private:
	/// <发送 接收数据>
	static DWORD CALLBACK SocketIOProc(LPVOID pParam); 
	/// <解析数据 处理send超时问题>
	static DWORD CALLBACK CustomerProc(LPVOID pParam); 
	/// <关闭线程，做清理操作>
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
