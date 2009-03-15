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

#ifndef __CHMSEE_IHTML_H__
#define __CHMSEE_IHTML_H__

#include <glib-object.h>

#define CHMSEE_TYPE_IHTML (chmsee_ihtml_get_type())
#define CHMSEE_IHTML(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), CHMSEE_TYPE_IHTML, ChmseeIhtml))
#define CHMSEE_IS_ITHML(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHMSEE_TYPE_IHTML))
#define CHMSEE_ITHML_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), CHMSEE_TYPE_IHTML, ChmseeIhtmlInterface))

typedef struct _ChmseeIhtml ChmseeIhtml; /* dummy */
typedef struct _ChmseeIhtmlInterface ChmseeIhtmlInterface;

struct _ChmseeIhtmlInterface
{
  GTypeInterface parent_iface;
};

GType chmsee_ihtml_get_type(void);

#endif
