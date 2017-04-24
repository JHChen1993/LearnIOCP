#include "stdafx.h"
#include "JHSocketBase.h"
#pragma comment(lib,"ws2_32.lib")
#define SELECT_TIMEOUT 15
#define MAXBUF_LEN 1024*10
#define IF_MESSAGEBOX(a,b) if(a) ::MessageBox(NULL,_T(b),_T("错误:"),MB_OK)

JHSocketBase::JHSocketBase(const char* ip, unsigned short nPort)
	: _port(nPort), _is_valid(false)
{
	strcpy_s(_ip, ip);
	_is_valid = _JHCreateSocket(&_mSocket);
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
bool JHSocketBase::_JHCreateSocket(SOCKET* pSocket)
{
	SOCKET hSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hSock == INVALID_SOCKET){
		return false;
	}
	*pSocket = hSock;
	return _JHConnect(_ip, _port);
}
bool JHSocketBase::_JHConnect(char* ip, unsigned short nPort)
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = inet_addr(ip);
	/// <connect>
	for (int i = 0; i < 5; ++i){
		int nRet = connect(_mSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
		if (nRet == 0){
			return true;
		}
		Sleep(1000);
	}
	if (::MessageBox(NULL, _T("连接失败，是否重试？"), _T("网络错误"), MB_OKCANCEL) == IDOK){
		return _JHConnect(ip, nPort);
	}
	return false;
}
/// <发送>
void JHSocketBase::Send(const PackInfo& pack_dt)
{
	/// <socket无效 通知回调，执行重连等操作>
	if (!_is_valid){
		_NotiySocketError();
		return;
	}
	/// <直接发送>
	_send_buf.push_back(pack_dt);
}
/// <重连>
void JHSocketBase::_ReConnect()
{
	_is_valid = _JHCreateSocket(&_mSocket);
}
bool JHSocketBase::_HasSendData()
{
	return _send_buf.empty();
}

void JHSocketBase::_SendHeartAlive()
{
	/// <主子码为0>
	Send(PackInfo());
}
void JHSocketBase::_ClearupPackInfo()
{
	DWORD nTickCount = ::GetTickCount();
	for (auto pack : _tmout_buf){
		if (nTickCount - pack.tickcount > 1000 * 15){
			_NotiyPackTimeout(pack.mcode, pack.ccode);
			_tmout_buf.remove(pack);
		}
	}
}
void JHSocketBase::_NotiySocketError()
{
	_is_valid = false;
	if (_mSocket != INVALID_SOCKET){
		closesocket(_mSocket);
		_mSocket = INVALID_SOCKET;
	}
	/// <TODO:获取SOCKET EROOR>
	/// <回调>
	PackInfo noty_pack;
	_func_cb(cbt_err,noty_pack);
}
void JHSocketBase::_NotiyPackTimeout(char mcode, char ccode)
{
	PackInfo noty_pack;
	noty_pack.mainCode = mcode;
	noty_pack.chilCode = ccode;
	_func_cb(cbt_tot, noty_pack);
}
void JHSocketBase::_NotiyPackRet(const PackInfo& pack_ret)
{
	_func_cb(cbt_ret, pack_ret);
}
bool JHSocketBase::_Read()
{
	char recv_buf[MAXBUF_LEN] = { 0 };
	int nRet = recv(_mSocket, recv_buf, MAXBUF_LEN, 0);
	if (nRet <= 0){ /// <0 或者SOCKET_ERROR>
		return false;
	}
	/// <TODO:buf放入队列>
	return true;
}
bool JHSocketBase::_Write()
{
	do{
		auto sd_pack = _send_buf.front();
		_send_buf.pop_front();
		/// <send>
		PackHead theHead;
		/// <心跳包>
		if (sd_pack.mainCode == 0 && sd_pack.chilCode == 0){
			int nRet = send(_mSocket, (char*)&theHead, theHead.nlen, 0);
			if (nRet != theHead.nlen){
				return false;
			}
			break;
		}
		/// <准备数据>
		theHead.mcode = sd_pack.mainCode;
		theHead.ccode = sd_pack.chilCode;
		theHead.nlen = sd_pack.nLen + PACKHEADLEN;
		_ASSERT(theHead.nlen < 4096);
		char* pData = new char[theHead.nlen];
		memcpy(pData, &theHead, PACKHEADLEN);
		memcpy(pData + PACKHEADLEN, sd_pack.pdata, sd_pack.nLen);
		int nRet = send(_mSocket, pData, theHead.nlen, 0);
		if (nRet != theHead.nlen){
			return false;
		}
		///<销毁对象>
		delete[] pData;
		delete[] sd_pack.pdata;
		/// <放入_tmout_buf>
		SDPackTimeout tmot_pack;
		tmot_pack.mcode = sd_pack.mainCode;
		tmot_pack.ccode = sd_pack.chilCode;
		tmot_pack.tickcount = ::GetTickCount();
		_tmout_buf.push_back(tmot_pack);
	} while (_HasSendData());
	return true;
}
//=============================================================================================
JHClientSocket::JHClientSocket(const char* ip, unsigned short nPort)
	:JHSocketBase(ip,nPort)
{
	_sockio_hand = ::CreateThread(NULL, NULL, SocketIOProc, this, 0, NULL);
	_custom_hand = ::CreateThread(NULL, NULL, SocketIOProc, this, 0, NULL);
}
JHClientSocket::~JHClientSocket()
{
	_ThreadClose();
}
void JHClientSocket::_ThreadClose()
{
	/// <退出线程>
	if (_is_valid){
		_is_valid = false;
	}
	if (_custom_hand != INVALID_HANDLE_VALUE){
		WaitForSingleObject(_custom_hand, INFINITE);
		CloseHandle(_custom_hand);
		_custom_hand = INVALID_HANDLE_VALUE;
	}
	if (_sockio_hand != INVALID_HANDLE_VALUE){
		WaitForSingleObject(_sockio_hand, INFINITE);
		CloseHandle(_sockio_hand);
		_sockio_hand = INVALID_HANDLE_VALUE;
	}
}

