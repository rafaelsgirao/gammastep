// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2017  Jon Lund Steffensen <jonlst@gmail.com>
// pipeutils.h -- Utilities for using pipes as signals

#ifndef REDSHIFT_PIPEUTILS_H
#define REDSHIFT_PIPEUTILS_H

int pipeutils_create_nonblocking(int pipefds[2]);

void pipeutils_signal(int write_fd);
void pipeutils_handle_signal(int read_fd);

#endif /* ! REDSHIFT_PIPEUTILS_H */
