Gammastep
========

Adjust the color temperature of your screen according to
your surroundings. This may help your eyes hurt less if you are
working in front of the screen at night.

Run `gammastep -h` for help on command line options.

A graphical indicator is provided, `gammastep-indicator`.

FAQ
---

**How do I install?**

Use the packages provided by your distribution, if available. Otherwise,
please see _Building from source_ below.

Distribution packaging status:

* Arch Linux: currently in [the community repository](https://www.archlinux.org/packages/community/x86_64/gammastep/)
* NixOS: Not packaged at this time, but there is [a pull request](https://github.com/NixOS/nixpkgs/pull/94565)
* Void Linux: Not packaged at this time, but there is [a pull request](https://github.com/void-linux/void-packages/pull/23643)
* Fedora: spec file in this repo, and also available from [this COPR repo](https://copr.fedorainfracloud.org/coprs/alebastr/sway-extras/)

**How do I setup a configuration file?**

A configuration file is not required but is useful for saving custom
configurations and manually defining the location in case of issues with the
automatic location provider. An example configuration can be found in
[gammastep.conf.sample](gammastep.conf.sample).

The configuration file should be saved in the following location:

- `${XDG_CONFIG_HOME}/gammastep/config.ini`

If `XDG_CONFIG_HOME` is unset, the default of `~/.config` is used.

**Where can I find my coordinates to put in the configuration file?**

There are multiple web sites that provide coordinates for map locations, for
example clicking anywhere on Google Maps will bring up a box with the
coordinates. Remember that longitudes in the western hemisphere (e.g. the
Americas) must be provided as negative numbers.

**Why does GeoClue fail with access denied error?**

It is possible that the location services have been disabled completely. The
check for this case varies by desktop environment. For example, in GNOME the
location services can be toggled in Settings > Privacy > Location Services.

If this is not the case, it is possible that the program has been improperly
installed or not been given the required permissions to obtain location
updates from a system administrator. See
https://github.com/jonls/redshift/issues/318 for further discussion on this
issue.

**Why doesn't this work on my Chromebook/Raspberry Pi?**

Certain video drivers do not support adjustable gamma ramps. In some cases
the program will fail with an error message, but other drivers silently ignore
adjustments to the gamma ramp.

**Why doesn't the backlight change when I use the brightness option?**

There is a brightness adjustment setting but it does not work the way most
people might expect. In fact it is a fake brightness adjustment obtained by
manipulating the gamma ramps which means that it does not reduce the backlight
of the screen. Preferably only use it if your normal backlight adjustment is
too coarse-grained.

**Why doesn't this work with Wayland compositors like Mutter, KWin, Mir, and
Enlightenment?**

This program supports the wlroots protocol for gamma adjustments,
but several Wayland compositors do not support this protocol.
GNOME and KDE have their own built-in mechanisms for adjusting color
temperature on Wayland.

**The redness effect is applied during the day instead of at night. Why?**

This usually happens to users in America when the longitude has been set in the
configuration file to a positive number. Longitudes in the western hemisphere
should be provided as negative numbers (e.g. New York City is at approximately
latitude/longitude 41, -74).

**Why does the redness effect occasionally switch off for a few seconds?**

Some applications (particularly games and video players) will reset the gamma
ramps while this program operates.
After a few seconds, this program will take effect again.

**Why does the redness effect continuously flicker?**

You may have multiple instances running simultaneously. Make sure
that only one instance is running for the display where you are seeing the
flicker.

**Why doesn't the color of the mouse cursor change?**

Mouse cursors are usually handled separately by the graphics hardware and is
not affected by gamma ramps. Some graphics drivers can be configured to use
software cursors instead.

**I have an issue but it was not mentioned in this FAQ. What do I do?**

Please go to [the issue tracker](https://gitlab.com/chinstrap/gammastep/issues) and
check if your issue has already been reported. If not, please open a new issue
describing you problem.


**When running as a systemd service, the connection to Xorg/Wayland fails**

You need to export your environment variables when you window manager or
compositor start up. Typically, you want to run this as part of its startup:

    systemctl --user import-environment; systemctl --user start graphical-session.target

See your compositor's (or window manager's) documentation for further details
of setting up the systemd user session.

Contributing / Building from source
-----------------------------------

See the file [CONTRIBUTING](CONTRIBUTING.md) for more details.
