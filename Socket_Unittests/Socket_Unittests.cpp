#include "pch.h"

#include <string>

#include "../SockSample/Socket.h"
#include "../SockSample/Socket.cpp"

TEST(SocketClassTests, ServerOpenTest) {
  TCPSocketSrv srv;
  auto opened = srv.Open(9999);
  EXPECT_TRUE(opened);
}

TEST(SocketClassTests, ServerOpenSecondTimeTest) {
  TCPSocketSrv srv;
  auto opened = srv.Open(9999);
  EXPECT_TRUE(opened);
  opened = srv.Open(19999);
  EXPECT_TRUE(opened);
}

TEST(SocketClassTests, ServerOpenFailedTest) {
  TCPSocketSrv srv1;
  auto s1opened = srv1.Open(9999);
  EXPECT_TRUE(s1opened);

  TCPSocketSrv srv2;
  auto s2opened = srv2.Open(9999);
  EXPECT_FALSE(s2opened);
}

TEST(SocketClassTests, ClientOpenFailedTest) {
  TCPSocketClt clt;
  auto opened = clt.Open(9999);
  EXPECT_FALSE(opened);
}

TEST(SocketClassTests, ClientOpenDiffPortFailedTest) {
  TCPSocketSrv srv;
  auto sOpened = srv.Open(19999);
  EXPECT_TRUE(sOpened);

  TCPSocketClt clt;
  auto cOpened = clt.Open(9999);
  EXPECT_FALSE(cOpened);
}

TEST(SocketClassTests, ClientOpenTest) {
  TCPSocketSrv srv;
  auto sOpened = srv.Open(9999);
  EXPECT_TRUE(sOpened);

  TCPSocketClt clt;
  auto cOpened = clt.Open(9999);
  EXPECT_TRUE(cOpened);
}

TEST(SocketClassTests, SendRecvTest) {
  TCPSocketSrv srv;
  auto sOpened = srv.Open(9999);
  EXPECT_TRUE(sOpened);

  TCPSocketClt clt;
  auto cOpened = clt.Open(9999);
  EXPECT_TRUE(cOpened);

  // check client send
  std::string cltData = "0987654321";
  auto sendedCltBytes = clt.Send(cltData.c_str(), cltData.length());
  EXPECT_EQ(cltData.length(), sendedCltBytes);

  // check server recv
  SOCKET cltSock;
  size_t cltDataLen;
  auto cltRecvData = srv.Recv(cltSock, cltDataLen);
  EXPECT_TRUE(cltData.compare(cltRecvData) == 0);
  EXPECT_EQ(cltData.length(), cltDataLen);

  // check server send
  std::string srvData = "12345678900987654321";
  auto sendedSrvBytes = srv.Send(cltSock, srvData.c_str(), srvData.length());
  EXPECT_EQ(srvData.length(), sendedSrvBytes);

  // check client recv
  SOCKET srvSock;
  size_t srvDataLen;
  auto srvRecvData = clt.Recv(srvSock, srvDataLen);
  EXPECT_TRUE(srvData.compare(srvRecvData) == 0);
  EXPECT_EQ(srvData.length(), srvDataLen);
}

TEST(SocketClassTests, ManyClientsOpenTest) {
  TCPSocketSrv srv;
  auto sOpened = srv.Open(9999);
  EXPECT_TRUE(sOpened);

  TCPSocketClt clts[FD_SETSIZE];
  for (auto clt : clts)
  {
    auto cOpened = clt.Open(9999);
    EXPECT_TRUE(cOpened);
  }
}

TEST(SocketClassTests, ManyClientsSendAndRecvTest) {
  TCPSocketSrv srv;
  auto sOpened = srv.Open(9999);
  EXPECT_TRUE(sOpened);

  TCPSocketClt clts[FD_SETSIZE-1];
  SOCKET cSocks[FD_SETSIZE-1];

  std::string cData = "Hello from client ";
  for (auto i = 0; i < FD_SETSIZE-1; ++i)
  {
    TCPSocketClt &clt = clts[i];
    auto cOpened = clt.Open(9999);
    EXPECT_TRUE(cOpened);

    auto cSended = clt.Send(cData.c_str(), cData.length());
    EXPECT_EQ(cData.length(), cSended);

    size_t recvDataLen;
    auto srvRecvData = srv.Recv(cSocks[i], recvDataLen);
    EXPECT_TRUE(cData.compare(srvRecvData) == 0);
    EXPECT_EQ(cData.length(), recvDataLen);
  }

  std::string sData = "Hello from server";
  for (auto i = 0; i < FD_SETSIZE - 1; ++i)
  {
    auto sSended = srv.Send(cSocks[i], sData.c_str(), sData.length());
    EXPECT_EQ(sData.length(), sSended);

    SOCKET sSock;
    size_t recvDataLen;
    auto cltRecvData = clts[i].Recv(sSock, recvDataLen);
    EXPECT_TRUE(sData.compare(cltRecvData) == 0);
    EXPECT_EQ(sData.length(), recvDataLen);
  }
}
