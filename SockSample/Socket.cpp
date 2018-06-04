#include "stdafx.h"
#include "Socket.h"

#include <process.h>
#include <windows.h>
#include <winsock2.h>
#include <processthreadsapi.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int SocketBase::SendData(SOCKET destSock, const char * data, size_t dataLen)
{
  return send(destSock, data, dataLen, 0);
}

SocketBase::SocketBase()
{
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
}

SocketBase::~SocketBase()
{
  freeaddrinfo(resultAddr_);
  WSACleanup();
}

void SocketBase::Close()
{
  shutdown(sock_, SD_BOTH);
  closesocket(sock_);
}

void TCPSocketSrv::AcceptThread(void * arg)
{
  TCPSocketSrv* srv = (TCPSocketSrv*)arg;
  while (true)
  {
    auto sock = accept(srv->sock_, NULL, NULL);
    if (sock == INVALID_SOCKET)
      _endthread();

    FD_SET(sock, &srv->s_read_);
  }
}

TCPSocketSrv::TCPSocketSrv()
{
}

TCPSocketSrv::~TCPSocketSrv()
{
  TerminateThread(HandleAcceptThread_, 0);
}

bool TCPSocketSrv::Open(unsigned short port, const char* ip)
{
  ADDRINFO localaddr_;
  ZeroMemory(&localaddr_, sizeof(localaddr_));
  localaddr_.ai_family = AF_INET;
  localaddr_.ai_socktype = SOCK_STREAM;
  localaddr_.ai_protocol = IPPROTO_TCP;
  localaddr_.ai_flags = AI_PASSIVE;

  char portValue[6];
  _itoa_s(port, portValue, 10);

  getaddrinfo(NULL, portValue, &localaddr_, &resultAddr_);

  sock_ = socket(resultAddr_->ai_family, resultAddr_->ai_socktype, resultAddr_->ai_protocol);
  if (sock_ == INVALID_SOCKET)
    return false;

  if (bind(sock_, resultAddr_->ai_addr, (int)resultAddr_->ai_addrlen) == SOCKET_ERROR)
    return false;

  if (listen(sock_, SOMAXCONN) == SOCKET_ERROR)
    return false;

  FD_ZERO(&s_read_);

  HandleAcceptThread_ = (HANDLE)_beginthread(AcceptThread, 0, this);

  return true;
}

int TCPSocketSrv::Send(SOCKET destSock, const char * data, size_t dataLen)
{
  return SendData(destSock, data, dataLen);
}

const char * TCPSocketSrv::Recv(SOCKET & srcSock, size_t & dataLen)
{
  memset((void*)&recvBuff_[0], 0, SO_MAX_MSG_SIZE);
  timeval tim;
  tim.tv_sec = 1;
  auto sockCount = select(0, &s_read_, NULL, NULL, &tim);
  if (sockCount <= 0)
    return recvBuff_;

  for (auto sock : s_read_.fd_array)
    if (FD_ISSET(sock, &s_read_))
    {
      auto recvBytes = recv(sock, &recvBuff_[0], SO_MAX_MSG_SIZE, MSG_PEEK);
      if (recvBytes)
      {
        memset((void*)&recvBuff_[0], 0, SO_MAX_MSG_SIZE);
        recv(sock, &recvBuff_[0], recvBytes, 0);
        srcSock = sock;
        dataLen = recvBytes;
        return recvBuff_;
      }
    }
  return recvBuff_;
}

TCPSocketClt::TCPSocketClt()
{
}

TCPSocketClt::~TCPSocketClt()
{
}

bool TCPSocketClt::Open(unsigned short port, const char* ip)
{
  ADDRINFO localaddr_;
  ZeroMemory(&localaddr_, sizeof(localaddr_));
  localaddr_.ai_family = AF_INET;
  localaddr_.ai_socktype = SOCK_STREAM;
  localaddr_.ai_protocol = IPPROTO_TCP;

  char portValue[6];
  _itoa_s(port, portValue, 10);

  getaddrinfo(NULL, portValue, &localaddr_, &resultAddr_);

  for (auto ptr = resultAddr_; ptr != NULL; ptr = ptr->ai_next) {

    sock_ = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (sock_ == INVALID_SOCKET)
      return false;

    if (connect(sock_, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
    {
      sock_ = SOCKET_ERROR;
      continue;
    }

    break;
  }

  if (sock_ == SOCKET_ERROR)
    return false;

  FD_ZERO(&s_read_);
  FD_SET(sock_, &s_read_);
  return true;
}

int TCPSocketClt::Send(const char * data, size_t dataLen)
{
  return SendData(sock_, data, dataLen);
}

const char * TCPSocketClt::Recv(SOCKET & srcSock, size_t & dataLen)
{
  memset((void*)&recvBuff_[0], 0, SO_MAX_MSG_SIZE);
  timeval tim;
  tim.tv_sec = 1;
  auto sockCount = select(0, &s_read_, NULL, NULL, &tim);
  if (sockCount <= 0)
    return recvBuff_;

  auto recvBytes = recv(sock_, &recvBuff_[0], SO_MAX_MSG_SIZE, MSG_PEEK);
  if (recvBytes)
  {
    memset((void*)&recvBuff_[0], 0, SO_MAX_MSG_SIZE);
    recv(sock_, &recvBuff_[0], recvBytes, 0);
    srcSock = sock_;
    dataLen = recvBytes;
    return recvBuff_;

  }
  return recvBuff_;
}
