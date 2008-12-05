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
#include "chmfile.h"

static int show_help;
static int show_version;

/* Short options */
static char const short_options[] = "hV";

/* Long options */
static struct option long_options[] =
{
        {"help",     no_argument,        &show_help,     1},
        {"version",  no_argument,        &show_version,  1},
        {0,          0,                  0,              0}
};

static void
display_version(int exitflag)
{
        printf("ChmSee %s\n\n", PACKAGE_VERSION);
        printf("Copyright (C) 2006  Ji YongGang <jungle@soforge-studio.com>\n");
        printf("This is free software; see the source for copying conditions. There is NO\n");
        printf("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n");

        if (exitflag)
                exit(exitflag - 1);
}

static void 
display_usage(int exitflag)
{
        printf("Gtk2+ based CHM file viewer\n\n");
        printf("Usage: chmsee [OPTION] [chmfile]\n");
        printf("Example: chmsee FreeBSD_Handbook.chm\n");
        printf("\nOptions:\n");
        printf("  -h, --help                display this help and exit\n");
        printf("  -V, --version             print version information and exit\n");
        printf("\nReport bugs to jungle@soforge-studio.com\n");

        if (exitflag)
                exit(exitflag - 1); 
}

int
main(int argc, char *argv[])
{
        extern char *optarg;
        extern int optind;

        ChmSee *chmsee;
        gchar **filename;
        int opt;

        if (!g_thread_supported()) 
                g_thread_init(NULL);

        gtk_init(&argc, &argv);

        /* Parse command line */
        while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
                switch (opt) {
                case 'h':
                        display_usage(1);
                        break;
                case 'V':
                        display_version(1);
                        break;
                case 0:
                        /* Long options */
                        break;
                default:
                        display_usage(2);
                        break;
                }
        }

        if (show_help)
                display_usage(1);

        if (show_version)
                display_version(1);

        filename = argv + optind;

        /* i18n */
        bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
        textdomain(GETTEXT_PACKAGE);

        /* Show splash screen */
        startup_popup_new();

        /* Create main window */
        chmsee = chmsee_new();

        if (*filename)
                chmsee_open_file(chmsee, (const gchar *)*filename);

        gtk_widget_show(GTK_WIDGET (chmsee));

        gtk_main();

        return 0;
}
