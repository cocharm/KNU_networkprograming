#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 2048

void error_handling(char* message);

int main(int argc, char* argv[])
{
	// connecting clients
	int cnt = 0;
	int clnt1;
	int clnt2;
	// sock
	int serv_sock, clnt_sock;
	// addr
	struct sockaddr_in serv_adr, clnt_adr;
	struct timeval timeout;
	// select set, copy set
	fd_set reads, cpy_reads;

	socklen_t adr_sz;
	int fd_max, str_len, fd_num, i;
	char buf[BUF_SIZE];

	if(argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	// make socket
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket error");
	// initiate serv_addr
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	// bind
	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");

	printf("binding complete!\n");

	// listen
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	printf("Server is ready!!");
	
	// access is also data
	FD_ZERO(&reads);
	FD_SET(serv_sock, &reads);
	fd_max = serv_sock;

	// observate select()'s return value
	while(1)
	{
		// reads is always original
		cpy_reads = reads;
		// have to be set everytime, why?
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		// fd_num is number that socket data received
		if((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout))  == -1)
			break;

		if(fd_num == 0)
			continue;

		// multiflexing
		for(i = 0; i < fd_max + 1; i++)
		{
			// can see this 'i' has received data
			if(FD_ISSET(i, &cpy_reads))
			{
				if(i == serv_sock)  // connection request
				{
					printf("client try to connect!\n");

					adr_sz = sizeof(clnt_adr);
					clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
					if(clnt_sock == -1)
						error_handling("accept() error");

					cnt++;
					if(cnt == 1)
					{
						clnt1 = clnt_sock;
						printf("client1 connected!\n");
					}

					else if (cnt == 2)
					{
						clnt2 = clnt_sock;
						printf("client2 connected!\n");
					}

					else
					{
						printf("Something wrong\n");
						exit(1);
					}
					
					printf("clnt_sock: %d\n", clnt_sock);
					printf("connected client: %d\n", clnt_sock);

					// 2 clients accepted
					if(cnt >= 2)
					{
						FD_SET(clnt1, &reads);
						FD_SET(clnt2, &reads);

						fd_max = clnt2;

						printf("two clients accepted, clnt1 : %d, clnt2: %d, fd_max: %d\n", clnt1, clnt2, fd_max);
					}
				}
				
				// read message, setted fd is not serv_sock, this is clnt1 or clnt2
				else
				{
					memset(buf, 0, BUF_SIZE);
					str_len = read(i, buf, BUF_SIZE);

					if(str_len == -1)
						error_handling("read() error in client");

					if(str_len == 0)
					{
						close(i);
						FD_CLR(i, &reads);
						printf("close client: %d\n", i);
						continue;
					}

					if(i == clnt1)
					{
						str_len = write(clnt2, buf, str_len);
						if(str_len == -1)
							error_handling("write() error with send data to client2");
						printf("[Forward] send data client1(%d) to client2(%d)\n", clnt1, clnt2);
					}
	
					else if(i == clnt2)
					{
						str_len = write(clnt1, buf, str_len);
						if(str_len == -1)
							error_handling("write() error with send data to client1");
						printf("[Backward] send data client2(%d) to client1(%d)\n", clnt2, clnt1);
					}

					else
					{
						printf("Something wrong  with read()\n");
					}
	
				}

			}
		}
	
	}

	return 0;	
}


void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
