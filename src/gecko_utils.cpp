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

/*
 * Copyright (C) 2004 Imendio AB
 * Copyright (C) 2004 Marco Pesenti Gritti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"
#include "gecko_utils.h"
#include <stdlib.h>

#ifdef XPCOM_GLUE
#include <gtkmozembed_glue.cpp>
#endif

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

#include <nsCOMPtr.h>
#include <nsMemory.h>
#include <nsEmbedString.h>
#include <nsIPrefService.h>
#include <nsICommandManager.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsIDOMWindow.h>

#define MOZILLA_INTERNAL_API
#include <nsIServiceManager.h>
#undef MOZILLA_INTERNAL_API

#include <nsISupportsPrimitives.h>
#include <nsILocalFile.h>
#include <nsIDOMMouseEvent.h>
#include <nsIWebBrowserFind.h>
#include <nsILocaleService.h>
#include <nsStringAPI.h>

#include "utils/utils.h"

#define LANG_TYPES_NUM 	7

static const gchar *lang[] = {
        "universal_charset_detector",
        "zhcn_parallel_state_machine",
        "zhtw_parallel_state_machine",
        "jp_parallel_state_machine",
        "ko_parallel_state_machine",
        "ruprob",
        "ukprob"
};

static nsresult
do_command(GtkMozEmbed *embed, const char *command)
{
	nsCOMPtr<nsIWebBrowser>     webBrowser;
	nsCOMPtr<nsICommandManager> cmdManager;

	gtk_moz_embed_get_nsIWebBrowser(embed, getter_AddRefs(webBrowser));

	cmdManager = do_GetInterface(webBrowser);

	return cmdManager->DoCommand(command, nsnull, nsnull);
}

static gboolean
util_split_font_string(const gchar *font_name, gchar **name, gint *size)
{
	PangoFontDescription *desc;
	PangoFontMask         mask;
	gboolean              retval = FALSE;

	if (font_name == NULL) {
		return FALSE;
	}

	mask = (PangoFontMask) (PANGO_FONT_MASK_FAMILY | PANGO_FONT_MASK_SIZE);

	desc = pango_font_description_from_string(font_name);
	if (!desc) {
		return FALSE;
	}

	if ((pango_font_description_get_set_fields(desc) & mask) == mask) {
		*size = PANGO_PIXELS(pango_font_description_get_size (desc));
		*name = g_strdup(pango_font_description_get_family (desc));
		retval = TRUE;
	}

	pango_font_description_free(desc);

	return retval;
}

/*
static gboolean
gecko_prefs_set_bool(const gchar *key, gboolean value)
{
	nsresult rv;
	nsCOMPtr<nsIPrefService> prefService(do_GetService(NS_PREFSERVICE_CONTRACTID, &rv));
	NS_ENSURE_SUCCESS (rv, FALSE);

	nsCOMPtr<nsIPrefBranch> pref;
	rv = prefService->GetBranch("", getter_AddRefs(pref));
	NS_ENSURE_SUCCESS (rv, FALSE);

	rv = pref->SetBoolPref(key, value);

	return NS_SUCCEEDED (rv) != PR_FALSE;
}
*/

static gboolean
gecko_prefs_set_string(const gchar *key, const gchar *value)
{
	nsresult rv;
	nsCOMPtr<nsIPrefService> prefService(do_GetService(NS_PREFSERVICE_CONTRACTID, &rv));
	NS_ENSURE_SUCCESS (rv, FALSE);

	nsCOMPtr<nsIPrefBranch> pref;
	rv = prefService->GetBranch("", getter_AddRefs(pref));
	NS_ENSURE_SUCCESS (rv, FALSE);

	rv = pref->SetCharPref(key, value);

	return NS_SUCCEEDED (rv) != PR_FALSE;
}

static gboolean
gecko_prefs_set_int(const gchar *key, gint value)
{
	nsresult rv;
	nsCOMPtr<nsIPrefService> prefService(do_GetService(NS_PREFSERVICE_CONTRACTID, &rv));
	NS_ENSURE_SUCCESS (rv, FALSE);

	nsCOMPtr<nsIPrefBranch> pref;
	rv = prefService->GetBranch("", getter_AddRefs(pref));
	NS_ENSURE_SUCCESS (rv, FALSE);

	rv = pref->SetIntPref(key, value);

	return NS_SUCCEEDED (rv) != PR_FALSE;
}

static nsresult
gecko_utils_init_prefs(void)
{
	nsresult rv;

	nsCOMPtr<nsIPrefService> prefService(do_GetService(NS_PREFSERVICE_CONTRACTID, &rv));
	NS_ENSURE_SUCCESS (rv, rv);

	nsCOMPtr<nsILocalFile> file;
	rv = NS_NewNativeLocalFile(nsEmbedCString(get_resource_path("default-prefs.js")),
                                   PR_TRUE, getter_AddRefs(file));
	NS_ENSURE_SUCCESS (rv, rv);

	rv = prefService->ReadUserPrefs(file);
	rv |= prefService->ReadUserPrefs(nsnull);
	NS_ENSURE_SUCCESS (rv, rv);

	return rv;
}

