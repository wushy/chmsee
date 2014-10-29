/*
 *  Copyright (c) 2014 Xianguang Zhou <624146104@qq.com>
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

#include <gdk/gdkkeysyms.h>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <xapian.h>
#include <htmlcxx/html/ParserDom.h>
#include "marshal.h"
#include "search_list.h"

struct _SearchListPrivate {
	GtkWidget * entry;

	GtkWidget * list;
	GtkListStore *store;

	const gchar * book_dir;
	const gchar * home_dir;

	Xapian::Database * database;
};

#define selfp (self->priv)
#define SEARCHLIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_SEARCHLIST, SearchListPrivate))

static void searchlist_class_init(SearchListClass *);
static void searchlist_init(SearchList *);
static void searchlist_finalize(GObject *);
static void searchlist_dispose(GObject*);
static void searchlist_search(SearchList *);
static void searchlist_index(SearchList *);
static void searchlist_clear(SearchList *);
static void searchlist_clear_list_store(SearchList *);

static void searchlist_entry_icon_press_cb(GtkEntry *entry,
		GtkEntryIconPosition icon_pos, GdkEvent *event, SearchList* self);
static gboolean searchlist_entry_key_press_cb(GtkEntry *entry, GdkEvent *event,
		SearchList * self);
static void searchlist_index_button_clicked_cb(GtkButton * button,
		SearchList * self);
static void searchlist_clear_button_clicked_cb(GtkButton * button,
		SearchList * self);
static void searchlist_list_selection_changed_cb(
		GtkTreeSelection * tree_selection, SearchList * self);

G_DEFINE_TYPE(SearchList, searchlist, GTK_TYPE_VBOX);

/* Signals */
enum {
	OPEN_SEARCHED_PAGE, LAST_SIGNAL
};
static gint signals[LAST_SIGNAL] = { 0 };

static void searchlist_class_init(SearchListClass *klass) {
	g_type_class_add_private(klass, sizeof(SearchListPrivate));
	G_OBJECT_CLASS(klass)->finalize = searchlist_finalize;
	G_OBJECT_CLASS(klass)->dispose = searchlist_dispose;

	signals[OPEN_SEARCHED_PAGE] = g_signal_new("open-searched-page",
			G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0,
			NULL, NULL,
			marshal_VOID__STRING,
			G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void searchlist_init(SearchList* self) {
	self->priv = SEARCHLIST_GET_PRIVATE(self);

	selfp->entry = gtk_entry_new();
	gtk_entry_set_icon_from_stock(GTK_ENTRY(selfp->entry),
			GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FIND);
	gtk_box_pack_start(GTK_BOX(self), selfp->entry, FALSE, TRUE, 1);
	g_signal_connect(G_OBJECT(selfp->entry), "icon-press",
			G_CALLBACK(searchlist_entry_icon_press_cb), self);
	g_signal_connect(G_OBJECT(selfp->entry), "key-press-event",
			G_CALLBACK(searchlist_entry_key_press_cb), self);

	GtkWidget * list_sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list_sw),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(self), list_sw, TRUE, TRUE, 4);

	selfp->store = gtk_list_store_new(1, G_TYPE_STRING);
	selfp->list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(selfp->store));
	gtk_container_add(GTK_CONTAINER(list_sw), selfp->list);
	GtkTreeSelection *tree_selection = gtk_tree_view_get_selection(
			GTK_TREE_VIEW(selfp->list));
	gtk_tree_selection_set_mode(tree_selection, GTK_SELECTION_SINGLE);
	g_signal_connect(tree_selection, "changed",
			G_CALLBACK (searchlist_list_selection_changed_cb), self);

	GtkWidget * bottom_box = gtk_hbox_new(TRUE, 4);
	gtk_box_pack_start(GTK_BOX(self), bottom_box, FALSE, TRUE, 4);

	GtkWidget * index_button = gtk_button_new_with_label("Index");
	gtk_box_pack_start(GTK_BOX(bottom_box), index_button, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(index_button), "clicked",
			G_CALLBACK(searchlist_index_button_clicked_cb), self);

	GtkWidget * clear_button = gtk_button_new_with_label("Clear");
	gtk_box_pack_start(GTK_BOX(bottom_box), clear_button, FALSE, TRUE, 0);
	g_signal_connect(G_OBJECT(clear_button), "clicked",
			G_CALLBACK(searchlist_clear_button_clicked_cb), self);
}

