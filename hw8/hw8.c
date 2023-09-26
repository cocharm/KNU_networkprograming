#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#define TTL 64
#define BUF_SIZE 120
void error_handling(char* message);

int main(int argc, char* argv[])
{
	int recv_sock;
	int str_len;
	char buf[BUF_SIZE];
	char name[20] = {0};
	struct sockaddr_in adr;
	struct ip_mreq join_adr;

	if(argc != 4) {
		printf("Usage: %s <GroupIP> <PORT> <NAME>\n", argv[0]);
		exit(1);
	}

	name[0] = '[';
	strcat(name, argv[3]);  // name
	strcat(name, "] ");

	pid_t pid = fork();

	// child process
	if(pid == 0)
	{

		recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
		memset(&adr, 0, sizeof(adr));
		adr.sin_family = AF_INET;
		adr.sin_addr.s_addr = htonl(INADDR_ANY);
		adr.sin_port = htons(atoi(argv[2]));

		int optval = 1;
		setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		if(bind(recv_sock, (struct sockaddr*)&adr, sizeof(adr)) == -1)
			error_handling("bind() error");

		// set multicast group ip
		join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]);
		// set my ip
		join_adr.imr_interface.s_addr = htonl(INADDR_ANY);

		// join multicast group
		setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));

		while(1)
		{
			str_len = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0, NULL, 0);
			if(str_len < 0)
				break;
			buf[str_len] = 0;
			fputs(buf, stdout);
		}

		close(recv_sock);
	}

	// parent
	else
	{
		int send_sock;
		struct sockaddr_in mul_adr;
		int time_live = TTL;
		char mess[120];

		send_sock = socket(PF_INET, SOCK_DGRAM, 0);
		memset(&mul_adr, 0, sizeof(mul_adr));

		mul_adr.sin_family = AF_INET;
		mul_adr.sin_addr.s_addr = inet_addr(argv[1]);  // multicast IP
		mul_adr.sin_port = htons(atoi(argv[2])); // multicast port

		setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));

		while(1)
		{
			memset(buf, 0, sizeof(buf));
			fgets(mess, BUF_SIZE, stdin);
			if(strlen(mess) >= 100)
			{
				printf("max message length is 100");
				continue;
			}

			if(strcmp(mess, "q\n") == 0 || strcmp(mess, "Q\n") == 0)
			{
				kill(pid, SIGKILL);
				printf("SIGKILL: Multicast Receiver terminate!\n");
				printf("Multicast Sender(Parent Process) exit!");
				break;
			}

			strcpy(buf, name);
			strcat(buf, mess);
			
			sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&mul_adr, sizeof(mul_adr));
		}

		close(send_sock);
	}


	return 0;
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
