
#include "SockSample/Socket.h"

int main(int argc, char* argv[])
{
	TCPSocketClt clt;
	bool isOpened = clt.Open(12345, "192.168.0.5");
	printf("isOpened = %d; WSALastError = %d", isOpened, WSAGetLastError());
	return 0;
}