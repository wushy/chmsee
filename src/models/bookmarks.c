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
#include "bookmarks.h"

#include <stdio.h>

#include "utils/utils.h"
#include "models/link.h"

Bookmarks *
bookmarks_load(const gchar *path)
{
  Bookmarks* links = NULL;
  GList *pairs, *list;

  d(g_debug("bookmarks path = %s", path));

  pairs = parse_config_file("bookmarks", path);

  for (list = pairs; list; list = list->next) {
    Link *link;
    Item *item;

    item = list->data;
    link = link_new(LINK_TYPE_PAGE, item->id, item->value);

    links = g_list_prepend(links, link);
  }

  free_config_list(pairs);

  return links;
}

static void
save_bookmark(Link *link, FILE *fd)
{
  save_option(fd, link->name, link->uri);
}

void
bookmarks_save(Bookmarks* links, const gchar* path)
{
  FILE *fd;
        
  d(g_debug("save bookmarks path = %s", path));

  fd = fopen(path, "w");

  if (!fd) {
    g_print("Faild to open bookmarks file: %s", path);
    return;
  }

  g_list_foreach(links, (GFunc)save_bookmark, fd);

  fclose(fd);
}
