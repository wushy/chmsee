/*
 *  Copyright (c) 2006           Ji YongGang <jungle@soforge-studio.com>
 *  Copyright (c) 2014           Xianguang Zhou <xianguang.zhou@outlook.com>
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

#include "html.h"

#include <stdlib.h>
#include <string.h>

#include "ihtml.h"
#include "marshal.h"
#include "utils/utils.h"
//#include "gecko_utils.h"

static void html_class_init(HtmlClass *);
static void html_init(Html *);

//static void html_title_cb(GtkMozEmbed *, Html *);
static void html_title_cb(WebKitWebView *embed, WebKitWebFrame *frame, gchar *title,Html *html);
//static void html_location_cb(GtkMozEmbed *, Html *);
static void html_location_cb(WebKitWebView *embed, GParamSpec* pspec, Html *html);
//static gboolean html_open_uri_cb(GtkMozEmbed *, const gchar *, Html *);
static gboolean html_open_uri_cb(WebKitWebView *embed,
		WebKitWebFrame            *frame,
        WebKitNetworkRequest      *request,
        WebKitWebNavigationAction *navigation_action,
        WebKitWebPolicyDecision   *policy_decision,
        Html *html);
//static gboolean html_mouse_click_cb(GtkMozEmbed *, gpointer, Html *);
static gboolean html_mouse_click_cb(GtkWidget *widget, GdkEvent  *event, Html *html);
//static gboolean html_link_message_cb(GtkMozEmbed *, Html *);
static void html_link_message_cb(WebKitWebView *web_view,
        gchar         *title,
        gchar         *uri,
        Html *html);
//static void html_child_add_cb(GtkMozEmbed *, GtkWidget *, Html *);
static void html_child_add_cb(WebKitWebView *embed, GtkWidget *child, Html *html);
//static void html_child_remove_cb(GtkMozEmbed *, GtkWidget *, Html *);
static void html_child_remove_cb(WebKitWebView *, GtkWidget *, Html *);
static void html_child_grab_focus_cb(GtkWidget *, Html *);
static gboolean html_scroll_web_view_cb(GtkWidget *, GdkEvent  *, Html *);
static WebKitWebView* html_create_web_view_cb(WebKitWebView  *web_view, WebKitWebFrame *frame, Html *html);

/* Signals */
enum {
        TITLE_CHANGED,
        LOCATION_CHANGED,
        OPEN_URI,
        CONTEXT_NORMAL,
        CONTEXT_LINK,
        OPEN_NEW_TAB,
        LINK_MESSAGE,
        SCROLL_WEB_VIEW,
        LAST_SIGNAL
};



static gint signals[LAST_SIGNAL] = { 0 };

/* Has the value of the URL under the mouse pointer, otherwise NULL */
//static gchar *current_url = NULL;

static void chmsee_ihtml_interface_init (ChmseeIhtmlInterface *iface);

G_DEFINE_TYPE_WITH_CODE (Html, html, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (CHMSEE_TYPE_IHTML,
                                                chmsee_ihtml_interface_init));

