/*
 *  Copyright (c) 2006           Ji YongGang <jungle@soforge-studio.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"
#include "link.h"

static gint parse_config_line(gchar *, gchar *, gchar *);
static gchar *strip_string(gchar *);
static gchar *escape_parse(gchar *);
static void save_option(FILE *, const gchar *, const gchar *);

#define MAXLINE 1024

typedef struct {
        gchar *id;
        gchar *value;
} Item;

static gint
parse_config_line(gchar *iline, gchar *id, gchar *value)
{
        gchar *p,*p2;
        gchar line[1024];
        gchar tmp[1024];

        strcpy(line, iline); 
        strcpy(id, "");
        p = strtok(line, "=");

        if (p != NULL) {
                strcpy(id, p); /* got id */
                strip_string(id);
        } else 
                return 1;

        strcpy(tmp, "");
        p = strtok(NULL, "");

        if (p != NULL) {
                strcpy(tmp, p); /* string after = */
                strip_string(tmp);
        } else
                return 1;

        /* Now strip quotes from string */
        p = tmp;
        if (*p == '\"')
                p2 = p+1;
        else
                p2 = p;

        if (p[strlen(p)-1] == '\"')
                p[strlen(p)-1] = '\0';

        strcpy(value, p2);

        /* Now reconvert escape-chars */
        escape_parse(value);

        /* All OK */
        return 0;
}

static gchar *
strip_string(gchar *str)
{
        gint i,j;
        gint c1;

        if (str == NULL) 
                return NULL;

        /* count how many leading chars to be whitespace */
        for (i = 0; i < strlen(str); i++) {
                if (str[i] != ' ' && str[i] != '\t' && str[i] != '\r') 
                        break;
        }

        /* count how many trailing chars to be whitespace */
        for (j = strlen(str)-1; j >= 0; j--) {
                if (str[j] != ' ' && str[j] != '\t' && str[j] != '\n') 
                        break;
        }

        /* string contains only whitespace? */
        if (j < i) {
                str[0] = '\0';

                return str;
        }

        /* now move the chars to the front */
        for (c1 = i; c1 <= j; c1++)
                str[c1-i] = str[c1]; 

        str[j+1-i] = '\0';      

        return str;
}

/* Parse escape-chars in string -> e.g. translate \n to newline */
static gchar *
escape_parse(gchar *str)
{
        gchar tmp[MAXLINE];
        gchar c;
        gint i, j;

        if (str == NULL) 
                return NULL;

        j = 0;

        for(i = 0; i < strlen(str); i++) {
                c = str[i];
                if (c == '\\') {
                        i++;
                        switch (str[i]) {

                        case 'n':
                                c = '\n';
                                break;

                        case 't':
                                c = '\t';
                                break;

                        case 'b':
                                c = '\b';
                                break;

                        default:
                                c = str[i];
                        }
                }       

                tmp[j] = c;
                j++;
        }

        tmp[j] = '\0';

        strcpy(str, tmp);

        return(str);
}

static void
save_option(FILE *file, const gchar *id, const gchar *value)
{
        gchar *p;

        p = g_strdup_printf("%s=%s\n", id, value);
        fprintf(file, p);

        g_free(p);
}

static GList *
parse_config_file(const gchar *info, const gchar *file)
{
        FILE *fd;
        GList *pairs = NULL;
        gchar line[MAXLINE];
        gchar id[MAXLINE];
        gchar value[MAXLINE];

        if ((fd = fopen(file, "r")) == NULL) {
                d(g_debug("Failed to open ChmSee %s.\n", info));
                return NULL;
        }
        
        while (fgets(line, MAXLINE, fd)) {
                /* Skip empty or hashed lines */
                strip_string(line);

                if (*line == '#' || *line == '\0') 
                        continue;

                /* Parse lines */
                if (parse_config_line(line, id, value)) {
                        g_print("Syntax error in %s config file\n", info);
                }

                Item *item = g_new(Item, 1);
                item->id = g_strdup(id);
                item->value = g_strdup(value);

                pairs = g_list_prepend(pairs, item);
        }

        fclose(fd);

        return pairs;
}

static void
free_pairs(GList *pairs)
{
        GList *list;

        for (list = pairs; list; list = list->next) {
                Item *item = list->data;

                g_free(item->id);
                g_free(item->value);
        }
        
        g_list_free(pairs);
}

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

        free_pairs(pairs);
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

        free_pairs(pairs);
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

GList *
load_bookmarks(const gchar *path)
{
        GList *links, *pairs, *list;

        links = NULL;

        d(g_debug("bookmarks path = %s", path));

        pairs = parse_config_file("bookmarks", path);

        for (list = pairs; list; list = list->next) {
                Link *link;
                Item *item;

                item = list->data;
                link = link_new(LINK_TYPE_PAGE, item->id, item->value);

                links = g_list_prepend(links, link);
        }

        free_pairs(pairs);

        return links;
}

static void
save_bookmark(Link *link, FILE *fd)
{
        save_option(fd, link->name, link->uri);
}

