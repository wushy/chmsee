AM_CPPFLAGS = -DCHMSEE_NO_LINK=\"chmsee_no_link\" -I$(srcdir)/.. @CHMSEE_MODELS_CFLAGS@
AM_CFLAGS = -Wall -Werror 

noinst_LIBRARIES = libchmsee-models.a
libchmsee_models_a_SOURCES = link.c bookmarks.c hhc.c

EXTRA_DIST = link.h bookmarks.h hhc.h