static void
html_class_init(HtmlClass *klass)
{
        signals[TITLE_CHANGED] =
                g_signal_new ("title-changed",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_LAST,
                              0,
                              NULL, NULL,
                              marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1, G_TYPE_STRING);

        signals[LOCATION_CHANGED] =
                g_signal_new("location-changed",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_VOID__STRING,
                             G_TYPE_NONE,
                             1, G_TYPE_STRING);

        signals[OPEN_URI] =
                g_signal_new("open-uri",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_BOOLEAN__STRING,
                             G_TYPE_BOOLEAN,
                             1, G_TYPE_STRING);

        signals[CONTEXT_NORMAL] =
                g_signal_new("context-normal",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_VOID__VOID,
                             G_TYPE_NONE,
                             0);

        signals[CONTEXT_LINK] =
                g_signal_new("context-link",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_VOID__STRING,
                             G_TYPE_NONE,
                             1, G_TYPE_STRING);

//        signals[OPEN_NEW_TAB] =
//                g_signal_new("open-new-tab",
//                             G_TYPE_FROM_CLASS (klass),
//                             G_SIGNAL_RUN_LAST,
//                             0,
//                             NULL, NULL,
//                             marshal_VOID__STRING,
//                             G_TYPE_NONE,
//                             1, G_TYPE_STRING);

        signals[OPEN_NEW_TAB] =
                        g_signal_new("open-new-tab",
                                     G_TYPE_FROM_CLASS (klass),
                                     G_SIGNAL_RUN_LAST,
                                     0,
                                     NULL, NULL,
                                     marshal_POINTER__POINTER,
                                     G_TYPE_POINTER,
                                     1, G_TYPE_POINTER);

        signals[LINK_MESSAGE] =
                g_signal_new("link-message",
                             G_TYPE_FROM_CLASS (klass),
                             G_SIGNAL_RUN_LAST,
                             0,
                             NULL, NULL,
                             marshal_VOID__STRING,
                             G_TYPE_NONE,
                             1, G_TYPE_STRING);

        signals[SCROLL_WEB_VIEW] =
        		g_signal_new("scroll-web-view",
        					G_TYPE_FROM_CLASS(klass),
        					G_SIGNAL_RUN_LAST,
        					0,
        					NULL,NULL,
        					marshal_BOOLEAN__POINTER,
        					G_TYPE_BOOLEAN,
        					1, G_TYPE_POINTER);
}

static void
html_init(Html *html)
{
//        html->gecko = GTK_MOZ_EMBED(gtk_moz_embed_new());
		html->scrolled_window= GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
		html->gecko = WEBKIT_WEB_VIEW(webkit_web_view_new());
		gtk_widget_show(GTK_WIDGET(html->gecko));
		gtk_container_add(GTK_CONTAINER(html->scrolled_window),GTK_WIDGET(html->gecko));

//		g_object_set (G_OBJECT(html->gecko), "self-scrolling", true, NULL);
        gtk_drag_dest_unset(GTK_WIDGET(html->gecko));

        g_signal_connect(G_OBJECT (html->gecko),
//                         "title",
        				 "title-changed",
                         G_CALLBACK (html_title_cb),
                         html);
        g_signal_connect(G_OBJECT (html->gecko),
//                         "location",
        				 "notify::load-status",
                         G_CALLBACK (html_location_cb),
                         html);
        g_signal_connect(G_OBJECT (html->gecko),
//                         "open-uri",
        				 "navigation-policy-decision-requested",
                         G_CALLBACK (html_open_uri_cb),
                         html);
//        g_signal_connect(G_OBJECT (html->gecko),
////                         "dom_mouse_click",
//        				 "button-press-event",
//                         G_CALLBACK (html_mouse_click_cb),
//                         html);
        g_signal_connect(G_OBJECT (html->gecko),
//                         "link_message",
        				 "hovering-over-link",
                         G_CALLBACK (html_link_message_cb),
                         html);
        g_signal_connect(G_OBJECT (html->gecko),
                         "add",
                         G_CALLBACK (html_child_add_cb),
                         html);
        g_signal_connect(G_OBJECT (html->gecko),
                         "remove",
                         G_CALLBACK (html_child_remove_cb),
                         html);
        g_signal_connect(G_OBJECT(html->gecko),
        				 "scroll-event",
        				 G_CALLBACK(html_scroll_web_view_cb),
        				 html);
        g_signal_connect(G_OBJECT(html->gecko),
        				 "create-web-view",
        				 G_CALLBACK(html_create_web_view_cb),
        				 html);
}

/* callbacks */

static void
//html_title_cb(GtkMozEmbed *embed, Html *html)
html_title_cb(WebKitWebView *embed, WebKitWebFrame *frame, gchar *title,Html *html)
{
//	const char *new_title;

//        new_title = gtk_moz_embed_get_title(embed);
        const char *new_title = webkit_web_view_get_title(embed);
        g_signal_emit(html, signals[TITLE_CHANGED], 0, new_title);
//        g_free(new_title);
}

