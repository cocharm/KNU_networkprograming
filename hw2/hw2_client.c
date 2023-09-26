#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 20

#define ERROR 0
#define SUCCESS 1

#define REQUEST 0
#define RESPONSE 1
#define QUIT 2

void error_handling(char *message);

typedef struct {
	int cmd;
	char addr[20];
	struct in_addr iaddr;
	int result;
} PACKET;

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char message[30];
	int str_len;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
		
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) 
		error_handling("connect() error!");

	// connect success

	while(1)
	{
			PACKET pk;
			memset(&pk, 0, sizeof(pk));

			pk.cmd = REQUEST;
			
			printf("\nInput dotted-decimal address: ");
			scanf("%s", pk.addr);

			if(strcmp(pk.addr, "quit") ==  0)
				pk.cmd = QUIT;

			int bytes = write(sock, &pk, sizeof(pk));

			if(bytes == -1)
				error_handling("write() error");

			if(pk.cmd == QUIT)
			{
				printf("[Tx] cmd: 2(Quit)\n");
				printf("Client socket close and exit\n");
				return 0;
			}

			printf("[Tx] cmd: %d, addr: %s\n", pk.cmd, pk.addr);

			memset(&pk, 0, sizeof(pk));

			bytes = read(sock, &pk, sizeof(pk));

			if(bytes == -1)
				error_handling("read() error");

			if(pk.result == SUCCESS)
			{
				printf("[Rx] cmd: %d, Address conversion: %#x (result: %d)\n", pk.cmd, pk.iaddr.s_addr, pk.result);
			}

			else if (pk.result == ERROR)
			{
				printf("[Rx] cmd: %d,  Address conversion fail! (result: %d)\n", pk.cmd, pk.result);
			}

			else
			{
				printf("What Happend??\n");
			}
	}

	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
