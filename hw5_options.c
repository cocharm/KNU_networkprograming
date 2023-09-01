#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

int main()
{
	printf("SO_SNDBUF: %d\n", SO_SNDBUF);
	printf("SO_RCVBUF: %d\n", SO_RCVBUF);
	printf("SO_REUSEADDR: %d\n", SO_REUSEADDR);
	printf("SO_KEEPALIVE: %d\n", SO_KEEPALIVE);
	printf("SO_BROADCAST: %d\n", SO_BROADCAST);
	printf("IP_TOS: %d\n", IP_TOS);
	printf("IP_TTL: %d\n", IP_TTL);
	printf("TCP_NODELAY: %d\n", TCP_NODELAY);
	printf("TCP_MAXSEG: %d\n", TCP_MAXSEG);

	return 0;
}
