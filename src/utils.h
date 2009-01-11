/*
 *  Copyright (C) 2006 Ji YongGang <jungle@soforge-studio.com>
 *  Copyright (C) 2009 LI Daobing <lidaobing@gmail.com>
 *
 *  ChmSee is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.

 *  ChmSee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with ChmSee; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <glib.h>

#include "chmsee.h"
#include "chmfile.h"

#ifdef _DEBUG
#define d(x) x
#else
#define d(x)
#endif

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

gchar *convert_filename_to_utf8(const gchar *, const gchar *);
gchar *convert_string_to_utf8(const gchar *, const gchar *);
gint ncase_compare_utf8_string(const gchar *, const gchar *);
gchar *file_exist_ncase(const gchar *);
char *url_decode(const char*);
void command_delete_tmpdir(char *);
gchar *get_real_uri(const gchar *);

void load_chmsee_config(ChmSee *);
void save_chmsee_config(ChmSee *);
void load_fileinfo(ChmFile *);
void save_fileinfo(ChmFile *);
GList *load_bookmarks(const gchar *);
void save_bookmarks(const gchar *, GList *);

/** 
 * return the correct filename
 * 
 * @param fname the original filename
 * 
 * @return NULL if failed.
 *
 * @return char* if new file name, The string returned is new and it's
 * the caller's responsibility to free the string.
 */
char* correct_filename(const gchar* fname);

#endif /* !__UTILS_H__ */
