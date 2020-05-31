// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2017  Jon Lund Steffensen <jonlst@gmail.com>
// pipeutils.c -- Utilities for using pipes as signals

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


/* Create non-blocking set of pipe fds. */
int
pipeutils_create_nonblocking(int pipefds[2])
{
	int r = pipe(pipefds);
	if (r == -1) {
		perror("pipe");
		return -1;
	}

	int flags = fcntl(pipefds[0], F_GETFL);
	if (flags == -1) {
		perror("fcntl");
		close(pipefds[0]);
		close(pipefds[1]);
		return -1;
	}

	r = fcntl(pipefds[0], F_SETFL, flags | O_NONBLOCK);
	if (r == -1) {
		perror("fcntl");
		close(pipefds[0]);
		close(pipefds[1]);
		return -1;
	}

	flags = fcntl(pipefds[1], F_GETFL);
	if (flags == -1) {
		perror("fcntl");
		close(pipefds[0]);
		close(pipefds[1]);
		return -1;
	}

	r = fcntl(pipefds[1], F_SETFL, flags | O_NONBLOCK);
	if (r == -1) {
		perror("fcntl");
		close(pipefds[0]);
		close(pipefds[1]);
		return -1;
	}

	return 0;
}

/* Signal on write-end of pipe. */
void
pipeutils_signal(int write_fd)
{
	write(write_fd, "", 1);
}

/* Mark signal as handled on read-end of pipe. */
void
pipeutils_handle_signal(int read_fd)
{
	char data;
	read(read_fd, &data, 1);
}