extern "C" void
gecko_utils_init(void)
{
	if (!g_thread_supported())
		g_thread_init(NULL);

        nsresult rv;

#ifdef XPCOM_GLUE
	NS_LogInit();

        static const GREVersionRange greVersion = {
                "1.9a", PR_TRUE,
                "1.9.*", PR_TRUE
        };

        char xpcomLocation[4096];
        rv = GRE_GetGREPathWithProperties(&greVersion, 1, nsnull, 0, xpcomLocation, 4096);
        if (NS_FAILED (rv))
        {
                g_warning ("Could not determine locale!\n");
                return;
        }

        // Startup the XPCOM Glue that links us up with XPCOM.
        rv = XPCOMGlueStartup(xpcomLocation);
        if (NS_FAILED (rv))
        {
                g_warning ("Could not determine locale!\n");
                return;
        }

        rv = GTKEmbedGlueStartup();
        if (NS_FAILED (rv))
        {
                g_warning ("Could not startup embed glue!\n");
                return;
        }

        rv = GTKEmbedGlueStartupInternal();
        if (NS_FAILED (rv))
        {
                g_warning ("Could not startup embed glue (internal)!\n");
                return;
        }

        char *lastSlash = strrchr(xpcomLocation, '/');
        if (lastSlash)
                *lastSlash = '\0';

        gtk_moz_embed_set_path(xpcomLocation);
#else
	gtk_moz_embed_set_comp_path(GECKO_LIB_ROOT);
#endif

        gchar *profile_dir = g_build_filename(g_get_home_dir(),
                                              ".chmsee",
                                              NULL);

	gtk_moz_embed_set_profile_path(profile_dir, "mozilla");
	g_free(profile_dir);

	gtk_moz_embed_push_startup();

	gecko_utils_init_prefs();
}

extern "C" void
gecko_utils_shutdown(void)
{
	gtk_moz_embed_pop_startup();

#ifdef XPCOM_GLUE
	NS_LogTerm();
#endif
}

extern "C" gint
gecko_utils_get_mouse_event_button(gpointer event)
{
	nsIDOMMouseEvent *aMouseEvent;
	PRUint16          button;

	aMouseEvent = (nsIDOMMouseEvent *) event;

	aMouseEvent->GetButton(&button);

	return button + 1;
}

extern "C" gint
gecko_utils_get_mouse_event_modifiers(gpointer event)
{
	nsIDOMMouseEvent *aMouseEvent;
	PRBool            ctrl, alt, shift, meta;
	gint              mask;

	aMouseEvent = (nsIDOMMouseEvent *) event;

	aMouseEvent->GetCtrlKey(&ctrl);
	aMouseEvent->GetAltKey(&alt);
	aMouseEvent->GetShiftKey(&shift);
	aMouseEvent->GetMetaKey(&meta);

	mask = 0;
	if (ctrl) {
		mask |= GDK_CONTROL_MASK;
	}
	if (alt || meta) {
		mask |= GDK_MOD1_MASK;
	}
	if (shift) {
		mask |= GDK_SHIFT_MASK;
	}

	return mask;
}

extern "C" void
gecko_utils_set_font(gint type, const gchar *fontname)
{
	gchar *name;
	gint   size;

	name = NULL;

	if (!util_split_font_string(fontname, &name, &size)) {
		g_free(name);
		return;
	}

	switch (type) {
	case GECKO_PREF_FONT_VARIABLE:
		gecko_prefs_set_string("font.name.variable.x-western", name);
		gecko_prefs_set_int("font.size.variable.x-western", size);
		break;
	case GECKO_PREF_FONT_FIXED:
		gecko_prefs_set_string("font.name.fixed.x-western", name);
		gecko_prefs_set_int("font.size.fixed.x-western", size);
		break;
	}

	g_free(name);
}

extern "C" void
gecko_utils_set_default_lang(gint type)
{
        if (type < LANG_TYPES_NUM )
                gecko_prefs_set_string("intl.charset.detector", lang[type]);
}

extern "C" void
gecko_utils_select_all(GtkMozEmbed *embed)
{
 	do_command(embed, "cmd_selectAll");
}

extern "C" void
gecko_utils_copy_selection(GtkMozEmbed *embed)
{
	do_command(embed, "cmd_copy");
}

extern "C" gfloat
gecko_utils_get_zoom(GtkMozEmbed *embed)
{
	nsCOMPtr<nsIWebBrowser>	webBrowser;
	nsCOMPtr<nsIDOMWindow> 	domWindow;
	float zoom;

	gtk_moz_embed_get_nsIWebBrowser(GTK_MOZ_EMBED(embed), getter_AddRefs(webBrowser));
	webBrowser->GetContentDOMWindow(getter_AddRefs(domWindow));

	if (!domWindow) {
		g_warning("could not get DOMWindow.");
		return 1.0;
	}

	domWindow->GetTextZoom(&zoom);

	return zoom;
}

extern "C" void
gecko_utils_set_zoom(GtkMozEmbed *embed, gfloat zoom)
{
	nsCOMPtr<nsIWebBrowser>	webBrowser;
	nsCOMPtr<nsIDOMWindow> 	domWindow;

	gtk_moz_embed_get_nsIWebBrowser(GTK_MOZ_EMBED(embed), getter_AddRefs(webBrowser));
	webBrowser->GetContentDOMWindow(getter_AddRefs(domWindow));

	if (!domWindow) {
		g_warning("Could not get DOMWindow.");
		return;
	}

	domWindow->SetTextZoom(zoom);
}
