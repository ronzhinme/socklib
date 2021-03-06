#include "pch.h"

#include <string>
#include <chrono>
#include <thread>
#include <unordered_map>

#if defined(WIN32)
#include "../SockSample/BytesData.h"
#include "../SockSample/BytesData.cpp"
#include "../SockSample/Socket.h"
#include "../SockSample/Socket_win.cpp"
#include <winsock2.h>
#elif defined(__linux__)
#include <Socket.h>
#endif
#include <forward_list>

using namespace std::chrono_literals;

TEST(SocketClassTests, ServerOpenTest)
{
	TCPSocketSrv srv;
	auto opened = srv.Open(0);
	EXPECT_TRUE(opened);
	EXPECT_TRUE(srv.IsConnected());

	srv.Close();
	EXPECT_FALSE(srv.IsConnected());
}

TEST(SocketClassTests, ServerOpenSecondTimeTest)
{
	TCPSocketSrv srv;
	auto opened = srv.Open(0);
	EXPECT_TRUE(opened);
	EXPECT_TRUE(srv.IsConnected());

	opened = srv.Open(0);
	EXPECT_TRUE(opened);
	EXPECT_TRUE(srv.IsConnected());

	srv.Close();
	EXPECT_FALSE(srv.IsConnected());
}

TEST(SocketClassTests, ServerOpenFailedTest)
{
	TCPSocketSrv srv1;
	auto s1opened = srv1.Open(9999);
	EXPECT_TRUE(s1opened);
	EXPECT_TRUE(srv1.IsConnected());

	TCPSocketSrv srv2;
	auto s2opened = srv2.Open(9999);
	EXPECT_FALSE(s2opened);
	EXPECT_FALSE(srv2.IsConnected());

	srv1.Close();
	EXPECT_FALSE(srv1.IsConnected());
	srv2.Close();
	EXPECT_FALSE(srv2.IsConnected());
}

TEST(SocketClassTests, ClientOpenFailedTest)
{
	TCPSocketClt clt;
	auto opened = clt.Open(0);
	EXPECT_FALSE(opened);
	EXPECT_FALSE(clt.IsConnected());
}

TEST(SocketClassTests, ClientOpenDiffPortFailedTest)
{
	TCPSocketSrv srv;
	auto sOpened = srv.Open(19999);
	EXPECT_TRUE(sOpened);
	EXPECT_TRUE(srv.IsConnected());

	TCPSocketClt clt;
	auto cOpened = clt.Open(9999, "localhost");
	EXPECT_FALSE(cOpened);
	EXPECT_FALSE(clt.IsConnected());

	EXPECT_EQ(0, srv.GetConnectedSockets());
	srv.Close();
}

TEST(SocketClassTests, ClientOpenTest)
{
	int port = 10999;
	TCPSocketSrv srv;
	auto sOpened = srv.Open(port);
	EXPECT_TRUE(sOpened);
	EXPECT_TRUE(srv.IsConnected());
	EXPECT_EQ(0, srv.GetConnectedSockets());

	TCPSocketClt clt;
	auto cOpened = clt.Open(port, "127.0.0.1");

	std::this_thread::sleep_for(10ms);

	EXPECT_TRUE(cOpened);
	EXPECT_TRUE(clt.IsConnected());
	EXPECT_EQ(1, srv.GetConnectedSockets());
	clt.Close();
	srv.Close();
}

TEST(SocketClassTests, ClientOpenAndReadNullTest)
{
	int port = 13999;
	TCPSocketSrv srv;
	auto sOpened = srv.Open(port);
	EXPECT_TRUE(sOpened);
	EXPECT_TRUE(srv.IsConnected());
	EXPECT_EQ(0, srv.GetConnectedSockets());

	TCPSocketClt clt;
	auto cOpened = clt.Open(port, "localhost");

	std::this_thread::sleep_for(10ms);

	EXPECT_TRUE(cOpened);
	EXPECT_TRUE(clt.IsConnected());
	EXPECT_EQ(1, srv.GetConnectedSockets());

	for (auto i = 0; i < 1000; ++i)
	{
		auto rData = clt.Recv();
		EXPECT_EQ(0, rData.size());
	}

	clt.Close();
	srv.Close();
}

TEST(SocketClassTests, SendRecvTest)
{
	int port = 12999;
	TCPSocketSrv srv;
	auto sOpened = srv.Open(port);
	EXPECT_TRUE(sOpened);

	TCPSocketClt clt;
	auto cOpened = clt.Open(port, "127.0.0.1");
	EXPECT_TRUE(cOpened);

	std::this_thread::sleep_for(100ms);
	EXPECT_EQ(1, srv.GetConnectedSockets());

	//check client send
	std::string cltData = "0987654321";
	auto sendedCltBytes = clt.Send(cltData.c_str(), cltData.length());
	EXPECT_EQ(cltData.length(), sendedCltBytes);

	//check server recv
	auto srvRecvData = srv.Recv();
	EXPECT_EQ(1, srvRecvData.size());
	EXPECT_TRUE(cltData.compare(0, cltData.length(), (*srvRecvData.begin()).second.Bytes, 0, (*srvRecvData.begin()).second.Count) == 0);

	std::string sData = "12345678900987654321";
	for (auto recvData : srvRecvData)
	{
		//check server send
		auto sendedSrvBytes = srv.Send(recvData.first, sData.c_str(), sData.length());
		EXPECT_EQ(sData.length(), sendedSrvBytes);
	}

	//check client recv
	auto i = 0;
	while (i++ < 10)
	{
		auto cltRecvData = clt.Recv();
		if (cltRecvData.size() == 0)
		{
			continue;
		}

		EXPECT_EQ(1, cltRecvData.size());
		EXPECT_TRUE(sData.compare(0, sData.length(), (*cltRecvData.begin()).second.Bytes, 0, (*cltRecvData.begin()).second.Count) == 0);
		break;
	}

	clt.Close();
	srv.Close();
}

