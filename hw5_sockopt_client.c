#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#define BUF_SIZE 30

typedef struct {
	int level;
	int option;
	int optval;
	int result;
}SO_PACKET;

void error_handling(char* message);

int main(int argc, char* argv[])
{
	int sock;
	socklen_t adr_sz;
	struct sockaddr_in serv_adr, from_adr;

	if(argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(sock == -1)
		error_handling("socket() error\n");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));

	SO_PACKET rcv_pkt;
	SO_PACKET snd_pkt;

	char* optname;

	while(1)
	{
		memset(&rcv_pkt, 0, sizeof(SO_PACKET));
		memset(&snd_pkt, 0, sizeof(SO_PACKET));
		optname = NULL;

		printf("---------------------------\n");
		printf("1: SO_SNDBUF\n");
		printf("2: SO_RCVBUF\n");
		printf("3: SO_REUSEADDR\n");
		printf("4: SO_KEEPALIVE\n");
		printf("5: SO_BROADCAST\n");
		printf("6: IP_TOS\n");
		printf("7: IP_TTL\n");
		printf("8: TCP_NODELAY\n");
		printf("9: TCP_MAXSEG\n");
		printf("10: Quit");
		printf("---------------------------\n");

		int opt;
		int level;
		int option;

		printf("Input option number: ");
		scanf("%d", &opt);

		if(opt <= 0 || 10 < opt)
		{
			printf("Wrong number. type again!\n");
			continue;
		}

		else if(1 <= opt && opt <= 5)
		{
			level = SOL_SOCKET;

			if(opt == 1)
			{
				option = SO_SNDBUF;
				optname = "SO_SNDBUF";
			}
			else if(opt == 2)
			{
				option = SO_RCVBUF;
				optname = "SO_RCVBUF";
			}
			else if(opt == 3)
			{
				option = SO_REUSEADDR;
				optname = "SO_REUSEADDR";
			}
			else if(opt == 4)
			{
				option = SO_KEEPALIVE;
				optname = "SO_KEEPALIVE";
			}
			else
			{
				option = SO_BROADCAST;
				optname = "SO_BROADCAST";
			}
		}

		else if(6 <= opt && opt <= 7)
		{
			level = IPPROTO_IP;
			if(opt == 6)
			{
				option = IP_TOS;
				optname = "IP_TOS";
			}
			else
			{
				option = IP_TTL;
				optname = "IP_TTL";
			}
		}

		else if(8 <= opt && opt <= 9)
		{
			level = IPPROTO_TCP;

			if(opt == 8)
			{
				option = TCP_NODELAY;
				optname = "TCP_NODELAY";
			}
			else
			{
				option = TCP_MAXSEG;
				optname = "TCP_MAXSEG";
			}
		}

		else if(opt == 10)
		{
			printf("Client quit.\n");
			break;
		}

		else
		{
			printf("opt wrong error\n");
		}

		snd_pkt.level = level;
		snd_pkt.option = option;

		sendto(sock, &snd_pkt, sizeof(SO_PACKET), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
		adr_sz = sizeof(from_adr);

		int bytes = recvfrom(sock, &rcv_pkt, sizeof(SO_PACKET), 0, (struct sockaddr*)&from_adr, &adr_sz);

		if(bytes == -1)
			error_handling("recvfrom() error");

		if(rcv_pkt.result == -1)
		{
			printf("result Failed\n");
			continue;
		}

		printf("Server result: %s: value: %d, result: %d\n\n", optname, rcv_pkt.optval, rcv_pkt.result);
	}

	close(sock);
	return 0;
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
