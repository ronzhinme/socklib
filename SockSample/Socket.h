
#if !defined(SOCKETCLASS_H)
#define SOCKETCLASS_H

#include "BytesData.h"
#include <unordered_map>
#include <thread>

#if defined(WIN32)
#include <WS2tcpip.h>
#elif defined(__linux__)
#include <sys/socket.h>
#include <sys/poll.h>
#define SOCKET int
#endif

class SocketBase
{
protected:
	static const size_t maxClients_ = 1024;
	size_t connectedClients_ = 0;
	SOCKET sock_;
	struct pollfd pfd_sock_[maxClients_];
	int SendData(SOCKET destSock, const char* data, size_t dataLen);
	unsigned long host_;
	unsigned short port_;
	bool isConnected_;
	void RemoveConnections(SOCKET socket);
	bool isServer_;
public:
	SocketBase();
	virtual ~SocketBase();
	virtual bool Open(unsigned short port, const char* ip = NULL) = 0;
	virtual std::unordered_map<SOCKET, BytesData> Recv();
	virtual void Close();
	size_t GetConnectedSockets();
	const size_t GetMaxAvailableSockets() const;
	unsigned short GetSocketPort() const;
	unsigned long GetSocketHost() const;
	bool IsConnected() const;
};

class TCPSocketSrv : public SocketBase
{
private:
	std::thread *acceptThread_;
	int GetSocketIndex(SOCKET s);
	void AcceptNewConnections();
public:
	TCPSocketSrv();
	~TCPSocketSrv();
	bool Open(unsigned short port, const char* ip = NULL);
	int Send(SOCKET destSock, const char* data, size_t dataLen);
	const SOCKET GetConnectedSocketByIndex(size_t indx) const;
};

class TCPSocketClt : public SocketBase
{
public:
	TCPSocketClt();
	~TCPSocketClt();
	bool Open(unsigned short port, const char* ip = NULL);
	int Send(const char* data, size_t dataLen);
};

#endif