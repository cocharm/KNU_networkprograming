#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/tcp.h>

typedef struct {
	int level;
	int option;
	int optval;
	int result;
}SO_PACKET;

void error_handling(char* message);

int main(int argc, char* argv[])
{
	int serv_sock;
	int bytes;
	socklen_t clnt_adr_sz;
	struct sockaddr_in serv_adr, clnt_adr;

	if(argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
	}

	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock == -1)
		error_handling("UDP socket creation error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");

	printf("Socket Option Server Start\n");

	SO_PACKET rcv_pkt;
	SO_PACKET snd_pkt;

	int state;
	int optval;
	int optlen;

	int sock = socket(PF_INET, SOCK_STREAM, 0);  // TCP socket
	char* send_option;

	while(1)
	{
		state = -1;
		optval = -1;
		send_option = NULL;

		memset(&rcv_pkt, 0, sizeof(SO_PACKET));
		memset(&snd_pkt, 0, sizeof(SO_PACKET));

		clnt_adr_sz = sizeof(clnt_adr);
		recvfrom(serv_sock, &rcv_pkt, sizeof(SO_PACKET), 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);  // received, so clnt_adr is received too 
		// check receive packet
		int level = rcv_pkt.level;
		int option = rcv_pkt.option;

		if(level == SOL_SOCKET)
		{
			switch(option)
			{
				case SO_SNDBUF:
					send_option = "SO_SNDBUF";
					break;
				case SO_RCVBUF:
					send_option = "SO_RCVBUF";
					break;
				case SO_REUSEADDR:
					send_option = "SO_REUSEADDR";
					break;
				case SO_KEEPALIVE:
					send_option = "SO_KEEPALIVE";
					break;
				case SO_BROADCAST:
					send_option = "SO_BROADCAST";
					break;
				default:
					printf("Socket Option error\n");
			}
		}

		else if(level == IPPROTO_IP)
		{
			switch(option)
			{
				case IP_TOS:
					send_option = "IP_TOS";
					break;
				case IP_TTL:
					send_option = "IP_TTL";
					break;
				default:
					printf("Socket Option error\n");
			}
		}

		else if(level == IPPROTO_TCP)
		{
			switch(option)
			{
				case TCP_NODELAY:
					send_option = "TCP_NODELAY";
					break;
				case TCP_MAXSEG:
					send_option = "TCP_MAXSEG";
					break;
				default:
					printf("Socekt Option error\n");
			}
		}

		else
		{
			printf("Socket Protocol level error\n");
		}
		
		if(send_option == NULL)  // if option is strange
		{
			snd_pkt.result = -1;
			sendto(serv_sock, &snd_pkt, sizeof(SO_PACKET), 0, (struct sockaddr*)&clnt_adr,clnt_adr_sz);  
			continue;
		}

		printf("Received Socket option: %s\n", send_option);

		// make send packet
		optlen = sizeof(optval);
		state = getsockopt(sock, rcv_pkt.level, rcv_pkt.option, (void*)&optval, &optlen); 

		if(state == -1)
		{
			printf("get socket option error\n");
			snd_pkt.result = -1;
			sendto(serv_sock, &snd_pkt, sizeof(SO_PACKET), 0, (struct sockaddr*)&clnt_adr,clnt_adr_sz);
			continue;
		}

		snd_pkt.optval = optval;
		snd_pkt.result = state;

		sendto(serv_sock, &snd_pkt, sizeof(SO_PACKET), 0, (struct sockaddr*)&clnt_adr,clnt_adr_sz);  // send

		printf("Send option: %s: %d, result: %d\n\n", send_option, optval, state);
		
	}

	close(serv_sock);

	return 0;
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
