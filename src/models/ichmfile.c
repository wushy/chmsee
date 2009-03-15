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

#include "config.h"
#include "ichmfile.h"

static void
chmsee_ichmfile_base_init (gpointer g_class)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      /* add properties and signals to the interface here */

      is_initialized = TRUE;
    }
}

GType chmsee_ichmfile_get_type(void)
{
  static GType iface_type = 0;
  if (iface_type == 0)
    {
      static const GTypeInfo info = {
        sizeof (ChmseeIchmfileInterface),
        chmsee_ichmfile_base_init,
        NULL,   /* base_finalize */
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE, "ChmseeIchmfile",
                                           &info, 0);
    }

  return iface_type;
}

const gchar* chmsee_ichmfile_get_dir(ChmseeIchmfile* self)
{
  g_return_val_if_fail(CHMSEE_IS_ICHMFILE(self), NULL);
  return CHMSEE_ICHMFILE_GET_INTERFACE(self)->get_dir(self);
}

const gchar* chmsee_ichmfile_get_home(ChmseeIchmfile* self)
{
  g_return_val_if_fail(CHMSEE_IS_ICHMFILE(self), NULL);
  return CHMSEE_ICHMFILE_GET_INTERFACE(self)->get_home(self);
}

const gchar* chmsee_ichmfile_get_title(ChmseeIchmfile* self)
{
  g_return_val_if_fail(CHMSEE_IS_ICHMFILE(self), NULL);
  return CHMSEE_ICHMFILE_GET_INTERFACE(self)->get_title(self);
}

const gchar* chmsee_ichmfile_get_variable_font(ChmseeIchmfile* self)
{
  g_return_val_if_fail(CHMSEE_IS_ICHMFILE(self), NULL);
  return CHMSEE_ICHMFILE_GET_INTERFACE(self)->get_variable_font(self);
}

const gchar* chmsee_ichmfile_get_fixed_font(ChmseeIchmfile* self)
{
  g_return_val_if_fail(CHMSEE_IS_ICHMFILE(self), NULL);
  return CHMSEE_ICHMFILE_GET_INTERFACE(self)->get_fixed_font(self);
}

Hhc* chmsee_ichmfile_get_link_tree(ChmseeIchmfile* self)
{
  g_return_val_if_fail(CHMSEE_IS_ICHMFILE(self), NULL);
  return CHMSEE_ICHMFILE_GET_INTERFACE(self)->get_link_tree(self);
}

Bookmarks* chmsee_ichmfile_get_bookmarks_list(ChmseeIchmfile* self) {
  g_return_val_if_fail(CHMSEE_IS_ICHMFILE(self), NULL);
  return CHMSEE_ICHMFILE_GET_INTERFACE(self)->get_bookmarks_list(self);
}

void chmsee_ichmfile_set_variable_font(ChmseeIchmfile* self, const gchar* font)
{
  g_return_if_fail(CHMSEE_IS_ICHMFILE(self));
  return CHMSEE_ICHMFILE_GET_INTERFACE(self)->set_variable_font(self, font);
}

void chmsee_ichmfile_set_fixed_font(ChmseeIchmfile* self, const gchar* font)
{
  g_return_if_fail(CHMSEE_IS_ICHMFILE(self));
  return CHMSEE_ICHMFILE_GET_INTERFACE(self)->set_fixed_font(self, font);
}


