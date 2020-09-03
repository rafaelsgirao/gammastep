
SUBDIRS = gammastep_indicator

# I18n
localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS = gammastep

gammastep_SOURCES = \
	colorramp.c colorramp.h \
	config-ini.c config-ini.h \
	gamma-dummy.c gamma-dummy.h \
	hooks.c hooks.h \
	location-manual.c location-manual.h \
	options.c options.h \
	pipeutils.c pipeutils.h \
	redshift.c redshift.h \
	signals.c signals.h \
	solar.c solar.h \
	systemtime.c systemtime.h \
	vlog.c vlog.h

EXTRA_gammastep_SOURCES = \
	gamma-drm.c gamma-drm.h \
	gamma-wl.c gamma-wl.h \
	gamma-randr.c gamma-randr.h \
	gamma-vidmode.c gamma-vidmode.h \
	location-geoclue2.c location-geoclue2.h

AM_CFLAGS =
gammastep_LDADD = @LIBINTL@
EXTRA_DIST = gamma-control.xml orbital-authorizer.xml

if ENABLE_DRM
gammastep_SOURCES += gamma-drm.c gamma-drm.h
AM_CFLAGS += $(DRM_CFLAGS)
gammastep_LDADD += \
	$(DRM_LIBS) $(DRM_CFLAGS)
endif

if ENABLE_WAYLAND
gammastep_SOURCES += gamma-wl.c gamma-wl.h os-compatibility.c os-compatibility.h

AM_CFLAGS += $(WAYLAND_CFLAGS)

gammastep_LDADD += \
	$(WAYLAND_LIBS) $(WAYLAND_CFLAGS)

nodist_gammastep_SOURCES = \
	gamma-control-protocol.c \
	gamma-control-client-protocol.h \
	orbital-authorizer-protocol.c \
	orbital-authorizer-client-protocol.h

BUILT_SOURCES = \
	gamma-control-protocol.c \
	gamma-control-client-protocol.h \
	orbital-authorizer-protocol.c \
	orbital-authorizer-client-protocol.h

EXTRA_DIST += gamma-control.xml orbital-authorizer.xml
endif

if ENABLE_RANDR
gammastep_SOURCES += gamma-randr.c gamma-randr.h
AM_CFLAGS += $(XCB_CFLAGS) $(XCB_RANDR_CFLAGS)
gammastep_LDADD += \
	$(XCB_LIBS) $(XCB_CFLAGS) \
	$(XCB_RANDR_LIBS) $(XCB_RANDR_CFLAGS)
endif

if ENABLE_VIDMODE
gammastep_SOURCES += gamma-vidmode.c gamma-vidmode.h
AM_CFLAGS += $(X11_CFLAGS) $(XF86VM_CFLAGS)
gammastep_LDADD += \
	$(X11_LIBS) $(X11_CFLAGS) \
	$(XF86VM_LIBS) $(XF86VM_CFLAGS)
endif


if ENABLE_GEOCLUE2
gammastep_SOURCES += location-geoclue2.c location-geoclue2.h
AM_CFLAGS += \
	$(GEOCLUE2_CFLAGS)
gammastep_LDADD += \
	$(GEOCLUE2_LIBS) $(GEOCLUE2_CFLAGS)
endif

CLEANFILES = *-protocol.c *-client-protocol.h

%-protocol.c : $(srcdir)/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) private-code < $< > $@

%-client-protocol.h : $(srcdir)/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) client-header < $< > $@