static void
//html_location_cb(GtkMozEmbed *embed, Html *html)
html_location_cb(WebKitWebView *embed, GParamSpec* pspec, Html *html)
{
//        char *location;

//        location = gtk_moz_embed_get_location(embed);
		WebKitLoadStatus status = webkit_web_view_get_load_status(embed);
		if(status==WEBKIT_LOAD_PROVISIONAL){
			const char *location = webkit_web_view_get_uri(embed);
			g_signal_emit(html, signals[LOCATION_CHANGED], 0, location);
		}
//        g_free(location);
}

static gboolean
//html_open_uri_cb(GtkMozEmbed *embed, const gchar *uri, Html *html)
html_open_uri_cb(WebKitWebView *embed,
		WebKitWebFrame            *frame,
        WebKitNetworkRequest      *request,
        WebKitWebNavigationAction *navigation_action,
        WebKitWebPolicyDecision   *policy_decision,
        Html *html)
{
	gboolean ret_val;
	gint button;
	gint mask;

	ret_val = TRUE;

	button = webkit_web_navigation_action_get_button(navigation_action);
	mask = webkit_web_navigation_action_get_modifier_state(navigation_action);

	const gchar *uri = webkit_network_request_get_uri(request);

	if (button == 2 || (button == 1 && mask & GDK_CONTROL_MASK)) {
		webkit_web_policy_decision_ignore (policy_decision);

		Html* new_html = NULL;
		g_signal_emit(html, signals[OPEN_NEW_TAB], 0, frame, &new_html);
		if (new_html != NULL) {
			html_open_uri(new_html, uri);
			ret_val = FALSE;
		}
	} else {
		g_signal_emit(html, signals[OPEN_URI], 0, uri, &ret_val);
	}

	/* Reset current url */
//        if (current_url != NULL) {
//                g_free(current_url);
//                current_url = NULL;
//
	g_signal_emit(html, signals[LINK_MESSAGE], 0, "");
//        }

	return ret_val;
}

static gboolean
//html_mouse_click_cb(GtkMozEmbed *widget, gpointer dom_event, Html *html)
html_mouse_click_cb(GtkWidget *widget, GdkEvent  *event, Html *html)
{
////        gint button;
//		guint button;
////        gint mask;
//		guint mask;

////        button = gecko_utils_get_mouse_event_button(dom_event);
//        button = event->button.button;
////        mask = gecko_utils_get_mouse_event_modifiers(dom_event);
//        mask = event->button.state;

//        const gchar * current_url;
//        WebKitHitTestResult * hitTestResult=webkit_web_view_get_hit_test_result(html->gecko,&(event->button));
//        WebKitHitTestResultContext * hitTestResultContext=(WebKitHitTestResultContext *)g_object_get_data(G_OBJECT(hitTestResult),"context");
//        fprintf(stderr,"-----------------------\n");
//        fprintf(stderr,"is link : %d\n",(int)(hitTestResultContext==WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK));

//        WebKitDOMNode*  dom_node=(WebKitDOMNode *)g_object_get_data(G_OBJECT(hitTestResult),"inner-node");
//        const gchar * node_name=webkit_dom_node_get_node_name(dom_node);
//        fprintf(stderr,"node name : %s\n",node_name);
//        WebKitDOMNamedNodeMap* attrMap= webkit_dom_node_get_attributes(domNode);
//        webkit_dom_named_node_map_get_named_item();

//        current_url=(const gchar *)g_object_get_data(G_OBJECT(hitTestResult),"link-uri");

//        fprintf(stderr,"click link : %s\n",current_url);

//        if(hitTestResultContext==WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK){
//        	current_url=(const gchar *)g_object_get_data(G_OBJECT(hitTestResult),"link-uri");
//        	fprintf(stderr,"click link : %s\n",current_url);
//        }else{
//        	current_url=NULL;
//        }

//        if (button == 2 || (button == 1 && mask & GDK_CONTROL_MASK)) {
//                if (current_url) {
//                        g_signal_emit(html, signals[OPEN_NEW_TAB], 0, current_url);
//
//                        return TRUE;
//                }
//        }
//		else if (button == 3) {
//                if (current_url)
//                        g_signal_emit(html, signals[CONTEXT_LINK], 0, current_url);
//                else
//                        g_signal_emit(html, signals[CONTEXT_NORMAL], 0);
//
//                return TRUE;
//        }

        return FALSE;
}

