#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 10

void error_handling(char* message);

int main(int argc, char* argv[])
{
	if(argc != 3)
		error_handling("mymove Usage: ./mymove src_file dest_file");

	int fd1 = open(argv[1], O_RDONLY);  // file to read
	int fd2 = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);  // file to remove

	if(fd1 == -1 || fd2 == -1)
		error_handling("open error");

	char buf[BUF_SIZE];

	int total = 0;
	int size = 0;

	while(1)
	{
		size = read(fd1, buf, sizeof(buf));
		if(size == 0) break;  // read_size = 0 -> EOF
		if(size == -1)
			error_handling("read error");
		total += size;
		int w_size = write(fd2, buf, size);
		if(w_size == -1)
			error_handling("write error");
	}

	printf("move from %s to %s (bytes: %d) finished.\n", argv[1], argv[2], total);

	close(fd1);
	close(fd2);

	remove(argv[1]);

	return 0;
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);

	exit(1);
}
