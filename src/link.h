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

#ifndef __LINK_H__ 
#define __LINK_H__ 

#include <glib.h>

typedef struct _Link   Link;

#define LINK(x) ((Link *) x)

typedef enum {
        LINK_TYPE_BOOK,
        LINK_TYPE_PAGE,
} LinkType;

struct _Link {
        gchar   *name;
        gchar   *uri;
        LinkType type;
        
        guint    ref_count;
};

Link *link_new(LinkType, const gchar *, const gchar *);
Link *link_copy(const Link *);
void link_free(Link *);

gint link_compare(gconstpointer, gconstpointer);
void link_change_type(Link *, LinkType);
Link *ink_ref(Link *);
void link_unref(Link *);

#endif /* __LINK_H__ */

