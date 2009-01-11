AM_CPPFLAGS = \
	-DCHMSEE_NO_LINK=\"chmsee_no_link\" \
	-DCHMSEE_BOOKMARK_FILE=\"chmsee_bookmarks\" \
	-DCHMSEE_BOOKINFO_FILE=\"chmsee_bookinfo\" \
	-I$(srcdir)/.. @CHMSEE_MODELS_CFLAGS@
AM_CFLAGS = -Wall -Werror 

noinst_LIBRARIES = libchmsee-models.a
libchmsee_models_a_SOURCES = link.c bookmarks.c hhc.c chmfile.c

EXTRA_DIST = link.h bookmarks.h hhc.h chmfile.h
