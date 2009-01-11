SUBDIRS = utils models

CHMSEE_DEFINES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DCHMSEE_DATA_DIR=\""$(datadir)/chmsee"\" \
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\" \
	-DGLADE_FILE=\"chmsee.glade\" \
	-DBOOKMARK_FILE=\"chmsee_bookmarks\" \
	-DBOOKINFO_FILE=\"chmsee_bookinfo\" \
	-DCHMSEE_NO_LINK=\"chmsee_no_link\"

bin_PROGRAMS = chmsee

chmsee_SOURCES = \
	main.c \
	marshal_main.c \
	ui_bookmarks.c ui_bookmarks.h \
	booktree.c booktree.h \
	chmfile.c chmfile.h \
	chmsee.c chmsee.h \
	html.c html.h \
	parser.c parser.h \
	setup.c	setup.h	\
	startup.c startup.h \
	gecko_utils.cpp gecko_utils.h \
	utils.c utils.h

chmsee_CFLAGS = \
	$(CHMSEE_DEFINES) \
	@GLADE_CFLAGS@ \
	@CHMLIB_CFLAGS@ \
	@GECKO_CFLAGS@	\
	$(AM_CFLAGS)

chmsee_CPPFLAGS = \
	$(CHMSEE_DEFINES) \
	@GLADE_CFLAGS@ \
	@CHMLIB_CFLAGS@ \
	@GECKO_CFLAGS@	\
	$(AM_CFLAGS)

chmsee_LDADD = \
	utils/libchmsee-utils.a \
	models/libchmsee-models.a \
	@GLADE_LIBS@ \
	@CHMLIB_LIBS@ \
	@MD5_LIBS@ \
	@GECKO_LIBS@ \
        $(AM_LDFLAGS)

marshal.h: marshal.list
	(cd $(srcdir) && \
	@GLIB_GENMARSHAL@ --prefix=marshal marshal.list --header) > xgen-gmh \
	&& (cmp -s xgen-gmh marshal.h || cp xgen-gmh marshal.h) \
	&& rm -f xgen-gmh xgen-gmh~

marshal.c: marshal.list
	(cd $(srcdir) && \
	@GLIB_GENMARSHAL@ --prefix=marshal marshal.list --body) > xgen-gmc \
	&& cp xgen-gmc marshal.c \
	&& rm -f xgen-gmc xgen-gmc~

marshal_main.c: marshal.c marshal.h

default_prefs_in_FILES = \
	default-prefs-common.js \
	default-prefs-mozilla.js \
	default-prefs-firefox.js \
	default-prefs-seamonkey.js \
	default-prefs-xulrunner.js \
	default-prefs-libxul.js

default-prefs.js: $(default_prefs_files)
	cat $(srcdir)/default-prefs-common.js $(srcdir)/default-prefs-@GECKO_FLAVOUR@.js > $@

default_prefsdir = $(pkgdatadir)
default_prefs_DATA = default-prefs.js

BUILT_SOURCES =						\
	marshal.h					\
	marshal.c

EXTRA_DIST =						\
	marshal.list					\
	$(default_prefs_in_FILES)

CLEANFILES = $(BUILT_SOURCES)				\
	 $(default_prefs_DATA)
