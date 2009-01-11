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

static gint parse_config_line(gchar *, gchar *, gchar *);
static gchar *strip_string(gchar *);
static gchar *escape_parse(gchar *);

#define MAXLINE 1024



void
load_chmsee_config(ChmSee *chmsee)
{
        GList *pairs, *list;
        gchar *path;

        path = g_build_filename(chmsee->home, "config", NULL);

        d(g_debug("config path = %s", path));

        pairs = parse_config_file("config", path);

        for (list = pairs; list; list = list->next) {
                Item *item;

                item = list->data;
                
                /* Get user prefered language */
                if (strstr(item->id, "LANG")) {
                        chmsee->lang = atoi(item->value);
                        continue;
                }

                /* Get last directory */
                if (strstr(item->id, "LAST_DIR")) {
                        chmsee->last_dir = g_strdup(item->value);
                        continue;
                }

                /* Get window position */
                if (strstr(item->id, "POS_X")) {
                        chmsee->pos_x = atoi(item->value);
                        continue;
                }
                if (strstr(item->id, "POS_Y")) {
                        chmsee->pos_y = atoi(item->value);
                        continue;
                }
                if (strstr(item->id, "WIDTH")) {
                        chmsee->width = atoi(item->value);
                        continue;
                }
                if (strstr(item->id, "HEIGHT")) {
                        chmsee->height = atoi(item->value);
                        continue;
                }
        }

        free_config_list(pairs);
        g_free(path);
}

void 
save_chmsee_config(ChmSee *chmsee)
{
        FILE *file;
        gchar *path;

        path = g_build_filename(chmsee->home, "config", NULL);

        file = fopen(path, "w");

        if (!file) {
                g_print("Faild to open chmsee config: %s", path);
                return;
        }

        save_option(file, "LANG", g_strdup_printf("%d", chmsee->lang));
        save_option(file, "LAST_DIR", chmsee->last_dir);
        save_option(file, "POS_X", g_strdup_printf("%d", chmsee->pos_x));
        save_option(file, "POS_Y", g_strdup_printf("%d", chmsee->pos_y));
        save_option(file, "WIDTH", g_strdup_printf("%d", chmsee->width));
        save_option(file, "HEIGHT", g_strdup_printf("%d", chmsee->height));

        fclose(file);
        g_free(path);
}

void
load_fileinfo(ChmFile *book)
{
        GList *pairs, *list;
        gchar *path;

        path = g_strdup_printf("%s/%s", book->dir, BOOKINFO_FILE);

        d(g_debug("bookinfo path = %s", path));

        pairs = parse_config_file("bookinfo", path);

        for (list = pairs; list; list = list->next) {
                Item *item;

                item = list->data;
                
                if (strstr(item->id, "hhc")) {
                        book->hhc = g_strdup(item->value);
                        continue;
                }

                if (strstr(item->id, "hhk")) {
                        book->hhk = g_strdup(item->value);
                        continue;
                }

                if (strstr(item->id, "home")) {
                        book->home = g_strdup(item->value);
                        continue;
                }

                if (strstr(item->id, "title")) {
                        book->title = g_strdup(item->value);
                        continue;
                }

                if (strstr(item->id, "encoding")) {
                        book->encoding = g_strdup(item->value);
                        continue;
                }

                if (strstr(item->id, "variable_font")) {
                        book->variable_font = g_strdup(item->value);
                        continue;
                }

                if (strstr(item->id, "fixed_font")) {
                        book->fixed_font = g_strdup(item->value);
                        continue;
                }
        }

        free_config_list(pairs);
}

void
save_fileinfo(ChmFile *book)
{
        FILE *fd;
        gchar *path;

        path = g_strdup_printf("%s/%s", book->dir, BOOKINFO_FILE);
        
        d(g_debug("save bookinfo path = %s", path));

        fd = fopen(path, "w");

        if (!fd) {
                g_print("Faild to open bookinfo file: %s", path);
                return;
        }

        save_option(fd, "hhc", book->hhc);
        save_option(fd, "hhk", book->hhk);
        save_option(fd, "home", book->home);
        save_option(fd, "title", book->title);
        save_option(fd, "encoding", book->encoding);
        save_option(fd, "variable_font", book->variable_font);
        save_option(fd, "fixed_font", book->fixed_font);

        fclose(fd);
}


