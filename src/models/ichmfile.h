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

#ifndef __ICHMFILE_H__
#define __ICHMFILE_H__

#include <glib-object.h>

#include "models/bookmarks.h"
#include "models/hhc.h"

#define CHMSEE_TYPE_ICHMFILE                (chmsee_ichmfile_get_type())
#define CHMSEE_ICHMFILE(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), CHMSEE_TYPE_ICHMFILE, ChmseeIchmfile))
#define CHMSEE_IS_ICHMFILE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHMSEE_TYPE_ICHMFILE))
#define CHMSEE_ICHMFILE_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), CHMSEE_TYPE_ICHMFILE, ChmseeIchmfileInterface))

typedef struct _ChmseeIchmfile ChmseeIchmfile;
typedef struct _ChmseeIchmfileInterface ChmseeIchmfileInterface;

struct _ChmseeIchmfileInterface
{
  GTypeInterface parent_iface;

  const gchar* (*get_dir) (ChmseeIchmfile* self);
  const gchar* (*get_home) (ChmseeIchmfile* self);
  const gchar* (*get_title) (ChmseeIchmfile* self);
  const gchar* (*get_variable_font) (ChmseeIchmfile* self);
  const gchar* (*get_fixed_font) (ChmseeIchmfile* self);
  Hhc* (*get_link_tree) (ChmseeIchmfile* self);
  Bookmarks* (*get_bookmarks_list) (ChmseeIchmfile* self);

  void (*set_variable_font) (ChmseeIchmfile* self, const gchar* font);
  void (*set_fixed_font) (ChmseeIchmfile* self, const gchar* font);  
};

GType chmsee_ichmfile_get_type(void);
const gchar* chmsee_ichmfile_get_dir(ChmseeIchmfile* self);
const gchar* chmsee_ichmfile_get_home(ChmseeIchmfile* self);
Hhc* chmsee_ichmfile_get_link_tree(ChmseeIchmfile* self);
Bookmarks* chmsee_ichmfile_get_bookmarks_list(ChmseeIchmfile* self);
const gchar* chmsee_ichmfile_get_title(ChmseeIchmfile* self);
const gchar* chmsee_ichmfile_get_variable_font(ChmseeIchmfile* self);
const gchar* chmsee_ichmfile_get_fixed_font(ChmseeIchmfile* self);
void chmsee_ichmfile_set_variable_font(ChmseeIchmfile* self, const gchar* font);
void chmsee_ichmfile_set_fixed_font(ChmseeIchmfile* self, const gchar* font);

#endif
