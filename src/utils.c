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

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>             /* R_OK */

#include "models/link.h"

void
load_chmsee_config(ChmSee *self)
{
	GList *pairs, *list;
	gchar *path;

	path = g_build_filename(self->home, "config", NULL);

	g_debug("config path = %s", path);

	pairs = parse_config_file("config", path);

	for (list = pairs; list; list = list->next) {
		Item *item;

		item = list->data;

		/* Get user prefered language */
		if (strstr(item->id, "LANG")) {
			self->lang = atoi(item->value);
			continue;
		}

		/* Get last directory */
		if (strstr(item->id, "LAST_DIR")) {
			self->last_dir = g_strdup(item->value);
			continue;
		}

		/* Get window position */
		if (strstr(item->id, "POS_X")) {
			self->pos_x = atoi(item->value);
			continue;
		}
		if (strstr(item->id, "POS_Y")) {
			self->pos_y = atoi(item->value);
			continue;
		}
		if (strstr(item->id, "WIDTH")) {
			self->width = atoi(item->value);
			continue;
		}
		if (strstr(item->id, "HEIGHT")) {
			self->height = atoi(item->value);
			continue;
		}
		if(strstr(item->id, "HPANED_POSTION")) {
			chmsee_set_hpaned_position(self, atoi(item->value));
			continue;
		}
	}

	free_config_list(pairs);
	g_free(path);
}

void
save_chmsee_config(ChmSee *self)
{
        FILE *file;
        gchar *path;

        path = g_build_filename(self->home, "config", NULL);

        file = fopen(path, "w");

        if (!file) {
                g_print("Faild to open chmsee config: %s", path);
                return;
        }

        save_option(file, "LANG", g_strdup_printf("%d", self->lang));
        save_option(file, "LAST_DIR", self->last_dir);
        save_option(file, "POS_X", g_strdup_printf("%d", self->pos_x));
        save_option(file, "POS_Y", g_strdup_printf("%d", self->pos_y));
        save_option(file, "WIDTH", g_strdup_printf("%d", self->width));
        save_option(file, "HEIGHT", g_strdup_printf("%d", self->height));
        save_option(file, "HPANED_POSTION", g_strdup_printf("%d", chmsee_get_hpaned_position(self)));

        fclose(file);
        g_free(path);
}

