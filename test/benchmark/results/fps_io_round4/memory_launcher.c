#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int main(int argc, char **argv) {
	if (argc != 2) return 2;
	pid_t pid = fork();
	if (pid < 0) return 3;
	if (!pid) { execl(argv[1], argv[1], (char *)0); _exit(127); }
	int status;
	struct rusage usage;
	while (wait4(pid, &status, 0, &usage) < 0) if (errno != EINTR) return 4;
	fprintf(stderr, "{\"maxrss_kib\":%ld,\"minflt\":%ld,\"majflt\":%ld}\n", usage.ru_maxrss, usage.ru_minflt, usage.ru_majflt);
	return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}
