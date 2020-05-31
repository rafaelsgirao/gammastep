// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2009-2015  Jon Lund Steffensen <jonlst@gmail.com>
// Copyright (c) 2015  Mattias Andrée <maandree@member.fsf.org>
// signals.c -- Signal processing source

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <signal.h>

#include "signals.h"


volatile sig_atomic_t exiting = 0;
volatile sig_atomic_t disable = 0;


/* Signal handler for exit signals */
static void
sigexit(int signo)
{
	exiting = 1;
}

/* Signal handler for disable signal */
static void
sigdisable(int signo)
{
	disable = 1;
}

int
signals_install_handlers(void)
{
	struct sigaction sigact;
	sigset_t sigset;
	int r;
	sigemptyset(&sigset);

	/* Install signal handler for INT and TERM signals */
	sigact.sa_handler = sigexit;
	sigact.sa_mask = sigset;
	sigact.sa_flags = 0;

	r = sigaction(SIGINT, &sigact, NULL);
	if (r < 0) {
		perror("sigaction");
		return -1;
	}

	r = sigaction(SIGTERM, &sigact, NULL);
	if (r < 0) {
		perror("sigaction");
		return -1;
	}

	/* Install signal handler for USR1 signal */
	sigact.sa_handler = sigdisable;
	sigact.sa_mask = sigset;
	sigact.sa_flags = 0;

	r = sigaction(SIGUSR1, &sigact, NULL);
	if (r < 0) {
		perror("sigaction");
		return -1;
	}

	/* Ignore CHLD signal. This causes child processes
	   (hooks) to be reaped automatically. */
	sigact.sa_handler = SIG_IGN;
	sigact.sa_mask = sigset;
	sigact.sa_flags = 0;

	r = sigaction(SIGCHLD, &sigact, NULL);
	if (r < 0) {
		perror("sigaction");
		return -1;
	}

	return 0;
}