//static gboolean
//html_link_message_cb(GtkMozEmbed *widget, Html *html)
static void
html_link_message_cb(WebKitWebView *web_view,
        gchar         *title,
        gchar         *uri,
        Html *html)
{
//        g_free(current_url);

		const gchar * current_url = uri!=NULL ? uri : "" ;

//        if(uri!=NULL){
//        current_url = (gchar *)malloc(strlen((char *)uri)+1);
//        if(current_url!=NULL){
//        strcpy((char *)current_url,(char *)uri);

        g_signal_emit(html, signals[LINK_MESSAGE], 0, current_url);

//        if (current_url!=NULL && current_url[0] == '\0') {
//                free(current_url);
//                current_url = NULL;
//        }
//        }
//        }

//        return FALSE;
}

static void
//html_child_add_cb(GtkMozEmbed *embed, GtkWidget *child, Html *html)
html_child_add_cb(WebKitWebView *embed, GtkWidget *child, Html *html)
{
        g_signal_connect(G_OBJECT (child),
                         "grab-focus",
                         G_CALLBACK (html_child_grab_focus_cb),
                         html);
}

static void
//html_child_remove_cb(GtkMozEmbed *embed, GtkWidget *child, Html *html)
html_child_remove_cb(WebKitWebView *embed, GtkWidget *child, Html *html)
{
        g_signal_handlers_disconnect_by_func(child, html_child_grab_focus_cb, html);
}

static void
html_child_grab_focus_cb(GtkWidget *widget, Html *html)
{
        GdkEvent *event;

        event = gtk_get_current_event();

        if (event == NULL)
                g_signal_stop_emission_by_name(widget, "grab-focus");
        else
                gdk_event_free(event);
}

static gboolean
html_scroll_web_view_cb(GtkWidget *widget, GdkEvent  *event, Html *html)
{
	gboolean ret_val;
	g_signal_emit(html, signals[SCROLL_WEB_VIEW], 0, event, &ret_val);
	return ret_val;
}

static WebKitWebView*
html_create_web_view_cb(WebKitWebView  *web_view, WebKitWebFrame *frame, Html *html)
{
	Html* new_html=NULL;
	g_signal_emit(html, signals[OPEN_NEW_TAB], 0, frame, &new_html);
	if(new_html!=NULL){
		return new_html->gecko;
	}else{
		return NULL;
	}
}

/* external functions */

Html *
html_new(void)
{
        Html *html;

        html = g_object_new(TYPE_HTML, NULL);

        return html;
}

void
html_clear(Html *html)
{
        static const char *data = "<html><body bgcolor=\"white\"></body></html>";

        g_return_if_fail(IS_HTML (html));

//        gtk_moz_embed_render_data(html->gecko, data, strlen(data), "file:///", "text/html");
        webkit_web_view_load_html_string(html->gecko,data,"file:///");
}

void
html_open_uri(Html *html, const gchar *str_uri)
{
        gchar *full_uri;

        g_return_if_fail(IS_HTML (html));
        g_return_if_fail(str_uri != NULL);

        if (str_uri[0] == '/')
                full_uri = g_strdup_printf("file://%s", str_uri);
        else
                full_uri = g_strdup(str_uri);

        g_debug("Open uri %s", full_uri);
//        gtk_moz_embed_load_url(html->gecko, full_uri);
        webkit_web_view_load_uri(html->gecko,full_uri);
        g_free(full_uri);
}

GtkWidget *
html_get_widget(Html *html)
{
        g_return_val_if_fail(IS_HTML (html), NULL);

        return GTK_WIDGET (html->scrolled_window);
}

gboolean
html_can_go_forward(Html *html)
{
        g_return_val_if_fail(IS_HTML (html), FALSE);

//        return gtk_moz_embed_can_go_forward(html->gecko);
        return webkit_web_view_can_go_forward(html->gecko);
}

