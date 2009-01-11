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
#include "utils/utils.h"

void load_chmsee_config(ChmSee *);
void save_chmsee_config(ChmSee *);
void load_fileinfo(ChmFile *);
void save_fileinfo(ChmFile *);
GList *load_bookmarks(const gchar *);
void save_bookmarks(const gchar *, GList *);


#endif /* !__UTILS_H__ */
