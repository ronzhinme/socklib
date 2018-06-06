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
  WSACleanup();
}

void SocketBase::Close()
{
  shutdown(sock_, SD_BOTH);
  closesocket(sock_);
}

TCPSocketSrv::TCPSocketSrv()
{
}

TCPSocketSrv::~TCPSocketSrv()
{
}

bool TCPSocketSrv::Open(unsigned short port, const char* ip)
{
  ADDRINFO *resultAddr;
  ADDRINFO localaddr_;
  ZeroMemory(&localaddr_, sizeof(localaddr_));
  localaddr_.ai_family = AF_INET;
  localaddr_.ai_socktype = SOCK_STREAM;
  localaddr_.ai_protocol = IPPROTO_TCP;
  localaddr_.ai_flags = AI_PASSIVE;

  char portValue[6];
  _itoa_s(port, portValue, 10);

  getaddrinfo(NULL, portValue, &localaddr_, &resultAddr);

  sock_ = socket(resultAddr->ai_family, resultAddr->ai_socktype, resultAddr->ai_protocol);
  if (sock_ == INVALID_SOCKET)
    return false;

  u_long nonBlockMode = 1;
  if (ioctlsocket(sock_, FIONBIO, &nonBlockMode) != NO_ERROR)
    return false;

  if (bind(sock_, resultAddr->ai_addr, (int)resultAddr->ai_addrlen) == SOCKET_ERROR)
    return false;

  if (listen(sock_, SOMAXCONN) == SOCKET_ERROR)
    return false;

  freeaddrinfo(resultAddr);

  FD_ZERO(&s_read_);
  FD_SET(sock_, &s_read_);

  return true;
}

int TCPSocketSrv::Send(SOCKET destSock, const char * data, size_t dataLen)
{
  return SendData(destSock, data, dataLen);
}

const char * TCPSocketSrv::Recv(SOCKET & srcSock, size_t & dataLen)
{
  memset((void*)&recvBuff_[0], 0, SO_MAX_MSG_SIZE);
  dataLen = 0;
  srcSock = INVALID_SOCKET;

  Sleep(1);
  while (auto sock = accept(sock_, NULL, NULL))
  {
    if (sock == INVALID_SOCKET)
      break;
    FD_SET(sock, &s_read_);
  }
  fd_set fdset = s_read_;

  timeval tim;
  tim.tv_sec = 1;
  auto sockCount = select(0, &fdset, NULL, NULL, &tim);
  if (sockCount <= 0)
    return recvBuff_;

  for (auto sock : fdset.fd_array)
    if (FD_ISSET(sock, &fdset))
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
  ADDRINFO *resultAddr;
  ADDRINFO localaddr_;
  ZeroMemory(&localaddr_, sizeof(localaddr_));
  localaddr_.ai_family = AF_INET;
  localaddr_.ai_socktype = SOCK_STREAM;
  localaddr_.ai_protocol = IPPROTO_TCP;

  char portValue[6];
  _itoa_s(port, portValue, 10);

  getaddrinfo(NULL, portValue, &localaddr_, &resultAddr);

  for (auto ptr = resultAddr; ptr != NULL; ptr = ptr->ai_next) {

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
  freeaddrinfo(resultAddr);
  return true;
}

int TCPSocketClt::Send(const char * data, size_t dataLen)
{
  return SendData(sock_, data, dataLen);
}

const char * TCPSocketClt::Recv(SOCKET & srcSock, size_t & dataLen)
{
  memset((void*)&recvBuff_[0], 0, SO_MAX_MSG_SIZE);
  dataLen = 0;
  srcSock = INVALID_SOCKET;

  Sleep(1);
  while (auto sock = accept(sock_, NULL, NULL))
  {
    if (sock == INVALID_SOCKET)
      break;
    FD_SET(sock, &s_read_);
  }
  fd_set fdset = s_read_;

  timeval tim;
  tim.tv_sec = 1;
  auto sockCount = select(0, &fdset, NULL, NULL, &tim);
  if (sockCount <= 0)
    return recvBuff_;

  for (auto sock : fdset.fd_array)
    if (FD_ISSET(sock, &fdset))
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