static void searchlist_finalize(GObject *object) {
	SearchList* self = SEARCHLIST(object);

	searchlist_clear_list_store(self);
	g_object_unref(selfp->store);

	if (selfp->database) {
		selfp->database->close();
		delete selfp->database;
		selfp->database = NULL;
	}

	G_OBJECT_CLASS(searchlist_parent_class)->finalize(object);
}

static void searchlist_dispose(GObject* gobject) {
//	SearchList* self = SEARCHLIST(gobject);

	G_OBJECT_CLASS(searchlist_parent_class)->dispose(gobject);
}

GtkWidget * searchlist_new(const gchar * book_dir, const gchar * home_dir) {
	SearchList * self;

	self = SEARCHLIST(g_object_new(TYPE_SEARCHLIST, NULL));

	selfp->book_dir = book_dir;
	selfp->home_dir = home_dir;

	return GTK_WIDGET(self);
}

static void searchlist_entry_icon_press_cb(GtkEntry *entry,
		GtkEntryIconPosition icon_pos, GdkEvent *event, SearchList* self) {
	searchlist_search(self);
}

static gboolean searchlist_entry_key_press_cb(GtkEntry *entry, GdkEvent *event,
		SearchList* self) {
	if (event->key.keyval == GDK_KEY_Return) {
		searchlist_search(self);
	}
	return FALSE;
}

static void searchlist_index_button_clicked_cb(GtkButton * button,
		SearchList * self) {
	searchlist_index(self);
}

static void searchlist_clear_button_clicked_cb(GtkButton * button,
		SearchList * self) {
	searchlist_clear(self);
}

static void searchlist_list_selection_changed_cb(
		GtkTreeSelection * tree_selection, SearchList * self) {
	GtkTreeIter tree_iter;

	if (gtk_tree_selection_get_selected(tree_selection, NULL, &tree_iter)) {
		std::string unescaped_uri_string = std::string("file://")
				+ std::string(selfp->book_dir) + std::string("/")
				+ (*((std::string *) (tree_iter.user_data)));
		char * uri = g_uri_escape_string(unescaped_uri_string.c_str(), NULL,
				FALSE);
		g_signal_emit(self, signals[OPEN_SEARCHED_PAGE], 0, uri);
		g_free(uri);
	}
}

static void searchlist_clear(SearchList * self) {
	if (selfp->database) {
		selfp->database->close();
		delete selfp->database;
		selfp->database = NULL;
	}

	boost::filesystem::path book_index_dir_path(
			std::string(selfp->home_dir) + std::string("/index/")
					+ boost::filesystem::path(selfp->book_dir).filename().string());
	boost::filesystem::remove_all(book_index_dir_path);
}

//static gboolean searchlist_list_store_foreach_finalize(GtkTreeModel *model,
//		GtkTreePath *path, GtkTreeIter *iter, gpointer data) {
//	if (iter->user_data) {
//		delete ((std::string *) (iter->user_data));
//		iter->user_data = NULL;
//	}
//	return FALSE;
//}

static void searchlist_clear_list_store(SearchList * self) {
//	gtk_tree_model_foreach(GTK_TREE_MODEL(selfp->store),
//			searchlist_list_store_foreach_finalize, self);
	gtk_list_store_clear(selfp->store);
}

