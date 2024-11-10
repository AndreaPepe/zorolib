#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#include <string.h>
#include <zoro/log.h>
#include <zoro/compiler.h>

#define MAX_STANDARDS 2
#define MAX_BUFFER_SIZE 512

typedef struct log_config {
    uint8_t stdsdup[MAX_STANDARDS];
	int pipes[MAX_STANDARDS][2];
	int custom_stds[MAX_STANDARDS];
	int fd_logfile;
} log_config;

#define __zorolog_close(_fd) do {   \
    if (_fd != -1)                  \
        close(_fd);                 \
    } while (0)

int process_logger(log_config lc){
	char buffer[MAX_BUFFER_SIZE];
	struct pollfd fds[MAX_STANDARDS];
	int ret = 0;

	for (int i = 0; i < MAX_STANDARDS; i++) {
		fds[i].fd = lc.pipes[i][0];
		fds[i].events = POLLIN;
		if (lc.stdsdup[i])
			close(i + 1);
	}

	while (1) {
		ret = poll(fds, MAX_STANDARDS, -1);
		if (ret == -1)
			break;

		for (int i = 0; i < MAX_STANDARDS; i++) {
			if (!(fds[i].revents & POLLIN))
				continue;

			memset(buffer, 0, MAX_BUFFER_SIZE);

			ret = read(lc.pipes[i][0], buffer, MAX_BUFFER_SIZE);
			if (ret <= 0)
				goto child_exit;

			ret = write(lc.custom_stds[i], buffer, strlen(buffer));
			if (ret <= 0)
				goto child_exit;

			ret = write(lc.fd_logfile, buffer, strlen(buffer));
			if (ret <= 0)
				goto child_exit;
		}

		for (int i = 0; i < MAX_STANDARDS; i++) {
			if (fds[i].revents & POLLHUP) {
				ret = 0;
				goto child_exit;
			}
		}
	}

child_exit:
	close(lc.fd_logfile);
	for (int i = 0; i < MAX_STANDARDS; i++) {
		__zorolog_close(lc.pipes[i][0]);
		__zorolog_close(lc.custom_stds[i]);
	}

	return ret;
    return 0;
}

int __zorolog_duplicate(const char *logfile, uint8_t stds, int flags){
    log_config lc;
    int log_flags;

	/* Init to -1 all file descriptors */
	memset(lc.pipes, -1, (MAX_STANDARDS * MAX_STANDARDS) * sizeof(int));
	memset(lc.custom_stds, -1, MAX_STANDARDS * sizeof(int));

	/* Set flags */
	lc.stdsdup[0] = stds & ZOROLOG_DUP_STDOUT;
	lc.stdsdup[1] = stds & ZOROLOG_DUP_STDERR;

	log_flags = O_CREAT | O_RDWR;
	if (flags & ZOROLOG_APPEND)
		log_flags |= O_APPEND;
	else
		log_flags |= O_TRUNC;

	lc.fd_logfile = open(logfile, log_flags, S_IRWXU);
	if (lc.fd_logfile == -1)
		return -1;

	/* Standard duplication management */
	for (int i = 0; i < MAX_STANDARDS; i++) {
		if (lc.stdsdup[i]) {
			/* create a pipe */
			if (pipe(lc.pipes[i]))
				return -1;

			/* duplicate standard file descriptor */
			lc.custom_stds[i] = dup(i + 1);
			if (lc.custom_stds[i] == -1)
				return -1;

			/* redirect standard to write fd of pipe_out */
			if (dup2(lc.pipes[i][1], i + 1) == -1)
				return -1;
		}
	}

	/* Close fd write of both pipes */
	for (int i = 0; i < MAX_STANDARDS; i++)
		__zorolog_close(lc.pipes[i][1]);

	int pid = fork();
	if (pid == -1)
		return -1;

	if (pid == 0) { /* Child process */
		exit(process_logger(lc));
	} else {
		close(lc.fd_logfile);
		for (int i = 0; i < MAX_STANDARDS; i++) {
		    __zorolog_close(lc.pipes[i][0]);
			__zorolog_close(lc.custom_stds[i]);
		}
	}

    return 0;
}
int zorolog_duplicate(const char *logfile, uint8_t stds, int flags){
	if (logfile == NULL)
		return -EINVAL;

	if (stds == 0 || stds > (ZOROLOG_DUP_STDOUT | ZOROLOG_DUP_STDERR))
		return -EINVAL;

	if (flags > ZOROLOG_APPEND)
		return -EINVAL;

	return __zorolog_duplicate(logfile, stds, flags);
}
