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
#include "hhc.h"

#include <libxml/parser.h>
#include <libxml/HTMLparser.h>

#include "models/link.h"
#include "utils/utils.h"

static gint depth = -1;
static gint prev_depth = -1;
static gboolean tree_item = FALSE;
static gchar *title = NULL;
static gchar *local = NULL;

static GNode *parent = NULL;
static GNode *prev_node = NULL;

static void startDocumentHH(void *);
static void endDocumentHH(void *);
static void startElementHH(void *, const xmlChar *, const xmlChar **);
static void endElementHH(void *, const xmlChar *);

static xmlSAXHandler hhSAXHandlerStruct = {
  NULL, /* internalSubset */
  NULL, /* isStandalone */
  NULL, /* hasInternalSubset */
  NULL, /* hasExternalSubset */
  NULL, /* resolveEntity */
  NULL, /* getEntity */
  NULL, /* entityDecl */
  NULL, /* notationDecl */
  NULL, /* attributeDecl */
  NULL, /* elementDecl */
  NULL, /* unparsedEntityDecl */
  NULL, /* setDocumentLocator */
  startDocumentHH, /* startDocument */
  endDocumentHH, /* endDocument */
  startElementHH, /* startElement */
  endElementHH, /* endElement */
  NULL, /* reference */
  NULL, /* characters */
  NULL, /* ignorableWhitespace */
  NULL, /* processingInstruction */
  NULL, /* comment */
  NULL, /* xmlParserWarning */
  NULL, /* xmlParserError */
  NULL, /* xmlParserError */
  NULL, /* getParameterEntity */
  NULL, /* cdataBlock */
  NULL, /* externalSubset */
  1,    /* initialized */
  NULL, /* private */
  NULL, /* startElementNsSAX2Func */
  NULL, /* endElementNsSAX2Func */
  NULL  /* xmlStructuredErrorFunc */
};

static xmlSAXHandlerPtr hhSAXHandler = &hhSAXHandlerStruct;

static void
startDocumentHH(void *ctx)
{
  g_message("SAX.startDocument()");
}

static void
endDocumentHH(void *ctx)
{
  g_message("SAX.endDocument()");
}

static void
startElementHH(void *ctx, const xmlChar *name, const xmlChar **atts)
{
  g_debug("SAX.startElement(%s)", (char *)name);
        
  if (g_ascii_strcasecmp("ul", (char *)name) == 0) {
    depth++;
  } else if (g_ascii_strcasecmp("object", (char *)name) == 0) {
    gchar *type;

    type = g_strdup((gchar *)atts[1]);

    g_debug("type = %s", type);

    if (g_ascii_strcasecmp("text/sitemap", type) == 0)
      tree_item = TRUE;

    g_free(type);
  } else if (g_ascii_strcasecmp("param", (char *)name) == 0) {
    gchar *param_name;
    gchar *param_value;

    param_name = g_strdup((gchar *)atts[1]);
    param_value = g_strdup((gchar *)atts[3]);

    if (tree_item) {
      if (g_ascii_strcasecmp("Name", param_name) == 0)
        title = g_strdup(param_value);
      else if (g_ascii_strcasecmp("Local", param_name) == 0)
        local = g_strdup(param_value);
    }
         
    g_free(param_name);
    g_free(param_value);
  }
}

static void
endElementHH(void *ctx, const xmlChar *name)
{
  GNode *link_tree = (GNode *)ctx;
  GNode *node;
  Link *link;

  g_debug("SAX.endElement(%s)", (char *) name);

  if (g_ascii_strcasecmp("ul", (char *)name) == 0) {
    depth--;
  } else if (g_ascii_strcasecmp("object", (char *)name) == 0) {
    if (!tree_item) 
      return;

    if (local == NULL) {
      local = g_strdup(CHMSEE_NO_LINK);
    }

    g_debug("prev_depth = %d", prev_depth);
    g_debug("depth = %d", depth);

    g_debug("title = %s", title);
    g_debug("local = %s", local);

    link = link_new(LINK_TYPE_PAGE, 
                    title ? title : "default title",
                    local ? local : "default local");
    node = g_node_new(link);
                        
    if (depth == 0) {
      parent = link_tree;
    } else {
      if (depth > prev_depth)
        parent = prev_node;
      else
        for (; depth < prev_depth; prev_depth--)
          parent = parent->parent;
    }
                
    g_node_append(parent, node);
    prev_node = node;

    prev_depth = depth;
    tree_item = FALSE;

    g_free(title);
    g_free(local);

    title = local = NULL;
  }
}

Hhc *
hhc_load(const gchar *filename, const gchar *encoding)
{
  htmlDocPtr doc = NULL;
  GNode *link_tree;

  link_tree = g_node_new(NULL);

  g_debug("parse encoding = %s", encoding);
  g_debug("filename = %s", filename);

  doc = htmlSAXParseFile(filename, 
                         encoding, 
                         hhSAXHandler, 
                         link_tree);

  if (doc != NULL) {
    g_message("htmlSAXParseFile returned non-NULL");
    xmlFreeDoc(doc);
  }

  g_message("Parsing hhc file finish.");

  return link_tree;
}

void
hhc_free(Hhc* self) {
  g_node_destroy(self);
}
