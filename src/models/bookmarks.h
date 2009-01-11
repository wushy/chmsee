/*
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

#ifndef __CHMSEE_MODELS_BOOKMARKS_H__
#define __CHMSEE_MODELS_BOOKMARKS_H__

#include <glib.h>

typedef GList Bookmarks;

/** 
 * load bookmarks from file.
 * 
 * @param fname file path.
 * 
 * @return Bookmarks* if success
 * @return NULL if failed
 */
Bookmarks* bookmarks_load(const gchar* fname);

/** 
 * save bookmarks to file
 * 
 * @param bookmarks
 * @param ofname output file name
 */
void bookmarks_save(Bookmarks* bookmarks, const gchar* ofname);

/** 
 * free bookmarks
 * 
 * @param bookmarks 
 */
void bookmarks_free(Bookmarks* bookmarks);

#endif
