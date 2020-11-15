#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([gammastep], [2.0.5], [https://gitlab.com/chinstrap/gammastep/issues])
AC_CONFIG_SRCDIR([src/redshift.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC_C99
AC_PROG_LIBTOOL
AC_LANG([C])

AC_PROG_INTLTOOL([0.50])

AC_CANONICAL_HOST


# Checks for libraries.
AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])

GETTEXT_PACKAGE=gammastep
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Package name for gettext])


PKG_CHECK_MODULES([DRM], [libdrm], [have_drm=yes], [have_drm=no])

PKG_CHECK_MODULES([X11], [x11], [have_x11=yes], [have_x11=no])
PKG_CHECK_MODULES([XF86VM], [xxf86vm], [have_xf86vm=yes], [have_xf86vm=no])
PKG_CHECK_MODULES([XCB], [xcb], [have_xcb=yes], [have_xcb=no])
PKG_CHECK_MODULES([XCB_RANDR], [xcb-randr],
	[have_xcb_randr=yes], [have_xcb_randr=no])

PKG_CHECK_MODULES([WAYLAND], [wayland-client wayland-scanner >= 1.15.0],
	[have_wayland=yes], [have_wayland=no])
PKG_CHECK_VAR(WAYLAND_SCANNER, wayland-scanner, wayland_scanner)

PKG_CHECK_MODULES([GLIB], [glib-2.0 gobject-2.0], [have_glib=yes], [have_glib=no])
PKG_CHECK_MODULES([GEOCLUE2], [glib-2.0 gio-2.0 >= 2.26], [have_geoclue2=yes], [have_geoclue2=no])

# Check for Python
AM_PATH_PYTHON([3.2], [have_python=yes], [have_python=no])

# Check DRM method
AC_MSG_CHECKING([whether to enable DRM method])
AC_ARG_ENABLE([drm], [AC_HELP_STRING([--enable-drm],
	[enable DRM method])],
	[enable_drm=$enableval],[enable_drm=maybe])
AS_IF([test "x$enable_drm" != xno], [
	AS_IF([test $have_drm = yes], [
		AC_DEFINE([ENABLE_DRM], 1,
			[Define to 1 to enable DRM method])
		AC_MSG_RESULT([yes])
		enable_drm=yes
	], [
		AC_MSG_RESULT([missing dependencies])
		AS_IF([test "x$enable_drm" = xyes], [
			AC_MSG_ERROR([missing dependencies for DRM method])
		])
		enable_drm=no
	])
], [
	AC_MSG_RESULT([no])
	enable_drm=no
])
AM_CONDITIONAL([ENABLE_DRM], [test "x$enable_drm" = xyes])

# Check Wayland method
AC_MSG_CHECKING([whether to enable Wayland method])
AC_ARG_ENABLE([wayland], [AC_HELP_STRING([--enable-wayland],
	[enable Wayland method])],
	[enable_wayland=$enableval],[enable_wayland=maybe])
AS_IF([test "x$enable_wayland" != xno], [
	AS_IF([test $have_wayland = yes], [
		AC_DEFINE([ENABLE_WAYLAND], 1,
			[Define to 1 to enable Wayland method])
		AC_MSG_RESULT([yes])
		enable_wayland=yes
	], [
		AC_MSG_RESULT([missing dependencies])
		AS_IF([test "x$enable_wayland" = xyes], [
			AC_MSG_ERROR([missing dependencies for Wayland method])
		])
		enable_wayland=no
	])
], [
	AC_MSG_RESULT([no])
	enable_wayland=no
])
AM_CONDITIONAL([ENABLE_WAYLAND], [test "x$enable_wayland" = xyes])

# Check RANDR method
AC_MSG_CHECKING([whether to enable RANDR method])
AC_ARG_ENABLE([randr], [AC_HELP_STRING([--enable-randr],
	[enable RANDR method])],
	[enable_randr=$enableval],[enable_randr=maybe])
AS_IF([test "x$enable_randr" != xno], [
	AS_IF([test $have_xcb = yes -a $have_xcb_randr = yes], [
		AC_DEFINE([ENABLE_RANDR], 1,
			[Define to 1 to enable RANDR method])
		AC_MSG_RESULT([yes])
		enable_randr=yes
	], [
		AC_MSG_RESULT([missing dependencies])
		AS_IF([test "x$enable_randr" = xyes], [
			AC_MSG_ERROR([missing dependencies for RANDR method])
		])
		enable_randr=no
	])
], [
	AC_MSG_RESULT([no])
	enable_randr=no
])
AM_CONDITIONAL([ENABLE_RANDR], [test "x$enable_randr" = xyes])

# Check VidMode method
AC_MSG_CHECKING([whether to enable VidMode method])
AC_ARG_ENABLE([vidmode], [AC_HELP_STRING([--enable-vidmode],
	[enable VidMode method])],
	[enable_vidmode=$enableval],[enable_vidmode=maybe])