void JHClientSocket::ReConnect()
{
	_ThreadClose();
	__super::_ReConnect();
	if (_is_valid){
		_sockio_hand = ::CreateThread(NULL, NULL, SocketIOProc, this, 0, NULL);
		_custom_hand = ::CreateThread(NULL, NULL, SocketIOProc, this, 0, NULL);
	}
}
DWORD CALLBACK JHClientSocket::SocketIOProc(LPVOID pParam)
{
	auto pClientSock = (JHClientSocket*)pParam;
	SOCKET hSocket = pClientSock->_mSocket;
	timeval time_out;
	time_out.tv_sec = 0;
	time_out.tv_usec = SELECT_TIMEOUT;
	while (pClientSock->_is_valid){
		/// <fd 设置>
		fd_set fd_read, fd_err;
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_err);
		FD_SET(hSocket, &fd_read);
		/// <select 可读或超时>
		int nRet = select(0, &fd_read, NULL, &fd_err,&time_out);
		if (nRet == SOCKET_ERROR){
			pClientSock->_NotiySocketError();
			continue;
		}
		/// <超时>
		if (nRet == 0){
			/// <>
			if (!pClientSock->_HasSendData()){
				time_out.tv_usec = 15;
				continue;
			}
			/// <有可写数据>
			time_out.tv_usec = 0;
			if (!pClientSock->_Write()){
				pClientSock->_NotiySocketError();
				continue;
			}
		}
		/// <read error>
		if (nRet > 0){
			/// <socket 出错>
			if (FD_ISSET(hSocket, &fd_err)){
				pClientSock->_NotiySocketError();
				continue;
			}
			/// <socket 可读>
			if (FD_ISSET(hSocket, &fd_read) && !pClientSock->_Read()){
				pClientSock->_NotiySocketError();
				continue;
			}
			/// <>
			if (!pClientSock->_HasSendData()){
				time_out.tv_usec = 15;
				continue;
			}
			/// <有可写数据>
			time_out.tv_usec = 0;
			if (!pClientSock->_Write()){
				pClientSock->_NotiySocketError();
				continue;
			}
		}
	}
	return 0;
}
DWORD CALLBACK JHClientSocket::CustomerProc(LPVOID pParam)
{
	auto pClientSock = (JHClientSocket*)pParam;
	DWORD nTickCount = ::GetTickCount();
	while (pClientSock->_is_valid){
		bool need_sleep = true;
		/// <TODO:解析recv数据，并发送回调消息>
		/// <计时器 发送心跳消息>
		if (::GetTickCount() - nTickCount > 1000 * 5){
			pClientSock->_SendHeartAlive();
		}
		/// <查看send包是否未收到数据超时>
		pClientSock->_ClearupPackInfo();
		if (need_sleep){
			Sleep(15);
		}
	}
	return 0;
}
//=============================================================================================
JHUploadSoket::JHUploadSoket(const char* ip, unsigned short nPort) :JHSocketBase(ip,nPort)
{
}

JHUploadSoket::~JHUploadSoket()
{
}