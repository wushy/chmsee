## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = cs2w doc po src

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
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS\
	TODO \
	README.zh \
	NEWS.zh \
	$(appicon_DATA)	\
	$(desktop_in_FILES) \
	$(desktop_FILES) \
	$(mime_DATA) \
	$(chmsee_DATA) \
	po/chmsee.pot \
	autogen.sh \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in
