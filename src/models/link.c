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

#include "config.h"
#include "link.h"

#include <string.h>

#include "utils/utils.h"

Link *
link_new(LinkType type, const gchar *name, const gchar *uri)
{
        Link *link;

        g_return_val_if_fail(name != NULL, NULL);
        g_return_val_if_fail(uri != NULL, NULL);

        link = g_new0(Link, 1);

        link->type = type;

        link->name = g_strdup(name);
        link->uri  = g_strdup(uri);
        
        return link;
}

void 
link_free(Link *link)
{
        g_free(link->name);
        g_free(link->uri);

        g_free(link);
}

Link *
link_copy(const Link *link)
{
        return link_new(link->type, link->name, link->uri);
}

gint
link_compare(gconstpointer a, gconstpointer b)
{
        return ncase_compare_utf8_string(((Link *)a)->uri, ((Link *)b)->uri);
}

void 
link_change_type(Link *link, LinkType type)
{
        link->type = type;
}

Link *
link_ref(Link *link)
{
        g_return_val_if_fail(link != NULL, NULL);

        link->ref_count++;
        
        return link;
}

void
link_unref(Link *link)
{
        g_return_if_fail(link != NULL);
        
        link->ref_count--;

        if (link->ref_count == 0)
                link_free(link);
}

