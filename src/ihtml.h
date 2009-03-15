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
#include <gtk/gtk.h>

#define CHMSEE_TYPE_IHTML (chmsee_ihtml_get_type())
#define CHMSEE_IHTML(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), CHMSEE_TYPE_IHTML, ChmseeIhtml))
#define CHMSEE_IS_IHTML(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHMSEE_TYPE_IHTML))
#define CHMSEE_IHTML_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), CHMSEE_TYPE_IHTML, ChmseeIhtmlInterface))

typedef struct _ChmseeIhtml ChmseeIhtml; /* dummy */
typedef struct _ChmseeIhtmlInterface ChmseeIhtmlInterface;

struct _ChmseeIhtmlInterface
{
  GTypeInterface parent_iface;

  const gchar* (*get_title) (ChmseeIhtml* self);
  const gchar* (*get_location) (ChmseeIhtml* self);
  gboolean (*can_go_back) (ChmseeIhtml* self);
  gboolean (*can_go_forward) (ChmseeIhtml* self);
  

  void (*open_uri) (ChmseeIhtml* self, const gchar* uri);
  void (*copy_selection) (ChmseeIhtml* self);
  void (*select_all) (ChmseeIhtml* self);
  void (*go_back) (ChmseeIhtml* self);
  void (*go_forward) (ChmseeIhtml* self);
  void (*increase_size) (ChmseeIhtml* self);
  void (*decrease_size) (ChmseeIhtml* self);
  void (*reset_size) (ChmseeIhtml* self);
  void (*set_variable_font) (ChmseeIhtml* self, const gchar* font);
  void (*set_fixed_font) (ChmseeIhtml* self, const gchar* font);
  void (*clear) (ChmseeIhtml* self);
  void (*shutdown) (ChmseeIhtml* self);

  GtkWidget* (*get_widget) (ChmseeIhtml* self);
};

GType chmsee_ihtml_get_type(void);

const gchar* chmsee_ihtml_get_title(ChmseeIhtml* self);
const gchar* chmsee_ihtml_get_location(ChmseeIhtml* self);
gboolean chmsee_ihtml_can_go_back(ChmseeIhtml* self);
gboolean chmsee_ihtml_can_go_forward(ChmseeIhtml* self);

void chmsee_ihtml_open_uri(ChmseeIhtml* self, const gchar* uri);
void chmsee_ihtml_copy_selection(ChmseeIhtml* self);
void chmsee_ihtml_select_all(ChmseeIhtml* self);
void chmsee_ihtml_go_back(ChmseeIhtml* self);
void chmsee_ihtml_go_forward(ChmseeIhtml* self);
void chmsee_ihtml_increase_size(ChmseeIhtml* self);
void chmsee_ihtml_reset_size(ChmseeIhtml* self);
void chmsee_ihtml_decrease_size(ChmseeIhtml* self);
void chmsee_ihtml_shutdown(ChmseeIhtml* self);
void chmsee_ihtml_set_variable_font(ChmseeIhtml* self, const gchar* font);
void chmsee_ihtml_set_fixed_font(ChmseeIhtml* self, const gchar* font);
void chmsee_ihtml_clear(ChmseeIhtml* self);
GtkWidget* chmsee_ihtml_get_widget(ChmseeIhtml* self);

#endif
