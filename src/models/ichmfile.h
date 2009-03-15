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

#define CHMSEE_TYPE_ICHMFILE                (chmsee_ichmfile_get_type())
#define CHMSEE_ICHMFILE(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), CHMSEE_TYPE_ICHMFILE, ChmseeIchmfile))
#define CHMSEE_IS_ICHMFILE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHMSEE_TYPE_ICHMFILE))
#define CHMSEE_ICHMFILE_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), CHMSEE_TYPE_ICHMFILE, ChmseeIchmfileInterface))

typedef struct _ChmseeIchmfile ChmseeIchmfile;
typedef struct _ChmseeIchmfileInterface ChmseeIchmfileInterface;

struct _ChmseeIchmfileInterface
{
  GTypeInterface parent_iface;

  void (*do_action) (ChmseeIchmfile* self);
};

GType chmsee_ichmfile_get_type(void);
void chmsee_ichmfile_do_action(ChmseeIchmfile* self);

#endif
