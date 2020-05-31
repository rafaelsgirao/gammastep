// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2010-2014  Jon Lund Steffensen <jonlst@gmail.com>
// colorramp.h -- color temperature calculation header

#ifndef REDSHIFT_COLORRAMP_H
#define REDSHIFT_COLORRAMP_H

#include <stdint.h>

#include "redshift.h"

void colorramp_fill(uint16_t *gamma_r, uint16_t *gamma_g, uint16_t *gamma_b,
		    int size, const color_setting_t *setting);
void colorramp_fill_float(float *gamma_r, float *gamma_g, float *gamma_b,
			  int size, const color_setting_t *setting);

#endif /* ! REDSHIFT_COLORRAMP_H */
