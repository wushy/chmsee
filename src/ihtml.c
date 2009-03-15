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
#include "ihtml.h"

static void
chmsee_ihtml_base_init (gpointer g_class)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      /* add properties and signals to the interface here */

      is_initialized = TRUE;
    }
}

GType
chmsee_ihtml_get_type (void)
{
  static GType iface_type = 0;
  if (iface_type == 0)
    {
      static const GTypeInfo info = {
        sizeof (ChmseeIhtmlInterface),
        chmsee_ihtml_base_init,   /* base_init */
        NULL,   /* base_finalize */
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE, "ChmseeIhtml",
                                           &info, 0);
    }

  return iface_type;
}

const gchar* chmsee_ihtml_get_title(ChmseeIhtml* self) {
  g_return_val_if_fail (CHMSEE_IS_IHTML(self), NULL);
  return CHMSEE_IHTML_GET_INTERFACE (self)->get_title(self);  
}

const gchar* chmsee_ihtml_get_location(ChmseeIhtml* self) {
  g_return_val_if_fail (CHMSEE_IS_IHTML(self), NULL);
  return CHMSEE_IHTML_GET_INTERFACE (self)->get_location(self);  
}

gboolean chmsee_ihtml_can_go_back(ChmseeIhtml* self) {
  g_return_val_if_fail (CHMSEE_IS_IHTML(self), FALSE);
  return CHMSEE_IHTML_GET_INTERFACE (self)->can_go_back(self);    
}
gboolean chmsee_ihtml_can_go_forward(ChmseeIhtml* self) {
  g_return_val_if_fail (CHMSEE_IS_IHTML(self), FALSE);
  return CHMSEE_IHTML_GET_INTERFACE (self)->can_go_forward(self);
}


void chmsee_ihtml_open_uri(ChmseeIhtml* self, const gchar* uri)
{
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->open_uri(self, uri);
}

void chmsee_ihtml_copy_selection(ChmseeIhtml* self)
{
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->copy_selection(self);
}
void chmsee_ihtml_select_all(ChmseeIhtml* self)
{
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->select_all(self);
}
void chmsee_ihtml_go_back(ChmseeIhtml* self)
{
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->go_back(self);
}

void chmsee_ihtml_go_forward(ChmseeIhtml* self) {
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->go_forward(self);
}
void chmsee_ihtml_increase_size(ChmseeIhtml* self)
{
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->increase_size(self);
}

void chmsee_ihtml_reset_size(ChmseeIhtml* self) 
  {
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->reset_size(self);
}

void chmsee_ihtml_decrease_size(ChmseeIhtml* self)
{
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->decrease_size(self);
}

void chmsee_ihtml_shutdown(ChmseeIhtml* self) 
{
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->shutdown(self);
}

void chmsee_ihtml_set_variable_font(ChmseeIhtml* self, const gchar* font) {
  
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->set_variable_font(self, font);
}

void chmsee_ihtml_set_fixed_font(ChmseeIhtml* self, const gchar* font) {
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->set_fixed_font(self, font);
  
}
void chmsee_ihtml_clear(ChmseeIhtml* self)
  {
  g_return_if_fail (CHMSEE_IS_IHTML(self));
  CHMSEE_IHTML_GET_INTERFACE (self)->clear(self);
}

GtkWidget* chmsee_ihtml_get_widget(ChmseeIhtml* self) {
  g_return_val_if_fail (CHMSEE_IS_IHTML(self), NULL);
  return CHMSEE_IHTML_GET_INTERFACE (self)->get_widget(self);
}



