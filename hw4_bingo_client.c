#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>


#define RAND_START 1
#define RAND_END 30

#define ROW 4
#define COL 4

// result field value
#define FAIL	0
#define SUCCESS 1
#define CHECKED 2

// cmd field value
#define BINGO_REQ	0
#define BINGO_RES	1
#define BINGO_END	2

// Server -> Client send packet
// this is response packet

typedef struct {
	int cmd;  // BINGO_RES, BINGO_END
	int number;  // chosen number by user
	int board[ROW][COL];  // bingo state
	int result;  // result from server (FAIL, SUCCESS, CHECKED)
}RES_PACKET;


// Client -> Server send packet
// this is request packet

typedef struct {
	int cmd;  // BINGO_REQ
	int number;  // random number by user
}REQ_PACKET;

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);

	exit(1);
}

int bingo_array[ROW][COL] = {0};  // random bingo array
int player_choice_array[ROW][COL] = {0};  // Client state
int checked[ROW][COL] = {0};



int main(int argc, char *argv[])
{
	// make socket
	int sock;
	
	RES_PACKET res;
	REQ_PACKET req;

	socklen_t adr_sz;
	struct sockaddr_in serv_adr, from_adr;

	if(argc != 3)
	{
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(sock == -1)
		error_handling("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));

	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));

	srand(time(NULL));

	while(1)
	{
		// make random number
		int num = (rand() % RAND_END) + RAND_START;
		printf("number: %d\n", num);

		// BINGO REQ
		memset(&req, 0, sizeof(req));
		req.number= num;
		req.cmd = BINGO_REQ;
		sendto(sock, &req, sizeof(req), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));

		printf("[Tx] BINGO REQ(number: %d)\n\n", num);

		adr_sz = sizeof(from_adr);

		// BINGO RES
		memset(&res, 0, sizeof(res));
		recvfrom(sock, &res, sizeof(res), 0, (struct sockaddr*)&from_adr, &adr_sz);

		if(res.cmd == BINGO_END)
			printf("[Rx] BINGO END\n");

		else
			printf("[Rx] BINGO_RES(number: %d, result: %d)\n", res.number, res.result);

		for(int i = 0; i < ROW; i++)
		{
			printf("-------------\n");
			printf("|");
			for(int j = 0; j < COL; j++)
			{
				if(res.board[i][j] == 0)
					printf("  |");
				else
					printf("%2d|", res.board[i][j]);
			}
			printf("\n");
		}
		printf("-------------\n");
		printf("\n");

		if(res.cmd == BINGO_END)
		{
			printf("Exit Client\n");
			break;
		}
	}

	close(sock);

	
	return 0;
}

