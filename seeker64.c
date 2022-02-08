#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

#define BLOCKSIZE 512
#define TIMEOUT 30

int count;
time_t start;

off64_t maxoffset = 0;
off64_t minoffset = 249994674176000;

void done()
{
	time_t end;

	time(&end);

	if (end < start + TIMEOUT) {
		printf(".");
		alarm(1);
		return;
	}

	if (count) {
		printf
		    (".\nResults: %d seeks/second, %.2f ms random access time (%lu < offsets < %lu)\n",
		     count / TIMEOUT, 1000.0 * TIMEOUT / count, minoffset,
		     maxoffset);
	}
	exit(EXIT_SUCCESS);
}

void handle(const char *string, int error)
{
	if (error) {
		perror(string);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	char buffer[BLOCKSIZE];
	int fd, retval;
	unsigned long numblocks, numbytes;
	off64_t offset;

	setvbuf(stdout, NULL, _IONBF, 0);

	printf("Seeker v2.0, 2007-01-15, "
	       "http://www.linuxinsight.com/how_fast_is_your_disk.html\n");

	if (argc != 2) {
		printf("Usage: seeker <raw disk device>\n");
		exit(EXIT_SUCCESS);
	}

	fd = open(argv[1], O_RDONLY | O_LARGEFILE);
	handle("open", fd < 0);

	retval = ioctl(fd, BLKGETSIZE, &numblocks);
	handle("ioctl", retval == -1);

	numbytes = numblocks * BLOCKSIZE;
	printf
	    ("Benchmarking %s [%lu blocks, %lu bytes, %lu GiB], wait %d seconds\n",
	     argv[1], numblocks, numbytes, numbytes / 1000000000L, TIMEOUT);

	time(&start);
	srand(start);
	signal(SIGALRM, &done);
	alarm(1);

	for (;;) {
//              offset = (off64_t) ( (unsigned long) numbytes * (random() / (RAND_MAX + 1.0) ));
		offset =
		    (off64_t) ((unsigned long)numblocks *
			       (random() / (double)RAND_MAX));
		//printf("DEBUG: go %lu\n", offset);
//              retval = lseek64(fd, offset, SEEK_SET);
		retval = lseek64(fd, BLOCKSIZE * offset, SEEK_SET);
		handle("lseek64", retval == (off64_t) - 1);
		retval = read(fd, buffer, BLOCKSIZE);
		handle("read", retval < 0);
		count++;
		if (offset > maxoffset)
			maxoffset = offset;
		if (offset < minoffset)
			minoffset = offset;
	}
	/* notreached */
}
