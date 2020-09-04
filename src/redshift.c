// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2009-2017  Jon Lund Steffensen <jonlst@gmail.com>
// redshift.c -- Main program source

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <errno.h>

#include <poll.h>

#include <signal.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(s) gettext(s)
# define N_(s) (s)
#else
# define _(s) s
# define N_(s) s
# define gettext(s) s
#endif

#include "redshift.h"
#include "config-ini.h"
#include "solar.h"
#include "systemtime.h"
#include "hooks.h"
#include "signals.h"
#include "options.h"
#include "vlog.h"

#include "gamma-dummy.h"

#ifdef ENABLE_DRM
# include "gamma-drm.h"
#endif

#ifdef ENABLE_RANDR
# include "gamma-randr.h"
#endif

#ifdef ENABLE_VIDMODE
# include "gamma-vidmode.h"
#endif

#ifdef ENABLE_WAYLAND
# include "gamma-wl.h"
#endif


#include "location-manual.h"

#ifdef ENABLE_GEOCLUE2
# include "location-geoclue2.h"
#endif

#undef CLAMP
#define CLAMP(lo,mid,up)  (((lo) > (mid)) ? (lo) : (((mid) < (up)) ? (mid) : (up)))


/* Bounds for parameters. */
#define MIN_LAT   -90.0
#define MAX_LAT    90.0
#define MIN_LON  -180.0
#define MAX_LON   180.0
#define MIN_TEMP   1000
#define MAX_TEMP  25000
#define MIN_BRIGHTNESS  0.1
#define MAX_BRIGHTNESS  1.0
#define MIN_GAMMA   0.1
#define MAX_GAMMA  10.0

/* Duration of sleep between screen updates (milliseconds). */
#define SLEEP_DURATION        5000
#define SLEEP_DURATION_SHORT  100

/* Length of fade in numbers of short sleep durations. */
#define FADE_LENGTH  40


/* Names of periods of day */
static const char *period_names[] = {
	/* TRANSLATORS: Name printed when period of day is unknown */
	N_("None"),
	N_("Daytime"),
	N_("Night"),
	N_("Transition")
};


/* Determine which period we are currently in based on time offset. */
static period_t
get_period_from_time(const transition_scheme_t *transition, int time_offset)
{
	if (time_offset < transition->dawn.start ||
	    time_offset >= transition->dusk.end) {
		return PERIOD_NIGHT;
	} else if (time_offset >= transition->dawn.end &&
		   time_offset < transition->dusk.start) {
		return PERIOD_DAYTIME;
	} else {
		return PERIOD_TRANSITION;
	}
}

/* Determine which period we are currently in based on solar elevation. */
static period_t
get_period_from_elevation(
	const transition_scheme_t *transition, double elevation)
{
	if (elevation < transition->low) {
		return PERIOD_NIGHT;
	} else if (elevation < transition->high) {
		return PERIOD_TRANSITION;
	} else {
		return PERIOD_DAYTIME;
	}
}

/* Determine how far through the transition we are based on time offset. */
static double
get_transition_progress_from_time(
	const transition_scheme_t *transition, int time_offset)
{
	if (time_offset < transition->dawn.start ||
	    time_offset >= transition->dusk.end) {
		return 0.0;
	} else if (time_offset < transition->dawn.end) {
		return (transition->dawn.start - time_offset) /
			(double)(transition->dawn.start -
				transition->dawn.end);
	} else if (time_offset > transition->dusk.start) {
		return (transition->dusk.end - time_offset) /
			(double)(transition->dusk.end -
				transition->dusk.start);
	} else {
		return 1.0;
	}
}

/* Determine how far through the transition we are based on elevation. */
static double
get_transition_progress_from_elevation(
	const transition_scheme_t *transition, double elevation)
{
	if (elevation < transition->low) {
		return 0.0;
	} else if (elevation < transition->high) {
		return (transition->low - elevation) /
			(transition->low - transition->high);
	} else {
		return 1.0;
	}
}

/* Return number of seconds since midnight from timestamp. */
static int
get_seconds_since_midnight(double timestamp)
{
	time_t t = (time_t)timestamp;
	struct tm tm;
	localtime_r(&t, &tm);
	return tm.tm_sec + tm.tm_min * 60 + tm.tm_hour * 3600;
}

