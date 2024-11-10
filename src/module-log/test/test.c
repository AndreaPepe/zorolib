#include <stdio.h>
#include <string.h>
#include <zoro/log.h>

#define PATH_LOGFILE "./logfile"

int main(int argc, char *argv[]){
	uint8_t flags = 0;

	if (argc < 2) {
		zorolog_info("Usage: %s <stds>\n", argv[0]);
		zorolog_info("\t* 1: duplicate only standard output\n");
		zorolog_info("\t* 2: duplicate only standard error\n");
		zorolog_info("\t* 3: duplicate both standards\n");
		return -EINVAL;
	}

	switch (atoi(argv[1])) {
		case 1:
			flags = ZOROLOG_DUP_STDOUT;
			break;
		case 2:
			flags = ZOROLOG_DUP_STDERR;
			break;
		case 3:
			flags = ZOROLOG_DUP_STDOUT | ZOROLOG_DUP_STDERR;
			break;
		default:
			return -EINVAL;
	}

	int ret = zorolog_duplicate(PATH_LOGFILE, flags, 0);
	if (ret) {
		zorolog_error("Standard output duplication failed\n");
		return -1;
	}

	write(STDOUT_FILENO, "output msg\n", strlen("output msg\n"));
	write(STDERR_FILENO, "error msg\n", strlen("error msg\n"));
	write(STDOUT_FILENO, "output msg 2\n", strlen("output msg 2\n"));
	write(STDERR_FILENO, "error msg 2\n", strlen("error msg 2\n"));

	return 0;
}
