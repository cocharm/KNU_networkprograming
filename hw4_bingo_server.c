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
	int serv_sock;
	
	struct sockaddr_in serv_adr;
	struct sockaddr_in clnt_adr;
	socklen_t addr_sz;
	
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);  // clnt_ip, clnt_port
		exit(1);
	}

	// server socket done

	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(serv_sock == -1)
		error_handling("socket error");

	memset(&serv_adr, 0, sizeof(serv_adr));

	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
			error_handling("bind() error");

	
	printf("------------------\n");
	printf(" Bingo Server\n");
	printf("------------------\n");

	// makes random numbers and print
	int cnt = 0;  // chosen number count
	int arr[RAND_END];
	for(int i = 1; i <= RAND_END; i++)
	{
		arr[i - 1] = i;
	}

	srand(time(NULL));

	// make bingo_array
	while (cnt < 16)
	{
		int rand_num = (rand() % (RAND_END - cnt));

		bingo_array[cnt / COL][cnt % COL] = arr[rand_num];
		arr[rand_num] = arr[RAND_END - 1 - cnt];

		cnt += 1;
	}

	// print bingo_array
	printf("this's new bingo!\n");

	for(int i = 0; i < ROW; i++)
	{
		printf("----------------\n");
		printf("|");
		for(int j = 0; j < COL; j++)
		{
			printf("%2d|", bingo_array[i][j]);
		}
		printf("\n");
	}
	printf("----------------");
	printf("\n");

	REQ_PACKET req;
	RES_PACKET res;

	int succ_cnt = 0;
	int turn = 0;

	/* socket is file that medium  */
	while(1)
	{
		// read packet
		memset(&req, 0, sizeof(req));  // initiate req packet

		// need recvfrom
		int clnt_adr_sz = sizeof(clnt_adr); 
		int bytes = recvfrom(serv_sock, &req, sizeof(req), 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

		int cmd = req.cmd;
		int number = req.number;

		printf("[Rx] BINGO_REQ(cmd: %d, number: %d)\n", cmd, number);

		if (cmd != BINGO_REQ)
			error_handling("cmd error");

		// num check
		int result = -1;
		int broken = 0;

		for(int i = 0; i < ROW; i++)
		{
			for(int j = 0; j < COL; j++)
			{
				if(bingo_array[i][j] == number)  // there is number
				{
					if(checked[i][j] == 0)  // not checked number
					{
						checked[i][j] = 1;
						player_choice_array[i][j] = number;
						result = SUCCESS;
						succ_cnt++;  // success count += 1

						printf("Bingo: [%d][%d]: %d\n", i, j, number);
					}

					else
					{
						result = CHECKED;
						printf("Client already chose(num: %d)\n", number);
					}

					broken = 1;
					break;
				}
			}
			
			if(broken)
				break;
		}

		if(!broken)
		{
			result = FAIL;
			printf("Not Found: num: %d\n", number);
		}

		// num check complete
		// send packet

		memset(&res, 0, sizeof(res));

		if(succ_cnt == ROW * COL)
			res.cmd = BINGO_END;
		else
			res.cmd = BINGO_RES;

		res.number = number;

		// board copy
		for(int i = 0; i < ROW; i++)
		{
			for(int j = 0; j < COL; j++)
			{
				res.board[i][j] = player_choice_array[i][j];
			}
		}

		res.result = result;

		if(res.cmd == BINGO_END)
			printf("[Tx] BINGO_END\n");
		else
			printf("[Tx] BINGO RES(cmd: %d, result: %d)\n", res.cmd, res.result);

		// print bingo
		for(int i = 0; i < ROW; i++)
		{
			printf("-------------\n");
			printf("|");
			for(int j = 0; j < COL; j++)
			{
				if(player_choice_array[i][j] == 0)
					printf("  |");
				else
					printf("%2d|", player_choice_array[i][j]);
			}
			printf("\n");
		}
		printf("-------------");
		printf("\n\n");

		sendto(serv_sock, &res,	sizeof(res), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);

		if(res.cmd == BINGO_END)
		{	printf("BINGO END\n");
			break;
		}
	}

	// check

	int broken = 0;

	for(int i = 0; i < ROW; i++)
	{
		for(int j = 0; j < COL; j++)
		{
			if(bingo_array[i][j] != player_choice_array[i][j])
			{
				broken = 1;
				break;
			}
		}

		if(broken)
			break;
	}

	if(broken)
		printf("Wrong number exists\n");

	if(!broken)
		printf("Wrong number not exists, Client Correct!!\n");


	printf("Exit Server\n");
	close(serv_sock);
	
	return 0;
}