/* Log description of the given period. */
static void
print_period(period_t period, double transition)
{
	const char *p = _("Period");
	switch (period) {
	case PERIOD_NONE:
	case PERIOD_NIGHT:
	case PERIOD_DAYTIME:
		vlog_notice("%s: %s", p, gettext(period_names[period]));
		break;
	case PERIOD_TRANSITION:
		vlog_notice("%s: %s (%s: %.2f%%)",
		            p, gettext(period_names[period]),
		            _("Day"), transition*100);
		break;
	}
}

/* Print location */
static void
print_location(const location_t *location)
{
	/* TRANSLATORS: Abbreviation for `north' */
	const char *north = _("N");
	/* TRANSLATORS: Abbreviation for `south' */
	const char *south = _("S");
	/* TRANSLATORS: Abbreviation for `east' */
	const char *east = _("E");
	/* TRANSLATORS: Abbreviation for `west' */
	const char *west = _("W");

	vlog_notice("%s: %.2f %s, %.2f %s", _("Location"),
	            fabs(location->lat), location->lat >= 0.f ? north : south,
	            fabs(location->lon), location->lon >= 0.f ? east : west);
}

/* Interpolate color setting structs given alpha. */
static void
interpolate_color_settings(
	const color_setting_t *first,
	const color_setting_t *second,
	double alpha,
	color_setting_t *result)
{
	alpha = CLAMP(0.0, alpha, 1.0);

	result->temperature = (1.0-alpha)*first->temperature +
		alpha*second->temperature;
	result->brightness = (1.0-alpha)*first->brightness +
		alpha*second->brightness;
	for (int i = 0; i < 3; i++) {
		result->gamma[i] = (1.0-alpha)*first->gamma[i] +
			alpha*second->gamma[i];
	}
}

/* Interpolate color setting structs transition scheme. */
static void
interpolate_transition_scheme(
	const transition_scheme_t *transition,
	double alpha,
	color_setting_t *result)
{
	const color_setting_t *day = &transition->day;
	const color_setting_t *night = &transition->night;

	alpha = CLAMP(0.0, alpha, 1.0);
	interpolate_color_settings(night, day, alpha, result);
}

/* Return 1 if color settings have major differences, otherwise 0.
   Used to determine if a fade should be applied in continual mode. */
static int
color_setting_diff_is_major(
	const color_setting_t *first,
	const color_setting_t *second)
{
	return (abs(first->temperature - second->temperature) > 25 ||
		fabsf(first->brightness - second->brightness) > 0.1 ||
		fabsf(first->gamma[0] - second->gamma[0]) > 0.1 ||
		fabsf(first->gamma[1] - second->gamma[1]) > 0.1 ||
		fabsf(first->gamma[2] - second->gamma[2]) > 0.1);
}

/* Reset color setting to default values. */
static void
color_setting_reset(color_setting_t *color)
{
	color->temperature = NEUTRAL_TEMP;
	color->gamma[0] = 1.0;
	color->gamma[1] = 1.0;
	color->gamma[2] = 1.0;
	color->brightness = 1.0;
}


static int
provider_try_start(const location_provider_t *provider,
		   location_state_t **state, config_ini_state_t *config,
		   char *args)
{
	int r;

	r = provider->init(state);
	if (r < 0) {
		vlog_err("%s: %s", _("Failed to initialize provider"), provider->name);
		return -1;
	}

	/* Set provider options from config file. */
	config_ini_section_t *section =
		config_ini_get_section(config, provider->name);
	if (section != NULL) {
		config_ini_setting_t *setting = section->settings;
		while (setting != NULL) {
			r = provider->set_option(*state, setting->name,
						 setting->value);
			if (r < 0) {
				provider->free(*state);
				vlog_err("%s: %s", _("Failed to set option"),
				         provider->name);
				vlog_err("%s '-l %s:help'",
				         _("For more information, use option:"),
					 provider->name);
				return -1;
			}
			setting = setting->next;
		}
	}

	/* Set provider options from command line. */
	const char *manual_keys[] = { "lat", "lon" };
	int i = 0;
	while (args != NULL) {
		char *next_arg = strchr(args, ':');
		if (next_arg != NULL) *(next_arg++) = '\0';

		const char *key = args;
		char *value = strchr(args, '=');
		if (value == NULL) {
			/* The options for the "manual" method can be set
			   without keys on the command line for convencience
			   and for backwards compatability. We add the proper
			   keys here before calling set_option(). */
			if (strcmp(provider->name, "manual") == 0 &&
			    i < sizeof(manual_keys)/sizeof(manual_keys[0])) {
				key = manual_keys[i];
				value = args;
			} else {
				vlog_err("%s: '%s'", _("Failed to parse option"), args);
				return -1;
			}
		} else {
			*(value++) = '\0';
		}

		r = provider->set_option(*state, key, value);
		if (r < 0) {
			provider->free(*state);
			vlog_err("%s: %s", _("Failed to set option"), provider->name);
			vlog_err("%s '-l %s:help'",
			         _("For more information, use option:"),
			         provider->name);
			return -1;
		}

		args = next_arg;
		i += 1;
	}

	/* Start provider. */
	r = provider->start(*state);
	if (r < 0) {
		provider->free(*state);
		vlog_err("%s: %s", _("Failed to start provider"), provider->name);
		return -1;
	}

	return 0;
}

