
#if defined(WIN32)

#include "Socket.h"

#include <windows.h>
#include <winsock2.h>
#include <string.h>
#include <chrono>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std::chrono_literals;

size_t SocketBase::GetConnectedSockets()
{
	auto res = WSAPoll(pfd_sock_, maxClients_, 100);
	if (res > 0)
	{
		for (auto i = 1; i < maxClients_; ++i)
		{
			if (pfd_sock_[i].fd == INVALID_SOCKET)
			{
				continue;
			}

			if (pfd_sock_[i].revents & (POLLIN | POLLHUP))
			{
				RemoveConnections(pfd_sock_[i].fd);
			}
		}
	}

	return connectedClients_;
}

const size_t SocketBase::GetMaxAvailableSockets() const
{
	return maxClients_ - 1;
}

void SocketBase::RemoveConnections(SOCKET sock)
{
	for (auto i = 0; i < maxClients_; ++i)
	{
		if (pfd_sock_[i].fd == sock)
		{
			memcpy(&pfd_sock_[i], &pfd_sock_[connectedClients_], sizeof(struct pollfd));
			memset(&pfd_sock_[connectedClients_], 0, sizeof(struct pollfd));
			
			//printf("Connected clients: [%d] [%d]\n", connectedClients_, sock);
			--connectedClients_;
			break;
		}
	}

}

int SocketBase::SendData(SOCKET destSock, const char* data, size_t dataLen)
{
	auto result = send(destSock, data, dataLen, 0);
	//printf("Sent result: [%d] >>>> [%s] [%d] destSock:[%d]\n", result, data, dataLen, destSock);
	if (result <= 0)
	{
		RemoveConnections(destSock);
	}

	return result;
}

SocketBase::SocketBase()
{
	port_ = 0;
	host_ = 0;
	memset(pfd_sock_, 0, sizeof(pfd_sock_));

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

SocketBase::~SocketBase()
{
	WSACleanup();
}

std::unordered_map<SOCKET, BytesData> SocketBase::Recv()
{
	std::unordered_map<SOCKET, BytesData> recvData;
	char recvBuff[SO_MAX_MSG_SIZE];

	auto result = WSAPoll(&pfd_sock_[0], connectedClients_ + 1, 0);
	if (result <= 0)
	{
		return recvData;
	}

	for (auto i = 0; i < connectedClients_ + 1; ++i)
	{
		if (!isServer_ && pfd_sock_[i].fd != sock_)
		{
			continue;
		}

		if ((pfd_sock_[i].revents == 0 || pfd_sock_[i].fd == sock_) && isServer_)
		{
			continue;
		}

		auto recvBytes = recv(pfd_sock_[i].fd, &recvBuff[0], SO_MAX_MSG_SIZE, MSG_PEEK);
		if (recvBytes <= 0)
		{
			continue;
		}

		memset((void*)&recvBuff[0], 0, SO_MAX_MSG_SIZE);
		recv(pfd_sock_[i].fd, &recvBuff[0], recvBytes, 0);
		recvData.emplace(pfd_sock_[i].fd, BytesData(recvBuff, recvBytes));
	}

	return recvData;
}

void SocketBase::Close()
{
	if (isConnected_)
	{
		shutdown(sock_, SD_BOTH);
		closesocket(sock_);
	}

	isConnected_ = false;
}

constexpr bool SocketBase::IsConnected()
{
	return isConnected_;
}

unsigned short SocketBase::GetSocketPort() const
{
	return port_;
}

unsigned long SocketBase::GetSocketHost() const
{
	return host_;
}

/// <summary>
/// Server
/// </summary>

TCPSocketSrv::TCPSocketSrv()
{
	isConnected_ = false;
	acceptThread_ = NULL;
	isServer_ = true;
}

TCPSocketSrv::~TCPSocketSrv()
{
	isConnected_ = false;
}

bool TCPSocketSrv::Open(unsigned short port, const char* ip)
{
	if (isConnected_)
	{
		isConnected_ = false;
		while (acceptThread_ != NULL)
		{
			std::this_thread::yield();
		}
	}

	ADDRINFO* resultAddr;
	ADDRINFO localaddr_;
	ZeroMemory(&localaddr_, sizeof(localaddr_));
	localaddr_.ai_family = AF_INET;
	localaddr_.ai_socktype = SOCK_STREAM;
	localaddr_.ai_protocol = IPPROTO_TCP;
	localaddr_.ai_flags = AI_PASSIVE;

	char portValue[6];
	memset(&portValue[0], 0, 6);
	_itoa_s(port, portValue, 10);
	getaddrinfo(NULL, portValue, &localaddr_, &resultAddr);

	sock_ = socket(resultAddr->ai_family, resultAddr->ai_socktype, resultAddr->ai_protocol);
	if (sock_ == INVALID_SOCKET)
	{
		isConnected_ = false;
		return isConnected_;
	}

	//char reuseAddr = 1;
	//if(setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseAddr, sizeof(reuseAddr)) < 0)
	//{
	//	isConnected_ = false;
	//	return isConnected_;
	//}

	u_long nonBlockMode = 1;
	if (ioctlsocket(sock_, FIONBIO, &nonBlockMode) != NO_ERROR)
	{
		isConnected_ = false;
		return isConnected_;
	}

	if (bind(sock_, resultAddr->ai_addr, (int)resultAddr->ai_addrlen) == SOCKET_ERROR)
	{
		isConnected_ = false;
		return isConnected_;
	}

	if (listen(sock_, SOMAXCONN) == SOCKET_ERROR)
	{
		isConnected_ = false;
		return isConnected_;
	}

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(sock_, (struct sockaddr*)&sin, &len) != NO_ERROR)
	{
		isConnected_ = false;
		return isConnected_;
	}

	port_ = ntohs(sin.sin_port);
	host_ = sin.sin_addr.s_addr;

	freeaddrinfo(resultAddr);
	memset(pfd_sock_, 0, sizeof(pfd_sock_));
	pfd_sock_[0].fd = sock_;
	pfd_sock_[0].revents = POLLIN;

	isConnected_ = true;
	acceptThread_ = new std::thread(&TCPSocketSrv::AcceptNewConnections, this);
	return isConnected_;
}

