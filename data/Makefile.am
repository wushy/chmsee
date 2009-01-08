@INTLTOOL_DESKTOP_RULE@

desktop_in_FILES = chmsee.desktop.in
desktop_FILES =$(desktop_in_FILES:.desktop.in=.desktop)
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_FILES)
appicondir = $(datadir)/pixmaps
appicon_DATA = chmsee-icon.png

mimedir = $(datadir)/mime-info
mime_DATA = chmsee.keys chmsee.mime

chmseedir = $(datadir)/chmsee
chmsee_DATA = \
	chmsee-icon.png \
	chmsee-splash.png \
	book-open.png \
	book-closed.png \
	helpdoc.png \
	show-pane.png \
	hide-pane.png \
	chmsee.glade

EXTRA_DIST = \
	$(appicon_DATA)	\
	$(desktop_in_FILES) \
	$(desktop_FILES) \
	$(mime_DATA) \
	$(chmsee_DATA)
