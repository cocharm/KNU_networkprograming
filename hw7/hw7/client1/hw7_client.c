#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE 2048

void error_handling(char* message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char buf[BUF_SIZE];
	int str_len = 0;
	int fd_max = 2;
	struct timeval timeout;

	int chosen;

	if(argc != 3)
	{
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	printf("-------------------------------\n");
	printf("Choose function:\n");
	printf("1: Sender  2: Receiver\n");
	printf("-------------------------------\n");

	scanf("%d", &chosen);

	if(chosen != 1 && chosen != 2)
	{
		printf("Enter the number 1 or 2\n");
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock ==  -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	printf("Trying connect!!\n");

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error!");

	printf("Connect complete!!\n");

	fd_max = (fd_max < sock)? sock : fd_max;

	fd_set reads, temps;
	FD_ZERO(&reads);
	FD_SET(sock, &reads);
	
	// Sender
	// read from file, send to socket
	// read from server, print this
	if(chosen == 1)
	{
		int fd = open("rfc1180.txt", O_RDONLY);
		if(fd == -1)
			error_handling("open() error");
		fd_max= (fd_max < fd)? fd : fd_max;
		int result;

		FD_SET(fd, &reads);
		
		while(1)
		{
			temps = reads;

			// initiation everytime
			timeout.tv_sec = 3;
			timeout.tv_usec = 0;

			result = select(fd_max + 1, &temps, 0, 0, &timeout);

			if(result == -1)
			{
				puts("select() error!");
				break;
			}

			else if(result == 0)
			{
				puts("Time-out!");
			}

			else
			{
				for(int i = 0; i <= fd_max; i++)
				{
					if(FD_ISSET(i, &temps))
					{
						memset(buf, 0, BUF_SIZE);
						// sock, print
						if(i == sock)
						{
							str_len = read(i, buf, BUF_SIZE);
							if(str_len == -1)
								error_handling("read() error");
							printf("%s", buf);
						}

						// fd
						else if(i == fd)
						{
							str_len = read(i, buf, BUF_SIZE);
							sleep(1);
							if(str_len == -1)
								error_handling("read() error");
							// open file close
							if(str_len == 0)
							{
								close(fd);
								FD_CLR(fd, &reads);

								if(fd_max == fd)
									fd_max = sock;

								continue;
							}
							write(sock, buf, str_len);
						}

						else
						{
							printf("Why another socket catched?\n");
						}
					}
				}
			}
		}
	}

	else if(chosen == 2)
	{
		int result;	
		while(1)
		{
			temps = reads;

			// initiation everytime
			timeout.tv_sec = 3;
			timeout.tv_usec = 0;

			result = select(fd_max + 1, &temps, 0, 0, &timeout);

			if(result == -1)
			{
				puts("select() error!");
				break;
			}

			else if(result == 0)
			{
				puts("Time-out!");
			}

			else
			{
				for(int i = 0; i <= fd_max; i++)
				{
					memset(buf, 0, BUF_SIZE);
					if(FD_ISSET(i, &temps))
					{
						// sock, read and send back to server
						if(i == sock)
						{
							str_len = read(i, buf, BUF_SIZE);
							printf("%s", buf);
							write(sock, buf, str_len);
						}

						else
						{
							printf("Why another socket catched?\n");
						}
					}
				}
			}
		}
					
	}

	// error
	else
	{
		error_handling("chosen error");
	}


	
	
	return 0;
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

