// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2014  Jon Lund Steffensen <jonlst@gmail.com>
// hooks.h -- Hooks triggered by events

#ifndef REDSHIFT_HOOKS_H
#define REDSHIFT_HOOKS_H

#include "redshift.h"

void hooks_signal_period_change(period_t prev_period,
				period_t period);


#endif /* ! REDSHIFT_HOOKS_H */
