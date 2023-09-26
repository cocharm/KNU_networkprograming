#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

void parent_alrm(int sig);  // parent handles alarm signal
void child_alrm(int sig);  // child handles alarm signal

void parent_int(int sig);  // parent handles interrupt signal
void child_int(int sig);  // child don't want to interrupt

void read_childproc(int sig);  // parent handle child process's return

void error_handling(char* message);  // error handling

int time = 0;  // global variable, processes don't share  this
int cnt = 0; // child process uses this

int main()
{
	int pid = fork();
	if(pid == -1)
		error_handling("fork() error");

	if(pid == 0) // child
	{
		struct sigaction act_child;

		act_child.sa_handler = child_alrm;
		sigemptyset(&act_child.sa_mask);
		act_child.sa_flags = 0;

		sigaction(SIGALRM, &act_child, NULL);

		struct sigaction dontint;

		dontint.sa_handler = child_int;
		sigemptyset(&dontint.sa_mask);
		dontint.sa_flags = 0;

		sigaction(SIGINT, &dontint, NULL);


		for(int i = 0; i < 5; i++)
		{
			alarm(5);
			sleep(10);  // wakes up when get signal alarm
		}

		return 5;
	}

	else  // parent
	{
		struct sigaction act1;
		struct sigaction act2;
		struct sigaction act3;

		// assign signals by sigaction
		act1.sa_handler = parent_alrm;
		sigemptyset(&act1.sa_mask);
		act1.sa_flags = 0;

		act2.sa_handler = parent_int;
		sigemptyset(&act2.sa_mask);
		act2.sa_flags = 0;

		act3.sa_handler = read_childproc;
		sigemptyset(&act3.sa_mask);
		act3.sa_flags = 0;

		sigaction(SIGALRM, &act1, NULL);
		sigaction(SIGINT, &act2, NULL);
		sigaction(SIGCHLD, &act3, NULL);

		alarm(2);
		while(1)
		{
			sleep(1);  // infinite sleep
		}
	}

	return 0;
}

void child_alrm(int sig)
{
	time += 5;
	cnt += 1;
	printf("[Child] time out: 5, elapsed time: %d seconds(%d)\n", time, cnt);
}

void parent_alrm(int sig)
{
	time += 2;
	printf("<Parent> time out: 2, elapsed time: %d seconds\n", time);
	alarm(2);
}

void parent_int(int sig)
{
	printf("Do you want to exit (y or Y to exit)?\n");

	char buf[100]= {0};
	fgets(buf, 100, stdin);

	if(strcmp(buf, "y\n") == 0 || strcmp(buf, "Y\n") == 0)
	{
		exit(0);
	}

	else
		sleep(2);
}

void child_int(int sig)
{
	sleep(10);
	return;
}

void read_childproc(int sig)
{
	int status;
	pid_t id = waitpid(-1, &status, WNOHANG);
	if(WIFEXITED(status))
	{
		printf("Child id = %d, sent: %d\n", id, WEXITSTATUS(status));
	}
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

