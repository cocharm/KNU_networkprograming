#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

// SIGALRM handler
void parent_alrm(int sig);
void child_alrm(int sig);

// SIGINT handler
void parent_int(int sig);
void child_int(int sig);

// SIGCHLD handler
void read_childproc(int sig);

// error handler
void error_handling(char* message);

// global variable, processes don't share this
int time = 0;
// child process uses this
int cnt = 0;

int main()
{
	int pid = fork();
	if(pid == -1)
		error_handling("fork() error");

	// child process
	if(pid == 0)
	{
		// assign SIGALRM handler
		struct sigaction act_child;

		act_child.sa_handler = child_alrm;
		sigemptyset(&act_child.sa_mask);
		act_child.sa_flags = 0;

		sigaction(SIGALRM, &act_child, NULL);

		// assign SIGINT handler
		struct sigaction dontint;

		dontint.sa_handler = child_int;
		sigemptyset(&dontint.sa_mask);
		dontint.sa_flags = 0;

		sigaction(SIGINT, &dontint, NULL);


		for(int i = 0; i < 5; i++)
		{
			alarm(5);   // every 5 sec, SIGALRM
			sleep(10);  // wakes up by SIGALRM
		}

		return 5;
	}

	// parent process
	else
	{
		struct sigaction act1;
		struct sigaction act2;
		struct sigaction act3;

		// assign signal handlers
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
			sleep(1);  // Infinite Looping
		}
	}

	return 0;
}

// SIGALRM
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
	alarm(2);  // alarm makes alarm
}

// SIGINT
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
		sleep(2);  // don't wake up by SIGINT
}

void child_int(int sig)
{
	sleep(10);  // don't die by SIGINT
	return;
}

// SIGCHLD
void read_childproc(int sig)
{
	int status;
	pid_t id = waitpid(-1, &status, WNOHANG);
	if(WIFEXITED(status))
	{
		printf("Child id = %d, sent: %d\n", id, WEXITSTATUS(status));
	}
}

// error handling
void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