static int
method_try_start(const gamma_method_t *method,
		 gamma_state_t **state, config_ini_state_t *config, char *args)
{
	int r;

	r = method->init(state);
	if (r < 0) {
		vlog_err("%s: %s", _("Failed to initialize method"), method->name);
		return -1;
	}

	/* Set method options from config file. */
	config_ini_section_t *section =
		config_ini_get_section(config, method->name);
	if (section != NULL) {
		config_ini_setting_t *setting = section->settings;
		while (setting != NULL) {
			r = method->set_option(
				*state, setting->name, setting->value);
			if (r < 0) {
				method->free(*state);
				vlog_err("%s: %s", _("Failed to set option"), method->name);
				vlog_err("%s '-m %s: help'", _("For more information, try:"),
				         method->name);
				return -1;
			}
			setting = setting->next;
		}
	}

	/* Set method options from command line. */
	while (args != NULL) {
		char *next_arg = strchr(args, ':');
		if (next_arg != NULL) *(next_arg++) = '\0';

		const char *key = args;
		char *value = strchr(args, '=');
		if (value == NULL) {
			vlog_err("%s: %s", _("Failed to parse option"), args);
			return -1;
		} else {
			*(value++) = '\0';
		}

		r = method->set_option(*state, key, value);
		if (r < 0) {
			method->free(*state);
			vlog_err("%s: %s", _("Failed to set option"), method->name);
			vlog_err("%s '-m %s: help'", _("For more information, try:"),
			         method->name);
			return -1;
		}

		args = next_arg;
	}

	/* Start method. */
	r = method->start(*state);
	if (r < 0) {
		method->free(*state);
		vlog_err("%s: %s", _("Failed to start adjustment method"), method->name);
		return -1;
	}

	return 0;
}


/* Check whether gamma is within allowed levels. */
static int
gamma_is_valid(const float gamma[3])
{
	return !(gamma[0] < MIN_GAMMA ||
		 gamma[0] > MAX_GAMMA ||
		 gamma[1] < MIN_GAMMA ||
		 gamma[1] > MAX_GAMMA ||
		 gamma[2] < MIN_GAMMA ||
		 gamma[2] > MAX_GAMMA);
}

/* Check whether location is valid.
   Prints error message on stderr and returns 0 if invalid, otherwise
   returns 1. */
static int
location_is_valid(const location_t *location)
{
	/* Latitude */
	if (location->lat < MIN_LAT || location->lat > MAX_LAT) {
		vlog_err("%s [%.1f, %.1f]", _("Latitude must be in range"), 
		         MIN_LAT, MAX_LAT);
		return 0;
	}

	/* Longitude */
	if (location->lon < MIN_LON || location->lon > MAX_LON) {
		vlog_err("%s [%.1f, %.1f]", _("Longitude must be in range:"),
		         MIN_LON, MAX_LON);
		return 0;
	}

	return 1;
}

/* Wait for location to become available from provider.
   Waits until timeout (milliseconds) has elapsed or forever if timeout
   is -1. Writes location to loc. Returns -1 on error,
   0 if timeout was reached, 1 if location became available. */
