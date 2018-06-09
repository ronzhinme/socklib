#include "pch.h"

#include <string>

#if defined(WIN32)
#include "../SockSample/BytesData.h"
#include "../SockSample/BytesData.cpp"
#include "../SockSample/Socket.h"
#include "../SockSample/Socket_win.cpp"
#elif defined(LINUX)
#include "../SockSample/Socket.h"
#endif

TEST(SocketClassTests, ServerOpenTest)
{
  TCPSocketSrv srv;
  auto opened = srv.Open(9999);
  EXPECT_TRUE(opened);
  srv.Close();
}

TEST(SocketClassTests, ServerOpenSecondTimeTest)
{
  TCPSocketSrv srv;
  auto opened = srv.Open(9999);
  EXPECT_TRUE(opened);
  opened = srv.Open(19999);
  EXPECT_TRUE(opened);
  srv.Close();
}

TEST(SocketClassTests, ServerOpenFailedTest)
{
  TCPSocketSrv srv1;
  auto s1opened = srv1.Open(9999);
  EXPECT_TRUE(s1opened);

  TCPSocketSrv srv2;
  auto s2opened = srv2.Open(9999);
  EXPECT_FALSE(s2opened);
}

TEST(SocketClassTests, ClientOpenFailedTest)
{
  TCPSocketClt clt;
  auto opened = clt.Open(9999);
  EXPECT_FALSE(opened);
}

TEST(SocketClassTests, ClientOpenDiffPortFailedTest)
{
  TCPSocketSrv srv;
  auto sOpened = srv.Open(19999);
  EXPECT_TRUE(sOpened);

  TCPSocketClt clt;
  auto cOpened = clt.Open(9999);
  EXPECT_FALSE(cOpened);
}

TEST(SocketClassTests, ClientOpenTest)
{
  TCPSocketSrv srv;
  auto sOpened = srv.Open(9999);
  EXPECT_TRUE(sOpened);

  TCPSocketClt clt;
  auto cOpened = clt.Open(9999);
  EXPECT_TRUE(cOpened);
}

TEST(SocketClassTests, SendRecvTest)
{
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
  auto srvRecvData = srv.Recv();
  EXPECT_TRUE(cltData.compare(0, cltData.length(), (*srvRecvData.begin()).second.Bytes, 0, (*srvRecvData.begin()).second.Count) == 0);

  std::string sData = "12345678900987654321";
  for (auto recvData : srvRecvData)
  {
    // check server send
    auto sendedSrvBytes = srv.Send(recvData.first, sData.c_str(), sData.length());
    EXPECT_EQ(sData.length(), sendedSrvBytes);
  }
  // check client recv
  auto cltRecvData = clt.Recv();
  EXPECT_EQ(1, cltRecvData.size());
  EXPECT_TRUE(sData.compare(0, sData.length(), (*cltRecvData.begin()).second.Bytes, 0, (*cltRecvData.begin()).second.Count) == 0);
}

TEST(SocketClassTests, ManyClientsOpenTest)
{
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

TEST(SocketClassTests, ManyClientsSendAndRecvTest)
{
  TCPSocketSrv srv;
  auto sOpened = srv.Open(9999);
  EXPECT_TRUE(sOpened);

  TCPSocketClt clts[FD_SETSIZE - 1];
  SOCKET cSocks[FD_SETSIZE - 1];

  std::string cData = "Hello from client ";
  for (auto i = 0; i < FD_SETSIZE - 1; ++i)
  {
    TCPSocketClt &clt = clts[i];
    auto cOpened = clt.Open(9999);
    EXPECT_TRUE(cOpened);

    auto cSended = clt.Send(cData.c_str(), cData.length());
    EXPECT_EQ(cData.length(), cSended);
  }

  auto srvRecvData = srv.Recv();
  EXPECT_EQ(FD_SETSIZE - 1, srvRecvData.size());
  std::string sData = "Hello from server";
  for (auto recvData : srvRecvData)
  {
    EXPECT_TRUE(cData.compare(0, cData.length(), recvData.second.Bytes, 0, recvData.second.Count) == 0);

    auto sSended = srv.Send(recvData.first, sData.c_str(), sData.length());
    EXPECT_EQ(sData.length(), sSended);
  }

  for (auto i = 0; i < FD_SETSIZE - 1; ++i)
  {
    auto cltRecvData = clts[i].Recv();
    EXPECT_EQ(1, cltRecvData.size());
    EXPECT_TRUE(sData.compare(0, sData.length(), (*cltRecvData.begin()).second.Bytes, 0, (*cltRecvData.begin()).second.Count) == 0);
  }
}

#if defined(LINUX)
int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif