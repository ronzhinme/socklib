// SockSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Socket.h"
#include <iostream>
int main()
{
  TCPSocketSrv srv;
  TCPSocketClt clt;

  if (!srv.Open(9999, NULL))
  {
    std::cout << "srv.Open() error" << std::endl;
    system("pause");
    return 1;
  }
  if (!clt.Open(9999, NULL))
  {
    std::cout << "clt.Open() error" << std::endl;
    system("pause");
    return 1;
  }

  auto sended = clt.Send("Test", 4);
  std::cout << "Client sended: Test" << std::endl;
  std::cout << "Sended bytes: " << sended << std::endl;

  SOCKET srcSock;
  size_t dataLen;
  auto data = srv.Recv(srcSock, dataLen);
  std::cout << "Server recieved: " << data << std::endl;
  data = srv.Recv(srcSock, dataLen);
  std::cout << "Server recieved: " << data << std::endl;

  sended = srv.Send(srcSock, "Hello", 5);
  std::cout << "Server sended: Hello" << std::endl;
  std::cout << "Sended bytes: " << sended << std::endl;

  data = clt.Recv(srcSock, dataLen);
  std::cout << "Client recieved: " << data << std::endl;

  system("pause");

  ////init win sock
  //WSADATA wsaData;
  //if (WSAStartup(MAKEWORD(2, 2), &wsaData)) return 1;
  //
  //struct addrinfo cltSockSets;
  //ZeroMemory(&cltSockSets, sizeof(cltSockSets));
  //cltSockSets.ai_family = AF_UNSPEC;
  //cltSockSets.ai_socktype = SOCK_STREAM;
  //cltSockSets.ai_protocol = IPPROTO_TCP;

  //auto srvsock = socket();
  //bind();
  //listen();
  //auto sock = accept();

  //
  //struct addrinfo cltSockSets;
  //ZeroMemory(&cltSockSets, sizeof(cltSockSets));
  //cltSockSets.ai_family = AF_UNSPEC;
  //cltSockSets.ai_socktype = SOCK_STREAM;
  //cltSockSets.ai_protocol = IPPROTO_TCP;

  //auto cltsock = socket();
  //connect();


  return 0;
}