static int
provider_get_location(
	const location_provider_t *provider, location_state_t *state,
	int timeout, location_t *loc)
{
	int available = 0;
	struct pollfd pollfds[1];
	while (!available) {
		int loc_fd = provider->get_fd(state);
		if (loc_fd >= 0) {
			/* Provider is dynamic. */
			/* TODO: This should use a monotonic time source. */
			double now;
			int r = systemtime_get_time(&now);
			if (r < 0) {
				vlog_err(_("Unable to read system time."));
				return -1;
			}

			/* Poll on file descriptor until ready. */
			pollfds[0].fd = loc_fd;
			pollfds[0].events = POLLIN;
			r = poll(pollfds, 1, timeout);
			if (r < 0) {
				perror("poll");
				return -1;
			} else if (r == 0) {
				return 0;
			}

			double later;
			r = systemtime_get_time(&later);
			if (r < 0) {
				vlog_err(_("Unable to read system time."));
				return -1;
			}

			/* Adjust timeout by elapsed time */
			if (timeout >= 0) {
				timeout -= (later - now) * 1000;
				timeout = timeout < 0 ? 0 : timeout;
			}
		}


		int r = provider->handle(state, loc, &available);
		if (r < 0) return -1;
	}

	return 1;
}

/* Easing function for fade.
   See https://github.com/mietek/ease-tween */
static double
ease_fade(double t)
{
	if (t <= 0) return 0;
	if (t >= 1) return 1;
	return 1.0042954579734844 * exp(
		-6.4041738958415664 * exp(-7.2908241330981340 * t));
}


/* Run continual mode loop
   This is the main loop of the continual mode which keeps track of the
   current time and continuously updates the screen to the appropriate
   color temperature. */
