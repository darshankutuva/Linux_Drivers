#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 256

#define RW_DEV_NAME "/dev/rw_dev"

int main()
{
	int status = 0, fd, len, read_len;

	char string[BUFFER_SIZE];

	fd = open(RW_DEV_NAME, O_RDWR);

	if (fd < 0)
	{
		perror("Failed to open rw_dev device");

		status = errno;

		goto Exit;
	}

	printf("Enter a string to write to rw_dev: ");

	scanf("%[^\n]s", &string);

	len = write(fd, string, strlen(string));

	if (len != strlen(string))
	{
		perror("Failed to write string to rw_dev device.\n");

		status = errno;

		goto Exit;
	}

	read_len = read(fd, string, len);

	if (read_len != len)
	{
		perror("Failed to read string from rw_dev device.\n");

		status = errno;

		goto Exit;
	}

	printf("String read from rw_dev = %s\n", string);

Exit:

	close(fd);

	return status;
}