AS_IF([test "x$enable_vidmode" != xno], [
	AS_IF([test $have_x11 = yes -a $have_xf86vm = yes], [
		AC_DEFINE([ENABLE_VIDMODE], 1,
			[Define to 1 to enable VidMode method])
		AC_MSG_RESULT([yes])
		enable_vidmode=yes
	], [
		AC_MSG_RESULT([missing dependencies])
		AS_IF([test "x$enable_vidmode" = xyes], [
			AC_MSG_ERROR([missing dependencies for VidMode method])
		])
		enable_vidmode=no
	])
], [
	AC_MSG_RESULT([no])
	enable_vidmode=no
])
AM_CONDITIONAL([ENABLE_VIDMODE], [test "x$enable_vidmode" = xyes])


# Check Geoclue2 location provider
AC_MSG_CHECKING([whether to enable Geoclue2 location provider])
AC_ARG_ENABLE([geoclue2], [AC_HELP_STRING([--enable-geoclue2],
	[enable Geoclue2 location provider])],
	[enable_geoclue2=$enableval],[enable_geoclue2=maybe])
AS_IF([test "x$enable_geoclue2" != xno], [
	AS_IF([test "x$have_geoclue2" = xyes], [
		AC_DEFINE([ENABLE_GEOCLUE2], 1,
			[Define to 1 to enable Geoclue2 location provider])
		AC_MSG_RESULT([yes])
		enable_geoclue2=yes
	], [
		AC_MSG_RESULT([missing dependencies])
		AS_IF([test "x$enable_geoclue2" = xyes], [
			AC_MSG_ERROR([missing dependencies for Geoclue2 location provider])
		])
		enable_geoclue2=no
	])
], [
	AC_MSG_RESULT([no])
	enable_geoclue2=no
])
AM_CONDITIONAL([ENABLE_GEOCLUE2], [test "x$enable_geoclue2" = xyes])


# Check for GUI status icon
AC_MSG_CHECKING([whether to enable GUI status icon])
AC_ARG_ENABLE([gui], [AC_HELP_STRING([--enable-gui],
	[enable GUI status icon])],
	[enable_gui=$enableval],[enable_gui=maybe])
AS_IF([test "x$enable_gui" != xno], [
	AS_IF([test $have_python = yes], [
		AC_MSG_RESULT([yes])
		enable_gui=yes
	], [
		AC_MSG_RESULT([missing dependencies])
		AS_IF([test "x$enable_gui" = xyes], [
			AC_MSG_ERROR([GUI status icon script requires Python])
		])
		enable_gui=no
	])
], [
	AC_MSG_RESULT([no])
	enable_gui=no
])
AM_CONDITIONAL([ENABLE_GUI], [test "x$enable_gui" != xno])


# Check for systemd
PKG_PROG_PKG_CONFIG
AC_MSG_CHECKING([Directory to install systemd user unit files])
AC_ARG_WITH([systemduserunitdir],
            [AS_HELP_STRING([--with-systemduserunitdir=<dir>],
                            [Directory for systemd user unit files])],
            [], [with_systemduserunitdir=$($PKG_CONFIG --variable=systemduserunitdir systemd)])
AS_IF([test -n "$with_systemduserunitdir" -a "x$with_systemduserunitdir" != xno], [
	AC_SUBST([systemduserunitdir], [$with_systemduserunitdir])
	AC_MSG_RESULT([$systemduserunitdir])
	enable_systemd=yes
], [
	AC_MSG_RESULT([not enabled])
	enable_systemd=no
])
AM_CONDITIONAL([ENABLE_SYSTEMD], [test "x$enable_systemd" != xno])


# Check for AppArmor
AC_MSG_CHECKING([whether to enable AppArmor profile])
AC_ARG_ENABLE([apparmor], [AC_HELP_STRING([--enable-apparmor],
	[enable AppArmor profile])],
        [enable_apparmor=$enableval],[enable_apparmor=no])
AS_IF([test "x$enable_apparmor" != xno], [
        AC_MSG_RESULT([yes])
	enable_apparmor=yes
], [
        AC_MSG_RESULT([no])
	enable_apparmor=no
])
AM_CONDITIONAL([ENABLE_APPARMOR], [test "x$enable_apparmor" != xno])


# Checks for header files.
AC_CHECK_HEADERS([locale.h stdint.h stdlib.h string.h unistd.h signal.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT16_T

# Checks for library functions.
AC_SEARCH_LIBS([clock_gettime], [rt])
AC_SEARCH_LIBS([floor], [m])
AC_CHECK_FUNCS([setlocale strchr floor pow posix_fallocate mkostemp])

AC_CONFIG_FILES([
	Makefile
	po/Makefile.in
	src/Makefile
	src/gammastep_indicator/Makefile
])
AC_OUTPUT


echo "
 $PACKAGE_NAME $VERSION

    prefix:		${prefix}
    compiler:		${CC}
    cflags:		${CFLAGS}
    ldflags:		${LDFLAGS}

    Adjustment methods:
    DRM:		${enable_drm}
    Wayland:		${enable_wayland}
    RANDR:		${enable_randr}
    VidMode:		${enable_vidmode}

    Location providers:
    Geoclue2:		${enable_geoclue2}

    GUI:		${enable_gui}
    systemd units:	${enable_systemd} ${systemduserunitdir}
    AppArmor profile:   ${enable_apparmor}
"
