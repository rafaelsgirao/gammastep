// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2010  Jon Lund Steffensen <jonlst@gmail.com>
// solar.h -- Solar position header

#ifndef REDSHIFT_SOLAR_H
#define REDSHIFT_SOLAR_H

#include "time.h"

/* Model of atmospheric refraction near horizon (in degrees). */
#define SOLAR_ATM_REFRAC  0.833

#define SOLAR_ASTRO_TWILIGHT_ELEV   -18.0
#define SOLAR_NAUT_TWILIGHT_ELEV    -12.0
#define SOLAR_CIVIL_TWILIGHT_ELEV    -6.0
#define SOLAR_DAYTIME_ELEV           (0.0 - SOLAR_ATM_REFRAC)

typedef enum {
	SOLAR_TIME_NOON = 0,
	SOLAR_TIME_MIDNIGHT,
	SOLAR_TIME_ASTRO_DAWN,
	SOLAR_TIME_NAUT_DAWN,
	SOLAR_TIME_CIVIL_DAWN,
	SOLAR_TIME_SUNRISE,
	SOLAR_TIME_SUNSET,
	SOLAR_TIME_CIVIL_DUSK,
	SOLAR_TIME_NAUT_DUSK,
	SOLAR_TIME_ASTRO_DUSK,
	SOLAR_TIME_MAX
} solar_time_t;


double solar_elevation(double date, double lat, double lon);
void solar_table_fill(double date, double lat, double lon, double *table);

#endif /* ! REDSHIFT_SOLAR_H */
