# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2010  Francesco Marella <francesco.marella@gmail.com>
# Copyright (c) 2011  Jon Lund Steffensen <jonlst@gmail.com>

"""utils.py -- utility functions"""

import ctypes
import os
import sys

try:
    from xdg import BaseDirectory
    from xdg import DesktopEntry
    has_xdg = True
except ImportError:
    has_xdg = False

from . import defs

# Keys to set when enabling/disabling autostart.
# Only first one is checked on "get".
AUTOSTART_KEYS = (('Hidden', ('true', 'false')),
                  ('X-GNOME-Autostart-enabled', ('false', 'true')))


def open_autostart_file():
    autostart_dir = BaseDirectory.save_config_path("autostart")
    autostart_file = os.path.join(autostart_dir, defs.DESKTOP_FILENAME)

    if not os.path.exists(autostart_file):
        desktop_files = list(
            BaseDirectory.load_data_paths(
                "applications", defs.DESKTOP_FILENAME))

        if not desktop_files:
            raise IOError("Desktop file not found!")

        desktop_file_path = desktop_files[0]

        # Read installed file
        dfile = DesktopEntry.DesktopEntry(desktop_file_path)
        for key, values in AUTOSTART_KEYS:
            dfile.set(key, values[False])
        dfile.write(filename=autostart_file)
    else:
        dfile = DesktopEntry.DesktopEntry(autostart_file)

    return dfile, autostart_file


def supports_autostart():
    return has_xdg


def get_autostart():
    if not has_xdg:
        return False

    dfile, path = open_autostart_file()
    check_key, check_values = AUTOSTART_KEYS[0]
    return dfile.get(check_key) == check_values[True]


def set_autostart(active):
    if not has_xdg:
        return

    dfile, path = open_autostart_file()
    for key, values in AUTOSTART_KEYS:
        dfile.set(key, values[active])
    dfile.write(filename=path)