static void searchlist_index(SearchList * self) {
	searchlist_clear(self);

	boost::filesystem::path book_index_dir_path(
			std::string(selfp->home_dir) + std::string("/index/")
					+ boost::filesystem::path(selfp->book_dir).filename().string());
	boost::filesystem::create_directories(book_index_dir_path);

	Xapian::WritableDatabase writeable_database(book_index_dir_path.string(),
			Xapian::DB_CREATE_OR_OPEN);
	Xapian::TermGenerator term_generator;
	term_generator.set_stemmer(Xapian::Stem("en"));

	htmlcxx::HTML::ParserDom parser_dom;

	boost::filesystem::path book_dir_path(std::string(selfp->book_dir));
	boost::filesystem::recursive_directory_iterator end_iter;
	for (boost::filesystem::recursive_directory_iterator iter(book_dir_path);
			iter != end_iter; ++iter) {
		const boost::filesystem::path& path = iter->path();
		std::string extension_name = boost::filesystem::extension(path);
		if (extension_name == ".htm" || extension_name == ".html") {
			std::ifstream html_input_file_stream(path.string().c_str());
			std::string html(
					(std::istreambuf_iterator<char>(html_input_file_stream)),
					std::istreambuf_iterator<char>());
			tree<htmlcxx::HTML::Node> node_tree = parser_dom.parseTree(html);
			tree<htmlcxx::HTML::Node>::iterator node_tree_iter =
					node_tree.begin();
			tree<htmlcxx::HTML::Node>::iterator node_tree_end = node_tree.end();
			std::ostringstream text_output_string_stream;
			std::ostringstream title_output_string_stream;
			bool hasTitle = false;
			for (; node_tree_iter != node_tree_end; ++node_tree_iter) {
				if ((!node_tree_iter->isTag())
						&& (!node_tree_iter->isComment())) {
					text_output_string_stream << node_tree_iter->text();
				}
				if (!hasTitle && node_tree_iter->tagName() == "title") {
					for (tree<htmlcxx::HTML::Node>::sibling_iterator title_child_node_iter =
							node_tree_iter.begin();
							title_child_node_iter != node_tree_iter.end();
							++title_child_node_iter) {
						if ((!title_child_node_iter->isTag())
								&& (!title_child_node_iter->isComment())) {
							title_output_string_stream
									<< title_child_node_iter->text();
						}
					}

					hasTitle = true;
				}
			}
			std::string text = text_output_string_stream.str();
			std::string title = title_output_string_stream.str();

			std::string relative_path_string = path.string().substr(
					book_dir_path.string().length() + 1);

			Xapian::Document document;
			term_generator.set_document(document);

			term_generator.index_text(text);

			boost::property_tree::ptree json_tree;
			json_tree.put<std::string>("path", relative_path_string);
			json_tree.put<std::string>("title", title);
			std::ostringstream json_output_string_stream;
			boost::property_tree::json_parser::write_json(
					json_output_string_stream, json_tree);
			document.set_data(json_output_string_stream.str());

			document.add_boolean_term(relative_path_string);

			writeable_database.replace_document(relative_path_string, document);
		}
	}

	writeable_database.close();
}

static void searchlist_search(SearchList * self) {
	searchlist_clear_list_store(self);

	if (!selfp->database) {
		boost::filesystem::path book_index_dir_path(
				std::string(selfp->home_dir) + std::string("/index/")
						+ boost::filesystem::path(selfp->book_dir).filename().string());
		if (!boost::filesystem::is_directory(book_index_dir_path)) {
			searchlist_index(self);
		}
		selfp->database = new Xapian::Database(book_index_dir_path.string());
	}

	const gchar * text = gtk_entry_get_text(GTK_ENTRY(selfp->entry));
	guint16 text_len = gtk_entry_get_text_length(GTK_ENTRY(selfp->entry));
	std::string query_string(text, text_len);

	Xapian::QueryParser query_parser;
	query_parser.set_stemmer(Xapian::Stem("en"));
	query_parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
	Xapian::Query query = query_parser.parse_query(query_string);

	Xapian::Enquire enquire(*selfp->database);
	enquire.set_query(query);

	Xapian::MSet match_set = enquire.get_mset(0,
	selfp->database->get_doccount());
	std::cout<<match_set.size()<<std::endl;
	for (Xapian::MSetIterator match_set_iter = match_set.begin();
			match_set_iter != match_set.end(); ++match_set_iter) {
		Xapian::Document document = match_set_iter.get_document();
		std::string json_data = document.get_data();
		std::istringstream json_data_input_string_stream(json_data);
		boost::property_tree::ptree json_tree;
		boost::property_tree::json_parser::read_json(
				json_data_input_string_stream, json_tree);
		std::string relative_path_string = json_tree.get<std::string>("path");
		std::string title = json_tree.get<std::string>("title");

		GtkTreeIter tree_iter;
		tree_iter.user_data = (gpointer) new std::string(relative_path_string);
		gtk_list_store_append(GTK_LIST_STORE(selfp->store), &tree_iter);
		gchar * display_title = (gchar *) g_malloc0(title.length() + 1);
		std::strcpy(display_title, title.c_str());
		gtk_list_store_set(GTK_LIST_STORE(selfp->store), &tree_iter, 0,
				display_title, -1);
		g_free(display_title);
	}
}