gboolean
html_can_go_back(Html *html)
{
        g_return_val_if_fail(IS_HTML (html), FALSE);

//        return gtk_moz_embed_can_go_back(html->gecko);
        return webkit_web_view_can_go_back(html->gecko);
}

void
html_go_forward(Html *html)
{
        g_return_if_fail(IS_HTML (html));

//        gtk_moz_embed_go_forward(html->gecko);
        webkit_web_view_go_forward(html->gecko);
}

void
html_go_back(Html *html)
{
        g_return_if_fail(IS_HTML (html));

//        gtk_moz_embed_go_back(html->gecko);
        webkit_web_view_go_back(html->gecko);
}

const gchar *
html_get_title(Html *html)
{
        g_return_val_if_fail(IS_HTML (html), NULL);

//        return gtk_moz_embed_get_title(html->gecko);
        return webkit_web_view_get_title(html->gecko);
}

const gchar *
html_get_location(Html *html)
{
        g_return_val_if_fail(IS_HTML (html), NULL);

//        return gtk_moz_embed_get_location(html->gecko);
        return webkit_web_view_get_uri(html->gecko);
}

void
html_copy_selection(Html *html)
{
        g_return_if_fail(IS_HTML (html));

//        gecko_utils_copy_selection(html->gecko);
        webkit_web_view_copy_clipboard(html->gecko);
}

void
html_select_all(Html *html)
{
        g_return_if_fail(IS_HTML (html));

//        gecko_utils_select_all(html->gecko);
        webkit_web_view_select_all(html->gecko);
}

void
html_increase_size(Html *html)
{
//        gfloat zoom;

        g_return_if_fail(IS_HTML (html));

//        zoom = gecko_utils_get_zoom(html->gecko);
//        zoom *= 1.2;
//
//        gecko_utils_set_zoom(html->gecko, zoom);
        webkit_web_view_zoom_in(html->gecko);
}

void
html_reset_size(Html *html)
{
        g_return_if_fail(IS_HTML (html));

//        gecko_utils_set_zoom(html->gecko, 1.0);
        webkit_web_view_set_zoom_level(html->gecko, 1.0);
}

void
html_decrease_size(Html *html)
{
//        gfloat zoom;

        g_return_if_fail(IS_HTML (html));

//        zoom = gecko_utils_get_zoom(html->gecko);
//        zoom /= 1.2;
//
//        gecko_utils_set_zoom(html->gecko, zoom);
        webkit_web_view_zoom_out(html->gecko);
}

void html_shutdown(Html* html) {
//  gecko_utils_shutdown();
}

void html_init_system(void) {
//  gecko_utils_init();
}

void html_set_default_lang(gint lang) {
//  gecko_utils_set_default_lang(lang);
}

void html_set_variable_font(Html* html, const gchar* font) {
//  gecko_utils_set_font(GECKO_PREF_FONT_VARIABLE, font);
	WebKitWebSettings * settings=webkit_web_view_get_settings(html->gecko);
	g_object_set (G_OBJECT(settings), "serif-font-family", font, NULL);
}

void html_set_fixed_font(Html* html, const gchar* font) {
//  gecko_utils_set_font(GECKO_PREF_FONT_FIXED, font);
	WebKitWebSettings * settings=webkit_web_view_get_settings(html->gecko);
	g_object_set (G_OBJECT(settings), "sans-serif-font-family", font, NULL);
}


void chmsee_ihtml_interface_init (ChmseeIhtmlInterface *iface) {
  iface->get_title = html_get_title;
  iface->get_location = html_get_location;
  iface->can_go_back = html_can_go_back;
  iface->can_go_forward = html_can_go_forward;

  iface->open_uri = html_open_uri;
  iface->copy_selection = html_copy_selection;
  iface->select_all = html_select_all;
  iface->go_back = html_go_back;
  iface->go_forward = html_go_forward;
  iface->increase_size = html_increase_size;
  iface->decrease_size = html_decrease_size;
  iface->reset_size = html_reset_size;
  iface->set_variable_font = html_set_variable_font;
  iface->set_fixed_font = html_set_fixed_font;
  iface->clear = html_clear;
  iface->shutdown = html_shutdown;
  iface->get_widget = html_get_widget;
}
