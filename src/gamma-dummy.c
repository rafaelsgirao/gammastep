// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2013-2017  Jon Lund Steffensen <jonlst@gmail.com>
// gamma-dummy.c -- No-op gamma adjustment

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(s) gettext(s)
#else
# define _(s) s
#endif

#include "redshift.h"


static int
gamma_dummy_init(void **state)
{
	*state = NULL;
	return 0;
}

static int
gamma_dummy_start(void *state)
{
	fputs(_("WARNING: Using dummy gamma method! Display will not be affected by this gamma method.\n"), stderr);
	return 0;
}

static void
gamma_dummy_restore(void *state)
{
}

static void
gamma_dummy_free(void *state)
{
}

static void
gamma_dummy_print_help(FILE *f)
{
	fputs(_("Does not affect the display but prints the color temperature to the terminal.\n"), f);
	fputs("\n", f);
}

static int
gamma_dummy_set_option(void *state, const char *key, const char *value)
{
	fprintf(stderr, _("Unknown method parameter: `%s'.\n"), key);
	return -1;
}

static int
gamma_dummy_set_temperature(
	void *state, const color_setting_t *setting, int preserve)
{
	printf(_("Temperature: %i\n"), setting->temperature);
	return 0;
}


const gamma_method_t dummy_gamma_method = {
	"dummy", 0,
	(gamma_method_init_func *)gamma_dummy_init,
	(gamma_method_start_func *)gamma_dummy_start,
	(gamma_method_free_func *)gamma_dummy_free,
	(gamma_method_print_help_func *)gamma_dummy_print_help,
	(gamma_method_set_option_func *)gamma_dummy_set_option,
	(gamma_method_restore_func *)gamma_dummy_restore,
	(gamma_method_set_temperature_func *)gamma_dummy_set_temperature
};
