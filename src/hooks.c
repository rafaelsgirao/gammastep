// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2014  Jon Lund Steffensen <jonlst@gmail.com>
// hooks.c -- Hooks triggered by events

#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>

#include "hooks.h"
#include "redshift.h"

#define MAX_HOOK_PATH  4096
#define CONFIG_DIR "gammastep"


/* Names of periods supplied to scripts. */
static const char *period_names[] = {
	"none",
	"daytime",
	"night",
	"transition"
};


/* Try to open the directory containing hooks. HP is a string
   of MAX_HOOK_PATH length that will be filled with the path
   of the returned directory. */
static DIR *
open_hooks_dir(char *hp)
{
	char *env;

	if ((env = getenv("XDG_CONFIG_HOME")) != NULL &&
	    env[0] != '\0') {
		snprintf(hp, MAX_HOOK_PATH, "%s/" CONFIG_DIR "/hooks", env);
		return opendir(hp);
	}

	if ((env = getenv("HOME")) != NULL &&
	    env[0] != '\0') {
		snprintf(hp, MAX_HOOK_PATH, "%s/.config/" CONFIG_DIR "/hooks", env);
		return opendir(hp);
	}

	struct passwd *pwd = getpwuid(getuid());
	snprintf(hp, MAX_HOOK_PATH, "%s/.config/" CONFIG_DIR "/hooks", pwd->pw_dir);
	return opendir(hp);
}

/* Run hooks with a signal that the period changed. */
void
hooks_signal_period_change(period_t prev_period, period_t period)
{
	char hooksdir_path[MAX_HOOK_PATH];
	DIR *hooks_dir = open_hooks_dir(hooksdir_path);
	if (hooks_dir == NULL) return;

	struct dirent* ent;
	while ((ent = readdir(hooks_dir)) != NULL) {
		/* Skip hidden and special files (., ..) */
		if (ent->d_name[0] == '\0' || ent->d_name[0] == '.') continue;

		char *hook_name = ent->d_name;
		char hook_path[MAX_HOOK_PATH];
		snprintf(hook_path, sizeof(hook_path), "%s/%s",
			 hooksdir_path, hook_name);

		/* Fork and exec the hook. We close stdout
		   so the hook cannot interfere with the normal
		   output. */
		pid_t pid = fork();
		if (pid == (pid_t)-1) {
			perror("fork");
			continue;
		} else if (pid == 0) { /* Child */
			close(STDOUT_FILENO);

			int r = execl(hook_path, hook_name,
				      "period-changed",
				      period_names[prev_period],
				      period_names[period], NULL);
			if (r < 0 && errno != EACCES) perror("execl");

			/* Only reached on error */
			_exit(EXIT_FAILURE);
		}
	}
}
