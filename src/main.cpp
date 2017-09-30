/*
 *  Copyright (c) 2006           Ji YongGang <jungle@soforge-studio.com>
 *  Copyright (C) 2009 LI Daobing <lidaobing@gmail.com>
 *  Copyright (c) 2014 Xianguang Zhou <xianguang.zhou@outlook.com>
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

#include "config.h"

#include <stdlib.h>
#include <getopt.h>
#include <libintl.h>

#if defined(__linux__)
#include <errno.h>
#include <pthread.h>
#include <gcrypt.h>
#endif

#include "chmsee.h"
#include "startup.h"
#include "utils/utils.h"

static void dummy_log_handler (const gchar *log_domain,
                               GLogLevelFlags log_level,
                               const gchar *message,
                               gpointer unused_data)
{}

static void init_log(int log_level) {
  if(log_level < 1) g_log_set_handler(NULL, G_LOG_LEVEL_CRITICAL, dummy_log_handler, NULL);
  if(log_level < 2) g_log_set_handler(NULL, G_LOG_LEVEL_WARNING, dummy_log_handler, NULL);
  if(log_level < 3) g_log_set_handler(NULL, G_LOG_LEVEL_MESSAGE, dummy_log_handler, NULL);
  if(log_level < 4) g_log_set_handler(NULL, G_LOG_LEVEL_INFO, dummy_log_handler, NULL);
  if(log_level < 5) g_log_set_handler(NULL, G_LOG_LEVEL_DEBUG, dummy_log_handler, NULL);
}

static int log_level = 2; /* only show WARNING, CRITICAL, ERROR */
static gboolean callback_verbose(const gchar *option_name,
                                 const gchar *value,
                                 gpointer data,
                                 GError **error) {
  log_level++;
  return TRUE;
}

static gboolean callback_quiet(const gchar *option_name,
                               const gchar *value,
                               gpointer data,
                               GError **error) {
  log_level--;
  return TRUE;
}

#if defined(__linux__)
GCRY_THREAD_OPTION_PTHREAD_IMPL;
#endif

int
main(int argc, char** argv)
{
  ChmSee *chmsee;
  const gchar* filename = NULL;
  const gchar* datadir = NULL;
  GError* error = NULL;
  gboolean option_version = FALSE;

#if defined(__linux__)
  if (!gcry_check_version(NULL)) {
    fprintf(stderr, "failed to initialize gcrypt\n");
    exit(1);
  }
  gcry_control (GCRYCTL_SET_THREAD_CBS,&gcry_threads_pthread);
  gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
#endif

  if (!g_thread_supported())
    g_thread_init(NULL);


  GOptionEntry options[] = {
		  {"version", 0,
				  0, G_OPTION_ARG_NONE, &option_version,
				  _("Display the version and exit"),
				  NULL
		  },
		  {"verbose", 'v',
				  G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void*)callback_verbose,
				  _("be verbose, repeat 3 times to get all info"),
				  NULL
		  },
		  {"quiet", 'q',
				  G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void*)callback_quiet,
				  _("be quiet, repeat 2 times to disable all info"),
				  NULL
		  },
		  {"datadir", 0,
				  G_OPTION_FLAG_FILENAME, G_OPTION_ARG_FILENAME, &datadir,
				  "choose data dir, default is " CHMSEE_DATA_DIR_DEFAULT,
				  "PATH"
		  },
		  {NULL}
  };
  if(!gtk_init_with_args(&argc, &argv,
                         "[chmfile]\n"
                         "\n"
                         "GTK+ based CHM file viewer\n"
                         "Example: chmsee FreeBSD_Handbook.chm"
                         ,
                         options,
                         g_strdup(GETTEXT_PACKAGE),
                         &error))
  {
    g_printerr("%s\n", error->message);
    return 1;
  }
  if(option_version) {
    g_print("%s\n", PACKAGE_STRING);
    return 0;
  }

  init_log(log_level);
  if(datadir != NULL) {
	  set_data_dir(datadir);
  }

  if(argc == 1) {
  } else if(argc == 2) {
    filename = argv[1];
  } else {
    g_printerr(_("more than 1 argument\n"));
    return 1;
  }

  gtk_rc_parse_string("style \"tab-close-button-style\" {\n"
    			"GtkWidget::focus-padding = 0\n"
    			"GtkWidget::focus-line-width = 0\n"
    			"xthickness = 0\n"
    			"ythickness = 0\n"
    			"}\n"
    			"widget \"*.tab-close-button\" style \"tab-close-button-style\"");

  /* i18n */
  bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  /* Show splash screen */
  startup_popup_new();

  /* Create main window */
  chmsee = chmsee_new(filename);

  gtk_widget_show(GTK_WIDGET (chmsee));

  gtk_main();

  return 0;
}