void
save_bookmarks(const gchar *book_dir, GList *links)
{
        FILE *fd;
        gchar *path;

        path = g_strdup_printf("%s/%s", book_dir, BOOKMARK_FILE);
        
        d(g_debug("save bookmarks path = %s", path));

        fd = fopen(path, "w");

        if (!fd) {
                g_print("Faild to open bookmarks file: %s", path);
                return;
        }

        g_list_foreach(links, (GFunc)save_bookmark, fd);

        fclose(fd);
}

gchar *
convert_filename_to_utf8(const gchar *filename, const gchar *codeset)
{
        gchar * filename_utf8;

        if (g_utf8_validate(filename, -1, NULL)) {
                filename_utf8 = g_strdup(filename);
        } else {
                d(g_message("Convert filename to UTF8."));
                filename_utf8 = g_filename_to_utf8(filename, -1, NULL, NULL, NULL);

                if (filename_utf8 == NULL)
                        filename_utf8 = g_convert(filename, -1, "UTF-8",
                                                  codeset, 
                                                  NULL, NULL, NULL);
        }

        return filename_utf8;
}

gchar *
convert_string_to_utf8(const gchar *string, const gchar *codeset)
{
        gchar * string_utf8;

        if (g_utf8_validate(string, -1, NULL)) {
                d(g_message("string is utf8"));
                string_utf8 = g_strdup(string);
        } else {
                d(g_message("string is not utf8"));
                string_utf8 = g_convert(string, -1, "UTF-8", 
                                        codeset, 
                                        NULL, NULL, NULL);
        }

        return string_utf8;
}

gint
ncase_compare_utf8_string(const gchar *str1, const gchar *str2)
{
        gint result;
        gchar *ncase_str1, *ncase_str2;
        gchar *normalized_str1, *normalized_str2;

        ncase_str1 = g_utf8_casefold(str1, -1);
        ncase_str2 = g_utf8_casefold(str2, -1);

        normalized_str1 = g_utf8_normalize(ncase_str1, -1, G_NORMALIZE_DEFAULT);
        normalized_str2 = g_utf8_normalize(ncase_str2, -1, G_NORMALIZE_DEFAULT);

        result = g_utf8_collate(normalized_str1, normalized_str2);

        g_free(ncase_str1);
        g_free(ncase_str2);
        g_free(normalized_str1);
        g_free(normalized_str2);

        return result;
}

gchar *
file_exist_ncase(const gchar *path)
{
        gchar *ch;
        gchar *dirname;
        gchar *filename;
        gchar *found;

        GDir *dir;

        ch = g_strrstr(path, "/");
        dirname = g_strndup(path, ch - path);
        filename = g_strdup(ch + 1);

        d(g_debug("dirname = %s", dirname));
        d(g_debug("filename = %s", filename));

        dir = g_dir_open(dirname, 0, NULL);
        
        if (dir) {
                const gchar *entry;

                while ((entry = g_dir_read_name(dir))) {
                        d(g_debug("entry = %s", entry));
                        if (!g_ascii_strcasecmp(filename, entry)) {
                                d(g_debug("found case insensitive file: %s", entry));
                                found = g_strdup_printf("%s/%s", dirname, entry);
                                g_dir_close(dir);

                                return found;
                        }
                }
        }

        return NULL;
}

char *
url_decode(const char *encoded)
{
        const char *at = encoded;
        int length = 0;
        char *rv;
        char *out;

        while (*at != '\0') {
                if (*at == '%') {
                        if (at[1] == '\0' || at[2] == '\0') {
                                g_warning ("malformed URL encoded string");
                                return NULL;
                        }
                        at += 3;
                        length++;
                } else {
                        at++;
                        length++;
                }
        }

        rv = g_new (char, length + 1);
        out = rv;
        at = encoded;

        while (*at != '\0') {
                if (*at == '%') {
                        char hex[3];
                        hex[0] = at[1];
                        hex[1] = at[2];
                        hex[2] = '\0';
                        if (at[1] == '\0' || at[2] == '\0')
                                return NULL;
                        at += 3;
                        *out++ = (char) strtol (hex, NULL, 16);
                } else {
                        *out++ = *at++;
                        length++;
                }
        }

        *out = '\0';
        return rv;
}

void
command_delete_tmpdir(char *s_path)
{
        char *argv[4];
                
        g_return_if_fail(g_file_test(s_path, G_FILE_TEST_EXISTS));

        argv[0] = "rm";
        argv[1] = "-rf";
        argv[2] = s_path;
        argv[3] = NULL;

        g_spawn_async(g_get_tmp_dir(), argv, NULL, 
                      G_SPAWN_SEARCH_PATH, 
                      NULL, NULL, NULL,
                      NULL);
}

gchar *
get_real_uri(const gchar *uri)
{
        gchar *real_uri;
        gchar *p;

        p = g_strrstr(uri, "#");

        if (p)
                real_uri = g_strndup(uri, p - uri);
        else 
                real_uri = g_strdup(uri);

        return real_uri;
}
