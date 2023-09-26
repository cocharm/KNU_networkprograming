#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

#define BASIC_BUF 10
#define STANDARD_BUF 100
#define PREMIUM_BUF 1000

#define MAX_CLNT 256

#define MAX_SIZE PREMIUM_BUF

typedef struct {
	int command;
	int type;
	char buf[MAX_SIZE];
	int len;
} PACKET;

void* handle_clnt(void* args);
void error_handling(char* msg);
void close_sock(int sock);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];

pthread_mutex_t mutx;
pthread_mutex_t mutx_file;

int main(int argc, char* argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;

	if(argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");

	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while(1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		// while accept, clnt_adr which acees server is saved to clnt_adr, so it is &clnt_adr
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;  // global variable, critical section
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		// if thread returns, then detach realeases resources
		pthread_detach(t_id);

		printf("Connected Client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}

	close(serv_sock);
	return 0;
}

// the most important part
// which thread do
void* handle_clnt(void* arg)
{
	PACKET send_pack;
	PACKET recv_pack;

	memset(&send_pack, 0, sizeof(PACKET));
	memset(&recv_pack, 0, sizeof(PACKET));

	int clnt_sock = *((int*)arg);
	int str_len = read(clnt_sock, &recv_pack, sizeof(PACKET));

	if(str_len == -1)
		error_handling("read() error");

	if(recv_pack.command != 0)
	{
		printf("packet command is wrong");
		close_sock(clnt_sock);
		return NULL;
	}
		

	int BUF_SIZE;
	
	if(recv_pack.type == 1)
		BUF_SIZE = BASIC_BUF;
	else if(recv_pack.type == 2)
		BUF_SIZE = STANDARD_BUF;
	else if(recv_pack.type == 3)
		BUF_SIZE = PREMIUM_BUF;
	else
	{
		printf("packet type is wrong");
		close_sock(clnt_sock);
		return NULL;
	}

	int type = recv_pack.type;

	// buffer
	char buf[BUF_SIZE];

	// file descriptor is shared
	pthread_mutex_lock(&mutx_file);
	int fd = open("hw09.mp4", O_RDONLY);
	pthread_mutex_unlock(&mutx_file);

	if(fd == -1)
		error_handling("open() error");

	int size;
	int total = 0;
	char* user_type[] = {"", "Basic", "Standard", "Premium"};

	// file transmit
	while(1)
	{
		memset(&send_pack, 0, sizeof(PACKET));
		size = read(fd, buf, BUF_SIZE);
		printf("read size : %d\n", size);
		if(size == -1)
			error_handling("read .mp4 error");

		total += size;
		if(size < BUF_SIZE)
			break;
		send_pack.command = 1;
		send_pack.len = size;
		strcpy(send_pack.buf, buf);
		write(clnt_sock, &send_pack, sizeof(PACKET));
//		printf("send size: %d, send command: %d\n", send_pack.len, send_pack.command);
	}

	// last file transmit
	send_pack.command = 2;
	send_pack.len = size;
	strcpy(send_pack.buf, buf);
	write(clnt_sock, &send_pack, sizeof(PACKET));
//	printf("send size: %d, send command: %d\n", send_pack.len, send_pack.command);

	printf("Total Tx Bytes: %d to Client %d (%s)\n", total, clnt_sock, user_type[type]);
	
	str_len = read(clnt_sock, &recv_pack, sizeof(recv_pack));
	if(str_len == -1)
		error_handling("read() error");

	if(recv_pack.command == 3)
	{
		close_sock(clnt_sock);
		printf("[RX] FILE END ACK from Client %d => client sock: %d closed\n", clnt_sock, clnt_sock);
	}

	else
	{
		close_sock(clnt_sock);
		printf("abnormal close for client %d => client sock: %d closed\n", clnt_sock, clnt_sock);
	}

	pthread_mutex_lock(&mutx_file);
	close(fd);
	pthread_mutex_unlock(&mutx_file);
	
	return NULL;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void close_sock(int sock)
{
	pthread_mutex_lock(&mutx);
	for(int i = 0; i < clnt_cnt; i++)
	{
		if(sock == clnt_socks[i])
		{
			while(i < clnt_cnt)
			{
				clnt_socks[i] = clnt_socks[i + 1];
				i++;
			}
			break;
		}
	}

	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(sock);

	return;
}
