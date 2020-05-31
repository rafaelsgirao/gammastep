// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2010-2018  Jon Lund Steffensen <jonlst@gmail.com>
// config-ini.h -- INI config file parser header

#ifndef REDSHIFT_CONFIG_INI_H
#define REDSHIFT_CONFIG_INI_H

typedef struct _config_ini_section config_ini_section_t;
typedef struct _config_ini_setting config_ini_setting_t;

struct _config_ini_setting {
	config_ini_setting_t *next;
	char *name;
	char *value;
};

struct _config_ini_section {
	config_ini_section_t *next;
	char *name;
	config_ini_setting_t *settings;
};

typedef struct {
	config_ini_section_t *sections;
} config_ini_state_t;


int config_ini_init(config_ini_state_t *state, const char *filepath);
void config_ini_free(config_ini_state_t *state);

config_ini_section_t *config_ini_get_section(config_ini_state_t *state,
					     const char *name);

#endif /* ! REDSHIFT_CONFIG_INI_H */
