
SUBDIRS = src po
ACLOCAL_AMFLAGS = -I m4

# Install systemd user unit files locally for distcheck
DISTCHECK_CONFIGURE_FLAGS = \
	--with-systemduserunitdir=$$dc_install_base/$(systemduserunitdir)

UPDATE_ICON_CACHE = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor || :

EXTRA_ROOTDOC_FILES = \
	CONTRIBUTING.md \
	DESIGN \
	README \
	README-colorramp \
	gammastep.conf.sample

_HICOLOR_FILES = \
	data/icons/hicolor/scalable/apps/gammastep.svg \
	data/icons/hicolor/scalable/apps/gammastep-status-on.svg \
	data/icons/hicolor/scalable/apps/gammastep-status-off.svg

DESKTOP_IN_FILES = \
	data/applications/gammastep.desktop.in \
	data/applications/gammastep-indicator.desktop.in

SYSTEMD_USER_UNIT_IN_FILES = \
	data/systemd/gammastep.service.in \
	data/systemd/gammastep-indicator.service.in

APPDATA_IN_FILES = \
	data/appdata/gammastep-indicator.appdata.xml.in

APPARMOR_IN_FILES = \
	data/apparmor/com.gitlab.chinstrap.gammastep.in


# Icons
if ENABLE_GUI
hicolor_icondir = @datadir@/icons/hicolor/scalable/apps
hicolor_icon_DATA = $(_HICOLOR_FILES)
endif


# Desktop file
if ENABLE_GUI
desktopdir = @datadir@/applications
desktop_DATA = $(DESKTOP_IN_FILES:.desktop.in=.desktop)
endif

if ENABLE_GUI
install-data-hook:
	$(UPDATE_ICON_CACHE);

uninstall-hook:
	$(UPDATE_ICON_CACHE);

# We would preferable use @INTLTOOL_DESKTOP_RULE@ here but
# sadly it is broken for out-of-tree builds.
%.desktop: %.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po)
	$(AM_V_GEN)$(MKDIR_P) $(@D);
	$(INTLTOOL_V_MERGE)LC_ALL=C $(INTLTOOL_MERGE) $(INTLTOOL_V_MERGE_OPTIONS) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@
	rm -f -- $(top_builddir)/po/.intltool-merge-cache.lock
endif


# man page
dist_man1_MANS = gammastep.1


# Systemd service files
if ENABLE_SYSTEMD
systemduserunit_DATA = $(SYSTEMD_USER_UNIT_IN_FILES:.service.in=.service)
endif

$(systemduserunit_DATA): $(SYSTEMD_USER_UNIT_IN_FILES) Makefile
	$(AM_V_GEN)$(MKDIR_P) $(@D) && \
		sed -e "s|\@bindir\@|$(bindir)|g" "$(srcdir)/$(@:.service=.service.in)" > $@


# Appdata file
if ENABLE_GUI
appdatadir = @datadir@/appdata
appdata_DATA = $(APPDATA_IN_FILES:.xml.in=.xml)

# We would preferable use @INTLTOOL_XML_RULE@ here but
# sadly it is broken for out-of-tree builds.
%.xml: %.xml.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po)
	$(AM_V_GEN)$(MKDIR_P) $(@D);
	$(INTLTOOL_V_MERGE)LC_ALL=C $(INTLTOOL_MERGE) $(INTLTOOL_V_MERGE_OPTIONS) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@
	rm -f -- $(top_builddir)/po/.intltool-merge-cache.lock
endif


# AppArmor profile
if ENABLE_APPARMOR
apparmordir = @sysconfdir@/apparmor.d
apparmor_DATA = $(APPARMOR_IN_FILES:.in=)

$(apparmor_DATA): $(APPARMOR_IN_FILES) Makefile
	$(AM_V_GEN)$(MKDIR_P) $(@D) && \
		sed -e "s|\@bindir\@|$(bindir)|g" "$(srcdir)/$(@:=.in)" > $@
endif



EXTRA_DIST = \
	$(EXTRA_ROOTDOC_FILES) \
	$(_HICOLOR_FILES) \
	$(DESKTOP_IN_FILES) \
	$(SYSTEMD_USER_UNIT_IN_FILES) \
	$(APPDATA_IN_FILES) \
	$(APPARMOR_IN_FILES)

CLEANFILES = \
	$(desktop_DATA) \
	$(systemduserunit_DATA) \
	$(appdata_DATA) \
	$(apparmor_DATA)


# Update PO translations
.PHONY: update-po
update-po:
	cd po && $(MAKE) POTFILES gammastep.pot update-po
