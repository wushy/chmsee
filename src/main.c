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

/***************************************************************************
 *   Copyright (C) 2003 by zhong                                           *
 *   zhongz@163.com                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <stdlib.h>
#include <getopt.h>
#include <libintl.h>

#include "chmsee.h"
#include "startup.h"
#include "utils.h"

int
main(int argc, char** argv)
{
        extern char *optarg;
        extern int optind;

        ChmSee *chmsee;
        const gchar* filename = NULL;
        GError* error = NULL;
        gboolean option_version = FALSE;

        if (!g_thread_supported()) 
                g_thread_init(NULL);

        GOptionEntry options[] = {
          { "version", 'v',
            0, G_OPTION_ARG_NONE, &option_version,
            _("Display the version and exit"),
            NULL
          }
        };
        if(!gtk_init_with_args(&argc, &argv,
                               "[chmfile]\n"
                               "\n"
                               "GTK+ based CHM file viewer\n"
                               "Example: chmsee FreeBSD_Handbook.chm"
                               ,
                               options, GETTEXT_PACKAGE, &error)) {
          g_printerr("%s\n", error->message);
          return 1;
        }
        if(option_version) {
          g_print("%s\n", PACKAGE_STRING);
          return 0;
        }

        if(argc == 1) {
        } else if(argc == 2) {
          filename = argv[1];
        } else {
          g_printerr(_("more than 1 argument\n"));
          return 1;
        }

        /* i18n */
        bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
        textdomain(GETTEXT_PACKAGE);

        /* Show splash screen */
        startup_popup_new();

        /* Create main window */
        chmsee = chmsee_new();

        if (filename) {
          chmsee_open_file(chmsee, filename);
        }

        gtk_widget_show(GTK_WIDGET (chmsee));

        gtk_main();

        return 0;
}