int TCPSocketSrv::Send(SOCKET destSock, const char* data, size_t dataLen)
{
	return SendData(destSock, data, dataLen);
}

int TCPSocketSrv::GetSocketIndex(SOCKET s)
{
	for (auto i = 1; i < connectedClients_; ++i)
	{
		if (pfd_sock_[i].fd == s)
		{
			return i;
		}
	}

	return -1;
}

void TCPSocketSrv::AcceptNewConnections()
{
	do
	{
		SOCKET s = INVALID_SOCKET;
		do
		{
			if (!isConnected_)
			{
				break;
			}

			s = accept(sock_, NULL, NULL);
			if (s == INVALID_SOCKET || s < 0)
			{
				if (errno != EWOULDBLOCK)
				{
					//end_server = TRUE;
				}
			}
			else
			{
				auto i = GetSocketIndex(s);
				if ( i > -1)
				{
					std::this_thread::sleep_for(1ms);
					std::this_thread::yield();
					continue;
				}

				++connectedClients_;
				pfd_sock_[connectedClients_].fd = s;
				pfd_sock_[connectedClients_].events = POLLIN;
			}

			std::this_thread::sleep_for(1ms);
			std::this_thread::yield();
		} while (s != INVALID_SOCKET || isConnected_);


		std::this_thread::sleep_for(1ms);
		std::this_thread::yield();
	} while (isConnected_);

	shutdown(sock_, SD_BOTH);
	closesocket(sock_);
	acceptThread_ = NULL;
}

/// <summary>
/// Client
/// </summary>

TCPSocketClt::TCPSocketClt()
{
	isServer_ = false;
}

TCPSocketClt::~TCPSocketClt()
{
	Close();
}

bool TCPSocketClt::Open(unsigned short port, const char* ip)
{
	ADDRINFO* resultAddr;
	ADDRINFO localaddr_;
	ZeroMemory(&localaddr_, sizeof(localaddr_));
	localaddr_.ai_family = AF_INET;
	localaddr_.ai_socktype = SOCK_STREAM;
	localaddr_.ai_protocol = IPPROTO_TCP;

	char portValue[6];
	memset(&portValue[0], 0, 6);
	_itoa_s(port, portValue, 10);
	getaddrinfo(NULL, portValue, &localaddr_, &resultAddr);

	for (auto ptr = resultAddr; ptr != NULL; ptr = ptr->ai_next)
	{
		sock_ = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (sock_ == INVALID_SOCKET)
		{
			isConnected_ = false;
			Close();
			return isConnected_;
		}

		if (connect(sock_, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
		{
			sock_ = SOCKET_ERROR;
			continue;
		}

		port_ = ntohs(((struct sockaddr_in*)ptr->ai_addr)->sin_port);
		host_ = ((struct sockaddr_in*)ptr->ai_addr)->sin_addr.s_addr;
		break;
	}

	if (sock_ == SOCKET_ERROR)
	{
		isConnected_ = false;
		Close();
		return isConnected_;
	}

	freeaddrinfo(resultAddr);
	memset(pfd_sock_, 0, sizeof(pfd_sock_));
	pfd_sock_[0].fd = sock_;
	pfd_sock_[0].revents = POLLIN;
	isConnected_ = true;
	return isConnected_;
}

int TCPSocketClt::Send(const char* data, size_t dataLen)
{
	return SendData(sock_, data, dataLen);
}
#endif
