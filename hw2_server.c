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


typedef struct {
	int cmd;
	char addr[20];
	struct in_addr iaddr;
	int result;
} PACKET;

void error_handling2(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);

	exit(1);
}

void error_handling(char *message, int clnt_sock)
{
	PACKET error_pk;
	
	error_pk.cmd = 1;
	memset(error_pk.addr, 0, sizeof(error_pk.addr));
	memset(&error_pk.iaddr, 0, sizeof(error_pk.iaddr));
	error_pk.result = 0;

	write(clnt_sock, &error_pk, sizeof(error_pk));
	
	fputs(message, stderr);
	fputc('\n', stderr);

	return;
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
		error_handling2("socket() error");
	
	/* 주소 정보 초기화 */
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);  // 127.0.0.1
	serv_addr.sin_port=htons(atoi(argv[1]));  // port number
	
	/* 주소 정보 할당 */
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1 )
		error_handling2("bind() error"); 
	
	if(listen(serv_sock, 5)==-1)
		error_handling2("listen() error");
	
	printf("---------------------\n");
	printf(" Address Conversion Server\n");
	printf("---------------------\n");

	clnt_addr_size = sizeof(clnt_addr);  
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
	if(clnt_sock==-1)
		error_handling2("accept() error");

	/* socket is file that medium  */
	while(1)
	{
		PACKET pk;
		if(read(clnt_sock, &pk, sizeof(pk)) == -1)
			error_handling("read error!", clnt_sock);
		
		int cmd = pk.cmd;
		char addr[20];
		strcpy(addr, pk.addr);

		if(cmd == QUIT)
		{
			printf("[Rx] Quit message received\n");
			close(clnt_sock);
			close(serv_sock);
			return 0;
		}

		else if (cmd == REQUEST)
		{
			printf("[Rx] Recieved Dotted Decimal Address: %s\n", addr);
		}

		else
		{
			error_handling("cmd error!\n", clnt_sock);
			continue;
		}

		struct in_addr iaddr;
		memset(&iaddr, 0, sizeof(iaddr));
		int result = inet_aton(addr, &iaddr);
		
		pk.cmd = RESPONSE;
		strcpy(pk.addr, "");
		pk.iaddr = iaddr;

		char temp[60];
		sprintf(temp, "[Tx] Address Conversion fail:(%s)\n", addr);

		if(result == 0)
		{
			error_handling(temp, clnt_sock);
		}

		else
		{
			pk.result = SUCCESS;

			printf("inet_aton(%s) -> %#x\n", addr, pk.iaddr.s_addr);

			int bytes = write(clnt_sock, &pk, sizeof(pk));
			if(bytes == -1)
				error_handling2("write() error");

			printf("[Tx] cmd = %d, iaddr: %#x, (result: %d)\n\n", pk.cmd, pk.iaddr.s_addr, pk.result);
			
		}
	}
	
	return 0;
}