TEST(SocketClassTests, _1024ClientsOpenTest)
{
	int port = 39999;
	TCPSocketSrv srv;
	auto sOpened = srv.Open(port);
	EXPECT_TRUE(sOpened);
	EXPECT_TRUE(srv.IsConnected());

	if(!sOpened)
	{
		return;
	}

	const auto count = srv.GetMaxAvailableSockets();
	std::forward_list<TCPSocketClt *> clts;
	for (auto i = 0; i < count; ++i)
	{
		clts.emplace_front(new TCPSocketClt());
	}

	auto i = 0;
	for (auto clt : clts)
	{
		auto cOpened = clt->Open(port, i % 2 == 0 ? "localhost" : "127.0.0.1");
		EXPECT_TRUE(cOpened);
		EXPECT_TRUE(clt->IsConnected());
		std::this_thread::sleep_for(10ms);
		EXPECT_EQ(++i, srv.GetConnectedSockets());
	}

	for (auto clt : clts)
	{
		clt->Close();
		EXPECT_EQ(--i, srv.GetConnectedSockets());
	}

	srv.Close();
}

TEST(SocketClassTests, CountingClientsTest)
{
	int port = 50999;
	TCPSocketSrv srv;
	auto sOpened = srv.Open(port);
	EXPECT_TRUE(sOpened);
	EXPECT_TRUE(srv.IsConnected());

	const auto count = srv.GetMaxAvailableSockets();
	std::forward_list<TCPSocketClt *> clts;
	for (auto i = 0; i < 100; ++i)
	{
		clts.emplace_front(new TCPSocketClt());
	}

	auto i = 0;
	for (auto clt : clts)
	{
		auto cOpened = clt->Open(port);
		EXPECT_TRUE(cOpened);
		EXPECT_TRUE(clt->IsConnected());
		std::this_thread::sleep_for(10ms);
		EXPECT_EQ(++i, srv.GetConnectedSockets());
		printf(".");
	}

	printf("\n===\n");

	for (auto clt : clts)
	{
		clt->Close();
		EXPECT_FALSE(clt->IsConnected());
		std::this_thread::sleep_for(10ms);
		EXPECT_EQ(--i, srv.GetConnectedSockets());
		printf("!");
	}

	printf("\n");

	srv.Close();
}

TEST(SocketClassTests, SendClientsAfterCloseTest)
{
	int port = 9999;
	TCPSocketSrv srv;
	auto sOpened = srv.Open(port);
	EXPECT_TRUE(sOpened);
	EXPECT_TRUE(srv.IsConnected());
	TCPSocketClt c1, c2;
	auto c1Opened = c1.Open(9999);
	EXPECT_TRUE(c1Opened);
	EXPECT_TRUE(c1.IsConnected());
	auto c2Opened = c2.Open(9999);
	EXPECT_TRUE(c2Opened);
	EXPECT_TRUE(c2.IsConnected());

	//send before close
	auto c1Res = c1.Send(".", 1);
	EXPECT_EQ(c1Res, 1);
	auto c2Res = c2.Send(".", 1);
	EXPECT_EQ(c2Res, 1);

	//close 2nd
	c2.Close();
	EXPECT_FALSE(c2.IsConnected());

	//send after close
	c2Res = c2.Send(".", 1);
	EXPECT_LT(c2Res, 0);
	c1Res = c1.Send(".", 1);
	EXPECT_EQ(c1Res, 1);

	c1.Close();
	c2.Close();
	srv.Close();
}

TEST(SocketClassTests, _1024ClientsSendAndRecvTest)
{
	int port = 8999;
	TCPSocketSrv srv;
	auto sOpened = srv.Open(port);
	EXPECT_TRUE(sOpened);

	const std::string cData = "Hello from client ";
	const std::string sData = "Hello from server";

	const auto count = srv.GetMaxAvailableSockets();

	std::forward_list<TCPSocketClt *> clts;
	for (auto i = 0; i < count; ++i)
	{
		clts.emplace_front(new TCPSocketClt());
	}

	int i = 0;
	for (auto c : clts)
	{
		auto cOpened = c->Open(port);
		EXPECT_TRUE(cOpened);

		std::this_thread::sleep_for(100ms);
		EXPECT_EQ(++i, srv.GetConnectedSockets());
	}

	EXPECT_EQ(count, std::distance(clts.begin(), clts.end()));

	for (auto c : clts)
	{
		auto cSent = c->Send(cData.c_str(), cData.length());
		EXPECT_EQ(cData.length(), cSent);
	}

	auto srvRecvData = srv.Recv();
	EXPECT_EQ(count, srvRecvData.size());

	for (auto recvData : srvRecvData)
	{
		EXPECT_TRUE(cData.compare(0, cData.length(), recvData.second.Bytes, 0, recvData.second.Count) == 0);
		auto sSended = srv.Send(recvData.first, sData.c_str(), sData.length());
		EXPECT_EQ(sData.length(), sSended);
	}

	for (auto c : clts)
	{
		auto i = 0;
		while (i++ < 10)
		{
			auto cltRecvData = c->Recv();
			if (cltRecvData.size() == 0)
			{
				continue;
			}

			EXPECT_EQ(1, cltRecvData.size());
			EXPECT_TRUE(sData.compare(0, sData.length(), (*cltRecvData.begin()).second.Bytes, 0, (*cltRecvData.begin()).second.Count) == 0);
			break;
		}
	}

	for (auto c : clts)
	{
		c->Close();
	}

	srv.Close();
}

#if defined(LINUX)
int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
#endif