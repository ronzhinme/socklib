#define LINUX

#if defined(LINUX)

#include "Socket.h"

#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SO_MAX_MSG_SIZE 1024

int SocketBase::SendData(SOCKET destSock, const char *data, size_t dataLen)
{
  return send(destSock, data, dataLen, 0);
}

fd_set SocketBase::GetAcceptedSockets(SOCKET sock)
{
  while (auto acceptedSock = accept(sock, NULL, NULL))
  {
    if (acceptedSock == INVALID_SOCKET)
      break;
    FD_SET(acceptedSock, &s_read_);
    acceptedSockets_.insert(acceptedSock);
  }
  return s_read_;
}

SocketBase::SocketBase()
{
}

SocketBase::~SocketBase()
{
  Close();
}

std::map<SOCKET, BytesData> SocketBase::Recv()
{
  std::map<SOCKET, BytesData> recvData;
  char recvBuff[SO_MAX_MSG_SIZE];

  sleep(1);
  fd_set fdset = GetAcceptedSockets(sock_);

  auto sockCount = select(0, &fdset, NULL, NULL, NULL);
  if (sockCount <= 0)
    return recvData;

  for (auto sock : acceptedSockets_)
    if (FD_ISSET(sock, &fdset))
    {
      auto recvBytes = recv(sock, &recvBuff[0], SO_MAX_MSG_SIZE, MSG_PEEK);
      if (recvBytes <= 0)
        return recvData;

      memset((void *)&recvBuff[0], 0, SO_MAX_MSG_SIZE);
      recv(sock, &recvBuff[0], recvBytes, 0);
      recvData.emplace(sock, BytesData(recvBuff, recvBytes));
    }
  return recvData;
}

void SocketBase::Close()
{
  shutdown(sock_, SHUT_RDWR);
  close(sock_);
}

TCPSocketSrv::TCPSocketSrv()
{
}

TCPSocketSrv::~TCPSocketSrv()
{
}

bool TCPSocketSrv::Open(unsigned short port, const char *ip)
{
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_ < 0)
  {
    printf("Creation err\n");
    return false;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (fcntl(sock_, F_SETFL, O_NONBLOCK) <= SOCKET_ERROR)
    return false;

  int opt = 1;
  if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) <= SOCKET_ERROR)
    return false;

  if (bind(sock_, (struct sockaddr *)&addr, sizeof(addr)) <= SOCKET_ERROR)
    return false;

  if (listen(sock_, SOMAXCONN) <= SOCKET_ERROR)
    return false;

  return true;
}

int TCPSocketSrv::Send(SOCKET destSock, const char *data, size_t dataLen)
{
  return SendData(destSock, data, dataLen);
}

TCPSocketClt::TCPSocketClt()
{
}

TCPSocketClt::~TCPSocketClt()
{
}

bool TCPSocketClt::Open(unsigned short port, const char *ip)
{
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_ <= INVALID_SOCKET)
    return false;

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (ip == NULL)
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
  else
    addr.sin_addr.s_addr = inet_addr(ip);

  if (connect(sock_, (struct sockaddr *)&addr, sizeof(addr)) <= SOCKET_ERROR)
    return false;

  return true;
}

int TCPSocketClt::Send(const char *data, size_t dataLen)
{
  return SendData(sock_, data, dataLen);
}

#endif