
if ENABLE_GUI
gammastep_indicator_PYTHON = \
	__init__.py \
	controller.py \
	statusicon.py \
	utils.py
nodist_gammastep_indicator_PYTHON = \
	defs.py
gammastep_indicatordir = $(pythondir)/gammastep_indicator

bin_SCRIPTS = gammastep-indicator
endif

EXTRA_DIST = defs.py.in gammastep-indicator.in
CLEANFILES = defs.py gammastep-indicator


# Local python definitions
defs.py: defs.py.in
	$(AM_V_GEN)sed -e "s|\@bindir\@|$(bindir)|g" \
		-e "s|\@localedir\@|$(localedir)|g" $< > $@

gammastep-indicator: gammastep-indicator.in
	$(AM_V_GEN)sed -e "s|\@pythondir\@|$(pythondir)|g" $< > $@
