#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "test.h"

#define B(f)
#define T(f) void f();
#include "tests.h"
#undef T

static int failed;
static const char *name;

static int slow;
static int verbose;
static int count;
static int nfailed;

static void errtimer() { error("use *_timer in benchmarks only\n"); }
void start_timer() { errtimer(); }
void stop_timer() { errtimer(); }
void reset_timer() { errtimer(); }

void error__(const char *n, int l, const char *s, ...) {
	va_list ap;

	failed = 1;
	fprintf(stderr, " ERROR %s %s:%d: ", name, n, l);
	va_start(ap, s);
	vfprintf(stderr, s, ap);
	va_end(ap);
}

static void run(const char *n, void (*f)()) {
	pid_t pid;
	int s;

	count++;
	failed = 0;
	name = n;
	if (verbose)
		fprintf(stderr, "running %s:\n", name);

	pid = fork();
	if (pid == 0) {
		/* run test in a child process */
		f();
		exit(failed);
	}

	if (pid == -1)
		error("fork failed: %s\n", strerror(errno));
	else {
		if (waitpid(pid, &s, 0) == -1)
			error("waitpid failed: %s\n", strerror(errno));
		else if (!WIFEXITED(s))
			error("abnormal exit: %s\n", WIFSIGNALED(s) ? strsignal(WTERMSIG(s)) : "(unknown)");
		else
			failed = !!WEXITSTATUS(s);
	}

	if (failed) {
		nfailed++;
		fprintf(stderr, "FAILED %s\n", name);
	} else if (verbose)
		fprintf(stderr, "PASSED %s\n", name);
}

static int summary() {
	fprintf(stderr, "PASS:%d FAIL:%d\n", count-nfailed, nfailed);
	return !!nfailed;
}

static void usage() {
	fprintf(stderr, "usage: ./t [-vs]\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	int c;

	while((c = getopt(argc, argv, "vs")) != -1)
		switch(c) {
		case 'v':
			verbose = 1;
			break;
		case 's':
			slow = 1; /* TODO */
			break;
		default:
			usage();
		}
	if (optind != argc)
		usage();

#define T(t) run(#t, t);
#include "tests.h"
	return summary();
}