static int
run_continual_mode(const location_provider_t *provider,
		   location_state_t *location_state,
		   const transition_scheme_t *scheme,
		   const gamma_method_t *method,
		   gamma_state_t *method_state,
		   int use_fade, int preserve_gamma)
{
	int r;

	/* Short fade parameters */
	int fade_length = 0;
	int fade_time = 0;
	color_setting_t fade_start_interp;

	r = signals_install_handlers();
	if (r < 0) {
		return r;
	}

	/* Save previous parameters so we can avoid printing status updates if
	   the values did not change. */
	period_t prev_period = PERIOD_NONE;

	/* Previous target color setting and current actual color setting.
	   Actual color setting takes into account the current color fade. */
	color_setting_t prev_target_interp;
	color_setting_reset(&prev_target_interp);

	color_setting_t interp;
	color_setting_reset(&interp);

	location_t loc = { NAN, NAN };
	int need_location = !scheme->use_time;
	if (need_location) {
		vlog_info(_("Waiting for initial location to become available..."));

		/* Get initial location from provider */
		r = provider_get_location(provider, location_state, -1, &loc);
		if (r < 0) {
			vlog_err(_("Unable to get location from provider."));
			return -1;
		}

		if (!location_is_valid(&loc)) {
			vlog_err(_("Invalid location returned from provider."));
			return -1;
		}

		print_location(&loc);
	}

	vlog_notice("%s: %uK", _("Color temperature"), interp.temperature);
	vlog_notice("%s: %.2f", _("Brightness"), interp.brightness);

	/* Continuously adjust color temperature */
	int done = 0;
	int prev_disabled = 1;
	int disabled = 0;
	int location_available = 1;
	int should_reset = 0;
	while (1) {
		/* Check to see if disable signal was caught */
		if (disable && !done) {
			disabled = !disabled;
			disable = 0;
		}

		/* Check to see if exit signal was caught */
		if (exiting) {
			if (done) {
				/* On second signal stop the ongoing fade. */
				break;
			} else {
				done = 1;
				disabled = 1;
			}
			exiting = 0;
		}

		/* Print status change */
		if (disabled != prev_disabled) {
			vlog_notice("%s: %s", _("Status"), disabled ? _("Disabled") : _("Enabled"));
			should_reset = 1;
		}

		prev_disabled = disabled;

		/* Read timestamp */
		double now;
		r = systemtime_get_time(&now);
		if (r < 0) {
			vlog_err(_("Unable to read system time."));
			return -1;
		}

		period_t period;
		double transition_prog;
		if (scheme->use_time) {
			int time_offset = get_seconds_since_midnight(now);

			period = get_period_from_time(scheme, time_offset);
			transition_prog = get_transition_progress_from_time(
				scheme, time_offset);
		} else {
			/* Current angular elevation of the sun */
			double elevation = solar_elevation(
				now, loc.lat, loc.lon);

			period = get_period_from_elevation(scheme, elevation);
			transition_prog =
				get_transition_progress_from_elevation(
					scheme, elevation);
		}

		/* Use transition progress to get target color
		   temperature. */
		color_setting_t target_interp;
		interpolate_transition_scheme(
			scheme, transition_prog, &target_interp);

		if (disabled) {
			vlog_debug("Currently disabled");
			period = PERIOD_NONE;
			color_setting_reset(&target_interp);
		}

		if (done) {
			period = PERIOD_NONE;
		}

		/* Print period if it changed during this update,
		   or if we are in the transition period. In transition we
		   print the progress, so we always print it in
		   that case. */
		if (period != prev_period || period == PERIOD_TRANSITION) {
			print_period(period, transition_prog);
		}

		/* Activate hooks if period changed */
		if (period != prev_period) {
			hooks_signal_period_change(prev_period, period);
		}

		/* Start fade if the parameter differences are too big to apply
		   instantly. */
		if (use_fade) {
			if ((fade_length == 0 &&
			     color_setting_diff_is_major(
				     &interp,
				     &target_interp)) ||
			    (fade_length != 0 &&
			     color_setting_diff_is_major(
				     &target_interp,
				     &prev_target_interp))) {
				fade_length = FADE_LENGTH;
				fade_time = 0;
				fade_start_interp = interp;
			}
		}

		/* Handle ongoing fade */
		if (fade_length != 0) {
			fade_time += 1;
			double frac = fade_time / (double)fade_length;
			double alpha = CLAMP(0.0, ease_fade(frac), 1.0);

			interpolate_color_settings(
				&fade_start_interp, &target_interp, alpha,
				&interp);

			if (fade_time > fade_length) {
				fade_time = 0;
				fade_length = 0;
			}
		} else {
			interp = target_interp;
		}

		/* Break loop when done and final fade is over */
		if (done && fade_length == 0) break;

		if (prev_target_interp.temperature != target_interp.temperature) {
			vlog_notice("%s: %uK", _("Color temperature"), target_interp.temperature);
		}
		if (prev_target_interp.brightness != target_interp.brightness) {
			vlog_notice("%s: %.2f", _("Brightness"), target_interp.brightness);
		}

		/* Adjust temperature */
		if (!disabled || should_reset) {
			vlog_debug("Adjusting temperature");
			r = method->set_temperature(
				method_state, &interp, preserve_gamma);
			if (r < 0) {
				vlog_err(_("Temperature adjustment failed."));
				return -1;
			}
		}

		/* Save period and target color setting as previous */
		prev_period = period;
		prev_target_interp = target_interp;

		if (interp.temperature == target_interp.temperature &&
		    interp.brightness == target_interp.brightness) {
			/* All resets have been done now */
			should_reset = 0;
		}

		/* Sleep length depends on whether a fade is ongoing. */
		int delay = SLEEP_DURATION;
		if (fade_length != 0) {
			delay = SLEEP_DURATION_SHORT;
		}

		/* Update location. */
		int loc_fd = -1;
		if (need_location) {
			loc_fd = provider->get_fd(location_state);
		}

		if (loc_fd >= 0) {
			/* Provider is dynamic. */
			struct pollfd pollfds[1];
			pollfds[0].fd = loc_fd;
			pollfds[0].events = POLLIN;
			int r = poll(pollfds, 1, delay);
			if (r < 0) {
				if (errno == EINTR) continue;
				vlog_err("poll: %m");
				vlog_err(_("Unable to get location from provider."));
				return -1;
			} else if (r == 0) {
				continue;
			}

			/* Get new location and availability
			   information. */
			location_t new_loc;
			int new_available;
			r = provider->handle(
				location_state, &new_loc,
				&new_available);
			if (r < 0) {
				vlog_err(_("Unable to get location from provider."));
				return -1;
			}

			if (!new_available &&
			    new_available != location_available) {
				vlog_err(_("Location is temporarily"
				         " unavailable; Using previous"
				         " location until it becomes"
				         " available..."));
			}

			if (new_available &&
			    (new_loc.lat != loc.lat ||
			     new_loc.lon != loc.lon ||
			     new_available != location_available)) {
				loc = new_loc;
				print_location(&loc);
			}

			location_available = new_available;

			if (!location_is_valid(&loc)) {
				vlog_err(_("Invalid location returned from provider."));
				return -1;
			}
		} else {
			systemtime_msleep(delay);
		}
	}

	/* Restore saved gamma ramps */
	method->restore(method_state);

	return 0;
}


