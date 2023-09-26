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

int cur_seq = 1000;

typedef struct {
	int seq;  // SEQ number
	int ack;  // ACK number, ISN is 1000
	int buf_len;  // File read/write bytes
	char buf[BUF_SIZE];
} Packet;

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);

	exit(1);
}


void receivePacket(int clnt_sock, Packet* packet)
{
	if(read(clnt_sock, packet, sizeof(Packet)) == -1)
		error_handling("read error!");

	if(packet->ack > 1000)
		printf("[Server] Rx ACK: %d\n\n", packet->ack);

	return;
}

int sendPacket(int fd, int clnt_sock, Packet* packet)
{

		int bytes = read(fd, packet->buf, BUF_SIZE);
		if(bytes == -1)
		{
				packet->seq = -1;

				// sizeof(packet) is 8bytes
				write(clnt_sock, packet, sizeof(Packet));
				error_handling("read error");
		}
				
		else if(bytes < BUF_SIZE)  // finished sending file
		{
			packet->buf_len = bytes;
			packet->seq = cur_seq;

			write(clnt_sock, packet, sizeof(Packet));

			printf("[Server] Tx: SEQ: %d, %d byte data\n", cur_seq, bytes);

			cur_seq += bytes;

			return 0; // finished
		}

		else
		{
			packet->buf_len = bytes;
			packet->seq = cur_seq;

			write(clnt_sock, packet, sizeof(Packet));
			
			printf("[Server] Tx: SEQ: %d, %d byte data\n", cur_seq, bytes);
			
			cur_seq += BUF_SIZE;
			return 1; // not finished
		}
}


int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
	
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
	
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	/* 서버 소켓(리스닝 소켓) 생성 */
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);  // SOCK_STREAM means TCP
	if(serv_sock == -1)
		error_handling("socket() error");
	
	/* 주소 정보 초기화 */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);  // 127.0.0.1
	serv_addr.sin_port=htons(atoi(argv[1]));  // port number
	
	/* 주소 정보 할당 */
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1 )
		error_handling("bind() error"); 
	
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	printf("---------------------\n");
	printf(" File Transmission Server\n");
	printf("---------------------\n");

	clnt_addr_size = sizeof(clnt_addr);  
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error");

	int fd;
	char name[BUF_SIZE];

	/* socket is file that medium  */
	while(1)
	{
		Packet rcv_pk;
		memset(&rcv_pk, 0, sizeof(rcv_pk));  // initiate receive packet

		Packet snd_pk;
		memset(&snd_pk, 0, sizeof(snd_pk));
		
		receivePacket(clnt_sock, &rcv_pk);  // initiate receive packet
		
		int ack = rcv_pk.ack;

		if(ack == 1000)  // check file name
		{
			strcpy(name, rcv_pk.buf);  // copy file name

			fd = open(name, O_RDONLY);

			if(fd == -1) // file not exist
			{
				snd_pk.seq = -1;  // file not exist

				int bytes = write(clnt_sock, &snd_pk, sizeof(snd_pk));
				if(bytes == -1)
					error_handling("write error");

				char error_message[100];
				strcpy(error_message, name);
				strcat(error_message, " File Not Found");

				error_handling(error_message);
			}

			else
			{
				printf("[Server] sending %s\n\n", name);

				int status = sendPacket(fd, clnt_sock, &snd_pk);
				
				// after sending final data, break
				if(status == 0) break;
			}
		}

		else  // if ack is not 1000
		{
			int status = sendPacket(fd, clnt_sock, &snd_pk);

			if(status == 0) break;
		}
	}

	printf("%s sent (%d Bytes)\n", name, cur_seq - 1000);

	close(clnt_sock);
	close(serv_sock);
	
	return 0;
}

