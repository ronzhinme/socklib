#pragma once

#include <WS2tcpip.h>

class SocketBase
{
protected:
  SOCKET sock_;
  ADDRINFO *resultAddr_;
  char recvBuff_[SO_MAX_MSG_SIZE];

  int SendData(SOCKET destSock, const char* data, size_t dataLen);
public:
  SocketBase();
  virtual ~SocketBase();
  virtual bool Open(unsigned short port, const char* ip = NULL) = 0;
  virtual const char* Recv(SOCKET & srcSock, size_t & dataLen) = 0;
};

class TCPSocketSrv : public SocketBase
{
private:
  fd_set s_read_;
  static void AcceptThread(void*);
public:
  TCPSocketSrv();
  ~TCPSocketSrv();
  bool Open(unsigned short port, const char* ip = NULL);
  int Send(SOCKET destSock, const char* data, size_t dataLen);
  const char* Recv(SOCKET & srcSock, size_t & dataLen);
};

class TCPSocketClt : public SocketBase
{
public:
  TCPSocketClt();
  ~TCPSocketClt();
  bool Open(unsigned short port, const char* ip = NULL);
  int Send(const char* data, size_t dataLen);
  const char* Recv(SOCKET & srcSock, size_t & dataLen);
};