int
main(int argc, char *argv[])
{
	int r;

#ifdef ENABLE_NLS
	/* Init locale */
	setlocale(LC_CTYPE, "");
	setlocale(LC_MESSAGES, "");

	/* Internationalisation */
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	/* List of gamma methods. */
	const gamma_method_t gamma_methods[] = {
#ifdef ENABLE_WAYLAND
		wl_gamma_method,
#endif
#ifdef ENABLE_DRM
		drm_gamma_method,
#endif
#ifdef ENABLE_RANDR
		randr_gamma_method,
#endif
#ifdef ENABLE_VIDMODE
		vidmode_gamma_method,
#endif
		dummy_gamma_method,
		{ NULL }
	};

	/* List of location providers. */
	const location_provider_t location_providers[] = {
#ifdef ENABLE_GEOCLUE2
		geoclue2_location_provider,
#endif
		manual_location_provider,
		{ NULL }
	};

	/* Flush messages consistently even if redirected to a pipe or
	   file.  Change the flush behaviour to line-buffered, without
	   changing the actual buffers being used. */
	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	options_t options;
	options_init(&options);
	options_parse_args(
		&options, argc, argv, gamma_methods, location_providers);
	vlog_lvl_set((vlog_lvl_t) options.verbose);

	/* Load settings from config file. */
	config_ini_state_t config_state;
	r = config_ini_init(&config_state, options.config_filepath);
	if (r < 0) {
		vlog_err("Unable to load config file.");
		exit(EXIT_FAILURE);
	}

	free(options.config_filepath);

	options_parse_config_file(
		&options, &config_state, gamma_methods, location_providers);

	options_set_defaults(&options);

	if (options.scheme.dawn.start >= 0 || options.scheme.dawn.end >= 0 ||
	    options.scheme.dusk.start >= 0 || options.scheme.dusk.end >= 0) {
		if (options.scheme.dawn.start < 0 ||
		    options.scheme.dawn.end < 0 ||
		    options.scheme.dusk.start < 0 ||
		    options.scheme.dusk.end < 0) {
			vlog_err(_("Partial time-configuration unsupported!"));
			exit(EXIT_FAILURE);
		}

		if (options.scheme.dawn.start > options.scheme.dawn.end ||
		    options.scheme.dawn.end > options.scheme.dusk.start ||
		    options.scheme.dusk.start > options.scheme.dusk.end) {
			vlog_err(_("Invalid dawn/dusk time configuration!"));
			exit(EXIT_FAILURE);
		}

		options.scheme.use_time = 1;
	}

	/* Initialize location provider if needed. If provider is NULL
	   try all providers until one that works is found. */
	location_state_t *location_state;

	/* Location is not needed for reset mode and manual mode. */
	int need_location =
		options.mode != PROGRAM_MODE_RESET &&
		options.mode != PROGRAM_MODE_MANUAL &&
		!options.scheme.use_time;
	if (need_location) {
		if (options.provider != NULL) {
			/* Use provider specified on command line. */
			r = provider_try_start(
				options.provider, &location_state,
				&config_state, options.provider_args);
			if (r < 0) exit(EXIT_FAILURE);
		} else {
			/* Try all providers, use the first that works. */
			for (int i = 0;
			     location_providers[i].name != NULL; i++) {
				const location_provider_t *p =
					&location_providers[i];
				vlog_info("%s: '%s'", _("Trying location provider"),
				          p->name);
				r = provider_try_start(p, &location_state,
						       &config_state, NULL);
				if (r < 0) {
					vlog_info(_("Trying next provider..."));
					continue;
				}

				/* Found provider that works. */
				vlog_info("%s '%s'", _("Using provider:"), p->name);
				options.provider = p;
				break;
			}

			/* Failure if no providers were successful at this
			   point. */
			if (options.provider == NULL) {
				vlog_err(_("No more location providers to try."));
				exit(EXIT_FAILURE);
			}
		}

		/* Solar elevations */
		if (options.scheme.high < options.scheme.low) {
			vlog_err(
				_("High transition elevation cannot be lower than"
				  " the low transition elevation."));
			exit(EXIT_FAILURE);
		}

		vlog_notice("%s: > %.1f (%s), < %.1f (%s)",
		            _("Solar elevations"),
		            options.scheme.high, _("Day"),
		            options.scheme.low, _("Night"));
	}

	if (options.mode != PROGRAM_MODE_RESET &&
	    options.mode != PROGRAM_MODE_MANUAL) {
		vlog_notice("%s: %dK (%s), %dK (%s)",
		            _("Temperatures"),
		            options.scheme.day.temperature, _("Day"),
		            options.scheme.night.temperature, _("Night"));

		/* Color temperature */
		if (options.scheme.day.temperature < MIN_TEMP ||
		    options.scheme.day.temperature > MAX_TEMP ||
		    options.scheme.night.temperature < MIN_TEMP ||
		    options.scheme.night.temperature > MAX_TEMP) {
			vlog_err("%s [%uK, %uK]",
			         _("Temperature must be in range"),
			         MIN_TEMP, MAX_TEMP);
			exit(EXIT_FAILURE);
		}
	}

	if (options.mode == PROGRAM_MODE_MANUAL) {
		/* Check color temperature to be set */
		if (options.temp_set < MIN_TEMP ||
		    options.temp_set > MAX_TEMP) {
			vlog_err("%s [%uK, %uK]",
			         _("Temperature must be in range"),
			         MIN_TEMP, MAX_TEMP);
			exit(EXIT_FAILURE);
		}
	}

	/* Brightness */
	if (options.scheme.day.brightness < MIN_BRIGHTNESS ||
	    options.scheme.day.brightness > MAX_BRIGHTNESS ||
	    options.scheme.night.brightness < MIN_BRIGHTNESS ||
	    options.scheme.night.brightness > MAX_BRIGHTNESS) {
		vlog_err("%s [%.1f, %.1f]", _("Brightness must be in range"),
		         MIN_BRIGHTNESS, MAX_BRIGHTNESS);
		exit(EXIT_FAILURE);
	}

	vlog_notice("%s: %.2f:%.2f",
	            _("Brightness"),
	            options.scheme.day.brightness,
	            options.scheme.night.brightness);

	/* Gamma */
	if (!gamma_is_valid(options.scheme.day.gamma) ||
	    !gamma_is_valid(options.scheme.night.gamma)) {
		vlog_err("%s [%.1f, %.1f]", _("Gamma value must be in range"),
		         MIN_GAMMA, MAX_GAMMA);
		exit(EXIT_FAILURE);
	}

	/* TRANSLATORS: The string in parenthesis is either
	   Daytime or Night (translated). */
	vlog_notice("%s (%s): %.3f, %.3f, %.3f",
	            _("Gamma"), _("Day"), options.scheme.day.gamma[0],
	            options.scheme.day.gamma[1],
	            options.scheme.day.gamma[2]);
	vlog_notice("%s (%s): %.3f, %.3f, %.3f",
	            _("Gamma"), _("Night"), options.scheme.night.gamma[0],
	            options.scheme.night.gamma[1],
	            options.scheme.night.gamma[2]);

	transition_scheme_t *scheme = &options.scheme;

	/* Initialize gamma adjustment method. If method is NULL
	   try all methods until one that works is found. */
	gamma_state_t *method_state;

	/* Gamma adjustment not needed for print mode */
	if (options.mode != PROGRAM_MODE_PRINT) {
		if (options.method != NULL) {
			/* Use method specified on command line. */
			r = method_try_start(
				options.method, &method_state, &config_state,
				options.method_args);
			if (r < 0) exit(EXIT_FAILURE);
		} else {
			/* Try all methods, use the first that works. */
			for (int i = 0; gamma_methods[i].name != NULL; i++) {
				const gamma_method_t *m = &gamma_methods[i];
				if (!m->autostart) continue;

				r = method_try_start(
					m, &method_state, &config_state, NULL);
				if (r < 0) {
					vlog_info(_("Trying next method..."));
					continue;
				}

				/* Found method that works. */
				vlog_info("%s: '%s'", _("Using method"), m->name);
				options.method = m;
				break;
			}

			/* Failure if no methods were successful at this point. */
			if (options.method == NULL) {
				vlog_err(_("No more methods to try."));
				exit(EXIT_FAILURE);
			}
		}
	}

	config_ini_free(&config_state);

	switch (options.mode) {
	case PROGRAM_MODE_ONE_SHOT:
	case PROGRAM_MODE_PRINT:
	{
		location_t loc = { NAN, NAN };
		if (need_location) {
			vlog_info(_("Waiting for current location to become available..."));

			/* Wait for location provider. */
			int r = provider_get_location(
				options.provider, location_state, -1, &loc);
			if (r < 0) {
				vlog_err(_("Unable to get location from provider."));
				exit(EXIT_FAILURE);
			}

			if (!location_is_valid(&loc)) {
				vlog_err(_("Invalid location from provider."));
				exit(EXIT_FAILURE);
			}

			print_location(&loc);
		}

		double now;
		r = systemtime_get_time(&now);
		if (r < 0) {
			vlog_err(_("Unable to read system time."));
			options.method->free(method_state);
			exit(EXIT_FAILURE);
		}

		period_t period;
		double transition_prog;
		if (options.scheme.use_time) {
			int time_offset = get_seconds_since_midnight(now);
			period = get_period_from_time(scheme, time_offset);
			transition_prog = get_transition_progress_from_time(
				scheme, time_offset);
		} else {
			/* Current angular elevation of the sun */
			double elevation = solar_elevation(
				now, loc.lat, loc.lon);
			vlog_notice("%s: %f", _("Solar elevation"), elevation);

			period = get_period_from_elevation(scheme, elevation);
			transition_prog =
				get_transition_progress_from_elevation(
					scheme, elevation);
		}

		/* Use transition progress to set color temperature */
		color_setting_t interp;
		interpolate_transition_scheme(
			scheme, transition_prog, &interp);

		vlog_lvl_t saved_lvl = vlog_lvl_get();
		if (options.mode == PROGRAM_MODE_PRINT) {
			vlog_lvl_set(VLOG_LVL_NOTICE);
		}
		print_period(period, transition_prog);
		vlog_notice("%s: %uK", _("Color temperature"), interp.temperature);
		vlog_notice("%s: %.2f", _("Brightness"), interp.brightness);
		vlog_lvl_set(saved_lvl);

		if (options.mode != PROGRAM_MODE_PRINT) {
			/* Adjust temperature */
			r = options.method->set_temperature(
				method_state, &interp, options.preserve_gamma);
			if (r < 0) {
				vlog_err(_("Temperature adjustment failed."));
				options.method->free(method_state);
				exit(EXIT_FAILURE);
			}

			/* wlroots gamma adjustments automatically revert when
			 * the process exits, so we wait until interrupt. */
			if (strcmp(options.method->name, "wayland") == 0) {
				vlog_notice(_("Press ctrl-c to stop..."));
				pause();
			}
		}
	}
	break;
	case PROGRAM_MODE_MANUAL:
	{
		vlog_notice("%s: %uK", _("Color temperature"), options.temp_set);

		/* Adjust temperature */
		color_setting_t manual = scheme->day;
		manual.temperature = options.temp_set;
		r = options.method->set_temperature(
			method_state, &manual, options.preserve_gamma);
		if (r < 0) {
			vlog_err(_("Temperature adjustment failed."));
			options.method->free(method_state);
			exit(EXIT_FAILURE);
		}

		/* wlroots gamma adjustments automatically revert when
		 * the process exits, so we wait until interrupt. */
		if (strcmp(options.method->name, "wayland") == 0) {
			vlog_notice(_("Press ctrl-c to stop..."));
			pause();
		}
	}
	break;
	case PROGRAM_MODE_RESET:
	{
		/* Reset screen */
		color_setting_t reset;
		color_setting_reset(&reset);

		r = options.method->set_temperature(method_state, &reset, 0);
		if (r < 0) {
			vlog_err(_("Temperature adjustment failed."));
			options.method->free(method_state);
			exit(EXIT_FAILURE);
		}

		/* wlroots gamma adjustments automatically revert when
		 * the process exits, so we wait until interrupt. */
		if (strcmp(options.method->name, "wayland") == 0) {
			vlog_notice(_("Press ctrl-c to stop..."));
			pause();
		}
	}
	break;
	case PROGRAM_MODE_CONTINUAL:
	{
		r = run_continual_mode(
			options.provider, location_state, scheme,
			options.method, method_state,
			options.use_fade, options.preserve_gamma);
		if (r < 0) exit(EXIT_FAILURE);
	}
	break;
	}

	/* Clean up gamma adjustment state */
	if (options.mode != PROGRAM_MODE_PRINT) {
		options.method->free(method_state);
	}

	/* Clean up location provider state */
	if (need_location) {
		options.provider->free(location_state);
	}

	return EXIT_SUCCESS;
}
