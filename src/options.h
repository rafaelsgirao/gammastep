// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2017  Jon Lund Steffensen <jonlst@gmail.com>
// options.h -- Program options header

#ifndef REDSHIFT_OPTIONS_H
#define REDSHIFT_OPTIONS_H

#include "redshift.h"

typedef struct {
	/* Path to config file */
	char *config_filepath;

	transition_scheme_t scheme;
	program_mode_t mode;
	int verbose;

	/* Temperature to set in manual mode. */
	int temp_set;
	/* Whether to fade between large skips in color temperature. */
	int use_fade;
	/* Whether to preserve gamma ramps if supported by gamma method. */
	int preserve_gamma;

	/* Selected gamma method. */
	const gamma_method_t *method;
	/* Arguments for gamma method. */
	char *method_args;

	/* Selected location provider. */
	const location_provider_t *provider;
	/* Arguments for location provider. */
	char *provider_args;
} options_t;


void options_init(options_t *options);
void options_parse_args(
	options_t *options, int argc, char *argv[],
	const gamma_method_t *gamma_methods,
	const location_provider_t *location_providers);
void options_parse_config_file(
	options_t *options, config_ini_state_t *config_state,
	const gamma_method_t *gamma_methods,
	const location_provider_t *location_providers);
void options_set_defaults(options_t *options);

#endif /* ! REDSHIFT_OPTIONS_H */
