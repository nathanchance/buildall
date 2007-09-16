#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

unsigned int start_time;

static void print_time(void)
{
	int s = time(0) - start_time;

	printf("\b\b\b\b\b\b\b[%02d:%02d]", s / 60, s % 60);
}

static void error(const char *s)
{
	perror(s);
	exit(1);
}

static void sigchld_handler(int signo, siginfo_t *si, void *uc)
{
//	fprintf(stderr, "code %d, status %d\n", si->si_code, si->si_status);
	switch (si->si_code) {
		case CLD_STOPPED:
			fprintf(stderr, "stopped\n");
			break;
		case CLD_CONTINUED:
			fprintf(stderr, "continued\n");
			break;
		default:
			print_time();
			exit(si->si_status);
	}
}

static void init_signals(void)
{
	struct sigaction sa;

	sa.sa_sigaction = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	if (sigaction(SIGCHLD, &sa, 0) < 0)
		error("sigaction()");
}

int main(int argc, char **argv)
{
	int pid, fd;

	if (argc < 4) {
		fprintf(stderr, "Usage: %s <label> <logfile> <cmd> <args>\n",
		        argv[0]);
		return 1;
	}

	init_signals();

	setbuf(stdout, 0);

	start_time = time(0);

	printf(" %s        ", argv[1]);
	print_time();

	switch (pid = fork()) {
		case -1:
			error("fork()");

		case 0:
			fd = creat(argv[2], 0777);
			if (fd < 0)
				error("creat()");

			if (dup2(fd, 1) < 0)
				error("dup2()");

			if (dup2(fd, 2) < 0)
				error("dup2()");

			if (close(fd) < 0)
				error("close()");

			if (execvp(argv[3], argv+3))
				error("execvp()");

		default:
			for (;;) {
				print_time();
				sleep(1);
			}
	}

	return 0;
}
