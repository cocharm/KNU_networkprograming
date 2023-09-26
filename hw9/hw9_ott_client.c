#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define BASIC_BUF 10
#define STANDARD_BUF 100
#define PREMIUM_BUF 1000

#define MAX_SIZE PREMIUM_BUF

typedef struct {
	int command;
	int type;
	char buf[MAX_SIZE];
	int len;
} PACKET;

void error_handling(char* msg);
void* recv_msg(void* arg);
void print_menu();
void print_menu2();

char message[MAX_SIZE];
PACKET send_packet;
PACKET recv_packet;
int mode;

int main(int argc, char* argv[])
{
	int sock;
	memset(&send_packet, 0, sizeof(send_packet));
	memset(&recv_packet, 0, sizeof(recv_packet));
	struct sockaddr_in serv_addr;
	pthread_t t_id;

	if(argc != 3)
	{
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	int broken = 0;

	// menu
	while(1)
	{
		broken = 0;

		print_menu();
		scanf("%d", &mode);
		if(1 <= mode && mode <= 3)
		{
			int select;
			while(1)
			{
				print_menu2();
				scanf("%d", &select);
				if(select == 1)
				{
					broken = 1;
					break;
				}
				else if(select == 2)
				{
					break;
				}
				else
				{
					printf("You selected wrong number\n");
				}

			}
		}

		// socket is not allocated
		else if(mode == 4)
		{
			printf("Exit Program\n");
			return 0;		
		}

		else
		{
			printf("You selected wrong number\n");
			continue;
		}

		if(broken == 0)
			continue;

		else if(broken == 1)
			break;

		else
		{
			error_handling("Why broken is not 0 or 1?");
		}
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

	pthread_create(&t_id, NULL, recv_msg, &sock);

	pthread_join(t_id, NULL);

	close(sock);
	return 0;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void* recv_msg(void* arg)
{
	struct timespec start, end;
	unsigned long t1, t2;
	const unsigned long nano = 1000000000;

	int sock = *((int*)arg);
	send_packet.command = 0;
	send_packet.type = mode;
	int total = 0;
	int str_len;
	
	str_len = write(sock, &send_packet, sizeof(send_packet));
	if(str_len == -1)
		error_handling("write() error");

	clock_gettime(CLOCK_REALTIME, &start);
	t1 = start.tv_nsec + start.tv_sec * nano;

	while(1)
	{
		memset(&recv_packet, 0, sizeof(PACKET));
		str_len = read(sock, &recv_packet, sizeof(PACKET));
		if(str_len == -1)
			error_handling("read() error");
		total += recv_packet.len;
		printf(".");
//		printf("received size = %d\n", recv_packet.len);
//		printf("received command = %d\n", recv_packet.command);

		if(recv_packet.command == 1)
			continue;
		else if(recv_packet.command == 2)
		{
			printf("FIle transmission Finished\n");
			break;
		}
		else
		{
			printf("Why received packet's command is not 1 or 2?");
			break;
		}
	}

	clock_gettime(CLOCK_REALTIME, &end);
	t2 = end.tv_nsec + end.tv_sec * nano;

	memset(&send_packet, 0, sizeof(send_packet));
	send_packet.command = 3;
	write(sock, &send_packet, sizeof(send_packet));

	printf("Total received: %d\n", total);
	printf("Downloading time: %ld msec\n", (t2 - t1)/1000000);
	printf("Client closed\n");
}

void print_menu()
{
	printf("------------------------------\n");
	printf(" Choose a subscribe type\n");
	printf("------------------------------\n");
	printf("1: Basic, 2: Standard, 3: Premium, 4: quit:  ");
}

void print_menu2()
{
	printf("------------------------------\n");
	printf("1. Download, 2: Back to Main menu :  \n");
}
