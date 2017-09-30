/*
 *  Copyright (c) 2006		 Ji YongGang <jungle@soforge-studio.com>
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

/***************************************************************************
 *   Copyright (C) 2003 by zhong					   *
 *   zhongz@163.com							   *
 *									   *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	   *
 *   (at your option) any later version.				   *
 ***************************************************************************/

#include "config.h"
#include "setup.h"

#include <glade/glade.h>

#include "utils/utils.h"
#include "gecko_utils.h"

static void on_cache_clear(GtkWidget *, ChmSee *);
static void on_window_close(GtkButton *, ChmSee *);

typedef struct
{
	const gchar    *cchar_number;
	const gchar    *cchar_codeset;
} ChmseeChar;

static void
on_cache_clear(GtkWidget *widget, ChmSee *chmsee)
{
	command_delete_tmpdir(
			chmsee_get_cache_dir(chmsee));
}

static void
variable_font_set_cb(GtkFontButton *button, ChmSee *chmsee)
{
	gchar *font_name;

	font_name = g_strdup(gtk_font_button_get_font_name(button));

	g_debug("variable font set: %s", font_name);

//	gecko_utils_set_font(GECKO_PREF_FONT_VARIABLE, font_name);

	chmsee_set_variable_font(chmsee, font_name);
}

static void
fixed_font_set_cb(GtkFontButton *button, ChmSee *chmsee)
{
	gchar *font_name;

	font_name = g_strdup(gtk_font_button_get_font_name(button));

	g_debug("fixed font set: %s", font_name);

//	gecko_utils_set_font(GECKO_PREF_FONT_FIXED, font_name);

	chmsee_set_fixed_font(chmsee, font_name);
}

static void
cmb_lang_changed_cb(GtkWidget *widget, ChmSee *chmsee)
{
	GtkComboBox *combobox;
	gint index;

	combobox = GTK_COMBO_BOX (widget);
	index = gtk_combo_box_get_active(combobox);

	if (index >= 0) {
		g_debug("select lang: %d", index);
//		gecko_utils_set_default_lang(index);
		chmsee_set_lang(chmsee, index);
	}
}

static void
on_window_close(GtkButton *button, ChmSee *chmsee)
{
	gtk_widget_destroy(gtk_widget_get_toplevel (GTK_WIDGET(button)));
}

void
setup_window_new(ChmSee *chmsee)
{
	GladeXML *glade;

	GtkWidget *setup_window;
	GtkWidget *cache_entry;
	GtkWidget *clear_button;
	GtkWidget *variable_font_button;
	GtkWidget *fixed_font_button;
	GtkWidget *cmb_lang;
	GtkWidget *close_button;

	/* create setup window */
	glade = glade_xml_new(get_resource_path(GLADE_FILE), "setup_window", NULL);

	setup_window = glade_xml_get_widget(glade, "setup_window");
	g_signal_connect_swapped((gpointer) setup_window,
				 "destroy",
				 G_CALLBACK (gtk_widget_destroy),
				 GTK_OBJECT (setup_window));

	/* cache directory */
	cache_entry = glade_xml_get_widget(glade, "cache_dir_entry");
	gtk_entry_set_text(GTK_ENTRY(cache_entry), chmsee_get_cache_dir(chmsee));

	clear_button = glade_xml_get_widget(glade, "setup_clear");
	g_signal_connect(G_OBJECT (clear_button),
			 "clicked",
			 G_CALLBACK (on_cache_clear),
			 chmsee);

	/* font setting */
	variable_font_button = glade_xml_get_widget(glade, "variable_fontbtn");
	g_signal_connect(G_OBJECT (variable_font_button),
			 "font-set",
			 G_CALLBACK (variable_font_set_cb),
			 chmsee);

	fixed_font_button = glade_xml_get_widget(glade, "fixed_fontbtn");
	g_signal_connect(G_OBJECT (fixed_font_button),
			 "font-set",
			 G_CALLBACK (fixed_font_set_cb),
			 chmsee);

	/* default lang */
	cmb_lang = glade_xml_get_widget(glade, "cmb_default_lang");
	g_signal_connect(G_OBJECT (cmb_lang),
			 "changed",
			 G_CALLBACK (cmb_lang_changed_cb),
			 chmsee);
	gtk_combo_box_set_active(GTK_COMBO_BOX (cmb_lang), chmsee_get_lang(chmsee));

	close_button = glade_xml_get_widget(glade, "setup_close");
	g_signal_connect(G_OBJECT (close_button),
			 "clicked",
			 G_CALLBACK (on_window_close),
			 chmsee);

	if (chmsee_has_book(chmsee)) {
		gtk_font_button_set_font_name(GTK_FONT_BUTTON (variable_font_button),
                                              chmsee_get_variable_font(chmsee));
		gtk_font_button_set_font_name(GTK_FONT_BUTTON (fixed_font_button),
                                              chmsee_get_fixed_font(chmsee));
		gtk_widget_set_sensitive(variable_font_button, TRUE);
		gtk_widget_set_sensitive(fixed_font_button, TRUE);
	}

	g_object_unref(glade);
}
