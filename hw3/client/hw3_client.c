#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
#define SEQ_START 1000

#define ERROR 0
#define SUCCESS 1

#define REQUEST 0
#define RESPONSE 1
#define QUIT 2

int cur_seq = SEQ_START;

void error_handling(char *message);

typedef struct {
	int seq;
	int ack;
	int buf_len;
	char buf[BUF_SIZE];
} Packet;

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

	// input file name

	printf("Input file name: ");
	char name[100];
	scanf("%s", name);

	printf("[Client] request %s\n\n", name);

	Packet pk;
	memset(&pk, 0, sizeof(pk));

	pk.ack = cur_seq;  // first is 1000
	strcpy(pk.buf, name);

	write(sock, &pk, sizeof(pk));

	// make file to copy
	int fd = open(name, O_RDWR | O_CREAT, 0644);

	while(1)
	{
			memset(&pk, 0, sizeof(pk));

			read(sock, &pk, sizeof(pk));  // read packet

			// case1 file is not exist
			// seq is -1
			if(pk.seq == -1)
			{
				close(fd);
				remove(name);

				error_handling("File Not Found");
			}

			// case2 file transmit is done
			else if(pk.buf_len < BUF_SIZE)
			{
				printf("[Client] Rx SEQ: %d, len: %d bytes\n", pk.seq, pk.buf_len);

				write(fd, pk.buf, pk.buf_len);
				cur_seq += pk.buf_len;

				break;
			}

			// case3 file transmit is working
			else
			{
				printf("[Client] Rx SEQ: %d, len: %d bytes\n", pk.seq, pk.buf_len);

				write(fd, pk.buf, pk.buf_len);
				cur_seq += BUF_SIZE;

				memset(&pk, 0, sizeof(pk));
				pk.ack = cur_seq;
				pk.buf_len = BUF_SIZE;
				write(sock, &pk, sizeof(pk));


				printf("[Client] Tx ACK: %d\n\n", pk.ack);
			}
	}

	printf("%s received (%d Bytes)\n", name, cur_seq - 1000);

	close(fd);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
