/*
 *  Copyright (C) 2006 Ji YongGang <jungle@soforge-studio.com>
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
#include "chmfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(__linux__)
#include <fcntl.h>
#include <gcrypt.h>
#else
#include <md5.h>
#endif

#include <glib/gstdio.h>
#include <chm_lib.h>

#include "utils/utils.h"
#include "models/hhc.h"

#define UINT16ARRAY(x) ((unsigned char)(x)[0] | ((u_int16_t)(x)[1] << 8))
#define UINT32ARRAY(x) (UINT16ARRAY(x) | ((u_int32_t)(x)[2] << 16)      \
                        | ((u_int32_t)(x)[3] << 24))


struct extract_context
{
  const char *base_path;
  const char *hhc_file;
  const char *hhk_file;
};

static GObjectClass *parent_class = NULL;

static void chmfile_class_init(ChmFileClass *);
static void chmfile_init(ChmFile *);
static void chmfile_finalize(GObject *);
static void chmfile_file_info(ChmFile *);
static void chmfile_system_info(struct chmFile *, ChmFile *);
static void chmfile_windows_info(struct chmFile *, ChmFile *);

static int dir_exists(const char *);
static int rmkdir(char *);
static int _extract_callback(struct chmFile *, struct chmUnitInfo *, void *);
static gboolean extract_chm(const gchar *, ChmFile *);
static void load_fileinfo(ChmFile* self);
static void save_fileinfo(ChmFile* self);
static void extract_post_file_write(const gchar* fname);

GType
chmfile_get_type(void)
{
  static GType type = 0;

  if (!type) {
    static const GTypeInfo info = {
      sizeof(ChmFileClass),
      NULL,
      NULL,
      (GClassInitFunc)chmfile_class_init,
      NULL,
      NULL,
      sizeof(ChmFile),
      0,
      (GInstanceInitFunc)chmfile_init,
    };

    type = g_type_register_static(G_TYPE_OBJECT,
                                  "ChmFile",
                                  &info, 0);
  }

  return type;
}

static void
chmfile_class_init(ChmFileClass *klass)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = chmfile_finalize;
}

static void
chmfile_init(ChmFile *chmfile)
{
  chmfile->filename = NULL;
  chmfile->home = NULL;
  chmfile->hhc = NULL;
  chmfile->hhk = NULL;
  chmfile->title = NULL;
  chmfile->encoding = "UTF-8";
  chmfile->variable_font = g_strdup("Sans 12");
  chmfile->fixed_font = g_strdup("Monospace 12");
}

static void
chmfile_finalize(GObject *object)
{
  ChmFile *chmfile;

  chmfile = CHMFILE (object);

  g_message("chmfile finalize");

  save_fileinfo(chmfile);
  g_free(chmfile->filename);
  g_free(chmfile->hhc);
  g_free(chmfile->hhk);
  g_free(chmfile->home);
  g_free(chmfile->title);
  g_free(chmfile->variable_font);
  g_free(chmfile->fixed_font);

  if(chmfile->link_tree) {
    g_node_destroy(chmfile->link_tree);
  }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static int
dir_exists(const char *path)
{
  struct stat statbuf;

  return stat(path, &statbuf) != -1 ? 1 : 0;
}

static int
rmkdir(char *path)
{
  /*
   * strip off trailing components unless we can stat the directory, or we
   * have run out of components
   */

  char *i = rindex(path, '/');

  if (path[0] == '\0'  ||  dir_exists(path))
    return 0;

  if (i != NULL) {
    *i = '\0';
    rmkdir(path);
    *i = '/';
    mkdir(path, 0777);
  }

  return dir_exists(path) ? 0 : -1;
}

/*
 * callback function for enumerate API
 */
static int
_extract_callback(struct chmFile *h, struct chmUnitInfo *ui, void *context)
{
  gchar* fname = NULL;
  char buffer[32768];
  struct extract_context *ctx = (struct extract_context *)context;
  char *i;

  if (ui->path[0] != '/') {
    return CHM_ENUMERATOR_CONTINUE;
  }

  g_debug("ui->path = %s", ui->path);

  fname = g_build_filename(ctx->base_path, ui->path+1, NULL);

  if (ui->length != 0) {
    FILE *fout;
    LONGINT64 len, remain = ui->length;
    LONGUINT64 offset = 0;

    gchar *file_ext;

    file_ext = g_strrstr(g_path_get_basename(ui->path), ".");
    g_debug("file_ext = %s", file_ext);

    if ((fout = fopen(fname, "wb")) == NULL) {
      /* make sure that it isn't just a missing directory before we abort */
      char newbuf[32768];
      strcpy(newbuf, fname);
      i = rindex(newbuf, '/');
      *i = '\0';
      rmkdir(newbuf);

      if ((fout = fopen(fname, "wb")) == NULL) {
        g_message("CHM_ENUMERATOR_FAILURE fopen");
        return CHM_ENUMERATOR_FAILURE;
      }
    }

    while (remain != 0) {
      len = chm_retrieve_object(h, ui, (unsigned char *)buffer, offset, 32768);
      if (len > 0) {
        if(fwrite(buffer, 1, (size_t)len, fout) != len) {
          g_message("CHM_ENUMERATOR_FAILURE fwrite");
          return CHM_ENUMERATOR_FAILURE;
        }
        offset += len;
        remain -= len;
      } else {
        break;
      }
    }

    fclose(fout);
    extract_post_file_write(fname);
  } else {
    if (rmkdir(fname) == -1) {
      g_message("CHM_ENUMERATOR_FAILURE rmkdir");
      return CHM_ENUMERATOR_FAILURE;
    }
  }
  g_free(fname);
  return CHM_ENUMERATOR_CONTINUE;
}

static gboolean
extract_chm(const gchar *filename, ChmFile *chmfile)
{
  struct chmFile *handle;
  struct extract_context ec;

  handle = chm_open(filename);

  if (handle == NULL) {
    g_message(_("cannot open chmfile: %s"), filename);
    return FALSE;
  }

  ec.base_path = (const char *)chmfile->dir;

  if (!chm_enumerate(handle, CHM_ENUMERATE_NORMAL, _extract_callback, (void *)&ec)) {
    g_message(_("Extract chmfile failed: %s"), filename);
    return FALSE;
  }

  chm_close(handle);

  return TRUE;
}

#if defined(__linux__)
static char *
MD5File(const char *filename, char *buf)
{
  unsigned char buffer[1024];
  unsigned char* digest;
  static const char hex[]="0123456789abcdef";

  gcry_md_hd_t hd;
  int f, i;

  if(filename == NULL)
    return NULL;

  gcry_md_open(&hd, GCRY_MD_MD5, 0);
  f = open(filename, O_RDONLY);
  if (f < 0) {
    g_message(_("open \"%s\" failed: %s"), filename, strerror(errno));
    return NULL;
  }

  while ((i = read(f, buffer, sizeof(buffer))) > 0)
    gcry_md_write(hd, buffer, i);

  close(f);

  if (i < 0)
    return NULL;

  if (buf == NULL)
    buf = malloc(33);
  if (buf == NULL)
    return (NULL);

  digest = (unsigned char*)gcry_md_read(hd, 0);

  for (i = 0; i < 16; i++) {
    buf[i+i] = hex[(u_int32_t)digest[i] >> 4];
    buf[i+i+1] = hex[digest[i] & 0x0f];
  }

  buf[i+i] = '\0';
  return (buf);
}
#endif

static u_int32_t
get_dword(const unsigned char *buf)
{
  u_int32_t result;

  result = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);

  if (result == 0xFFFFFFFF)
    result = 0;

  return result;
}

static const char *
get_encoding(u_int32_t lcid)
{
  switch(lcid) {
  case 0x0436:
  case 0x042d:
  case 0x0403:
  case 0x0406:
  case 0x0413:
  case 0x0813:
  case 0x0409:
  case 0x0809:
  case 0x0c09:
  case 0x1009:
  case 0x1409:
  case 0x1809:
  case 0x1c09:
  case 0x2009:
  case 0x2409:
  case 0x2809:
  case 0x2c09:
  case 0x3009:
  case 0x3409:
  case 0x0438:
  case 0x040b:
  case 0x040c:
  case 0x080c:
  case 0x0c0c:
  case 0x100c:
  case 0x140c:
  case 0x180c:
  case 0x0407:
  case 0x0807:
  case 0x0c07:
  case 0x1007:
  case 0x1407:
  case 0x040f:
  case 0x0421:
  case 0x0410:
  case 0x0810:
  case 0x043e:
  case 0x0414:
  case 0x0814:
  case 0x0416:
  case 0x0816:
  case 0x040a:
  case 0x080a:
  case 0x0c0a:
  case 0x100a:
  case 0x140a:
  case 0x180a:
  case 0x1c0a:
  case 0x200a:
  case 0x240a:
  case 0x280a:
  case 0x2c0a:
  case 0x300a:
  case 0x340a:
  case 0x380a:
  case 0x3c0a:
  case 0x400a:
  case 0x440a:
  case 0x480a:
  case 0x4c0a:
  case 0x500a:
  case 0x0441:
  case 0x041d:
  case 0x081d:
    return "iso8859_1";
    break;
  case 0x041c:
  case 0x041a:
  case 0x0405:
  case 0x040e:
  case 0x0418:
  case 0x081a:
  case 0x041b:
  case 0x0424:
    return "iso8859_2";
    break;
  case 0x0401:
  case 0x0801:
  case 0x0c01:
  case 0x1001:
  case 0x1401:
  case 0x1801:
  case 0x1c01:
  case 0x2001:
  case 0x2401:
  case 0x2801:
  case 0x2c01:
  case 0x3001:
  case 0x3401:
  case 0x3801:
  case 0x3c01:
  case 0x4001:
  case 0x0429:
  case 0x0420:
    return "iso8859_6";
    break;
  case 0x0408:
    return "iso8859_7";
    break;
  case 0x040d:
    return "iso8859_8";
    break;
  case 0x042c:
  case 0x041f:
  case 0x0443:
    return "iso8859_9";
    break;
  case 0x041e:
    return "iso8859_11";
    break;
  case 0x0425:
  case 0x0426:
  case 0x0427:
    return "iso8859_13";
    break;
  case 0x0411:
    return "cp932";
    break;
  case 0x0804:
  case 0x1004:
    return "cp936";
    break;
  case 0x0412:
    return "cp949";
    break;
  case 0x0404:
  case 0x0c04:
  case 0x1404:
    return "cp950";
    break;
  case 0x082c:
  case 0x0423:
  case 0x0402:
  case 0x043f:
  case 0x042f:
  case 0x0419:
  case 0x0c1a:
  case 0x0444:
  case 0x0422:
  case 0x0843:
    return "cp1251";
    break;
  default:
    return "";
    break;
  }
}

static void
chmfile_file_info(ChmFile *chmfile)
{
  struct chmFile *cfd;

  cfd = chm_open(chmfile->filename);

  if (cfd == NULL) {
    g_error(_("Can not open chm file %s."), chmfile->filename);
    return;
  }

  chmfile_system_info(cfd, chmfile);
  chmfile_windows_info(cfd, chmfile);

  /* Convert book title to UTF-8 */
  if (chmfile->title != NULL && chmfile->encoding != NULL) {
    gchar *title_utf8;

    title_utf8 = g_convert(chmfile->title, -1, "UTF-8",
                           chmfile->encoding,
                           NULL, NULL, NULL);
    g_free(chmfile->title);
    chmfile->title = title_utf8;
  }

  /* Convert filename to UTF-8 */
  if (chmfile->hhc != NULL && chmfile->encoding != NULL) {
    gchar *filename_utf8;

    filename_utf8 = convert_filename_to_utf8(chmfile->hhc, chmfile->encoding);
    g_free(chmfile->hhc);
    chmfile->hhc = filename_utf8;
  }

  if (chmfile->hhk != NULL && chmfile->encoding != NULL) {
    gchar *filename_utf8;

    filename_utf8 = convert_filename_to_utf8(chmfile->hhk, chmfile->encoding);
    g_free(chmfile->hhk);
    chmfile->hhk = filename_utf8;
  }

  chm_close(cfd);
}

static void
chmfile_windows_info(struct chmFile *cfd, ChmFile *chmfile)
{
  struct chmUnitInfo ui;
  unsigned char buffer[4096];
  size_t size = 0;
  u_int32_t entries, entry_size;
  u_int32_t hhc, hhk, title, home;

  if (chm_resolve_object(cfd, "/#WINDOWS", &ui) != CHM_RESOLVE_SUCCESS)
    return;

  size = chm_retrieve_object(cfd, &ui, buffer, 0L, 8);

  if (size < 8)
    return;

  entries = get_dword(buffer);
  if (entries < 1)
    return;

  entry_size = get_dword(buffer + 4);
  size = chm_retrieve_object(cfd, &ui, buffer, 8L, entry_size);
  if (size < entry_size)
    return;

  hhc = get_dword(buffer + 0x60);
  hhk = get_dword(buffer + 0x64);
  home = get_dword(buffer + 0x68);
  title = get_dword(buffer + 0x14);

  if (chm_resolve_object(cfd, "/#STRINGS", &ui) != CHM_RESOLVE_SUCCESS)
    return;

  size = chm_retrieve_object(cfd, &ui, buffer, 0L, 4096);

  if (!size)
    return;

  if (chmfile->hhc == NULL && hhc)
    chmfile->hhc = g_strdup_printf("/%s", buffer + hhc);
  if (chmfile->hhk == NULL && hhk)
    chmfile->hhk = g_strdup_printf("/%s", buffer + hhk);
  if (chmfile->home == NULL && home)
    chmfile->home = g_strdup_printf("/%s", buffer + home);
  if (chmfile->title == NULL && title)
    chmfile->title = g_strdup((char *)buffer + title);
}

static void
chmfile_system_info(struct chmFile *cfd, ChmFile *chmfile)
{
  struct chmUnitInfo ui;
  unsigned char buffer[4096];

  int index = 0;
  unsigned char* cursor = NULL;
  u_int16_t value = 0;
  u_int32_t lcid = 0;
  size_t size = 0;

  if (chm_resolve_object(cfd, "/#SYSTEM", &ui) != CHM_RESOLVE_SUCCESS)
    return;

  size = chm_retrieve_object(cfd, &ui, buffer, 4L, 4096);

  if (!size)
    return;

  buffer[size - 1] = 0;

  for(;;) {
    // This condition won't hold if I process anything
    // except NUL-terminated strings!
    if(index > size - 1 - (long)sizeof(u_int16_t))
      break;

    cursor = buffer + index;
    value = UINT16ARRAY(cursor);
    g_debug("system value = %d", value);
    switch(value) {
    case 0:
      index += 2;
      cursor = buffer + index;

      chmfile->hhc = g_strdup_printf("/%s", buffer + index + 2);
      g_debug("hhc %s", chmfile->hhc);

      break;
    case 1:
      index += 2;
      cursor = buffer + index;

      chmfile->hhk = g_strdup_printf("/%s", buffer + index + 2);
      g_debug("hhk %s", chmfile->hhk);

      break;
    case 2:
      index += 2;
      cursor = buffer + index;

      chmfile->home = g_strdup_printf("/%s", buffer + index + 2);
      g_debug("home %s", chmfile->home);

      break;
    case 3:
      index += 2;
      cursor = buffer + index;

      chmfile->title = g_strdup((char *)buffer + index + 2);
      g_debug("title %s", chmfile->title);

      break;
    case 4: // LCID stuff
      index += 2;
      cursor = buffer + index;

      lcid = UINT32ARRAY(buffer + index + 2);
      g_debug("lcid %x", lcid);
      chmfile->encoding = get_encoding(lcid);
      break;

    case 6:
      index += 2;
      cursor = buffer + index;

      if(!chmfile->hhc) {
        char *hhc, *hhk;

        hhc = g_strdup_printf("/%s.hhc", buffer + index + 2);
        hhk = g_strdup_printf("/%s.hhk", buffer + index + 2);

        if (chm_resolve_object(cfd, hhc, &ui) == CHM_RESOLVE_SUCCESS)
          chmfile->hhc = hhc;

        if (chm_resolve_object(cfd, hhk, &ui) == CHM_RESOLVE_SUCCESS)
          chmfile->hhk = hhk;
      }

      break;
    case 16:
      index += 2;
      cursor = buffer + index;

      g_debug("font %s", buffer + index + 2);
      break;

    default:
      index += 2;
      cursor = buffer + index;
    }

    value = UINT16ARRAY(cursor);
    index += value + 2;
  }
}

ChmFile *
chmfile_new(const gchar *filename)
{
  ChmFile *chmfile;
  gchar *bookmark_file;
  gchar *md5;

  /* Use chmfile MD5 as book folder name */
  md5 = MD5File(filename, NULL);
  if(!md5) {
    return NULL;
  }

  chmfile = g_object_new(TYPE_CHMFILE, NULL);


  chmfile->dir = g_build_filename(g_getenv("HOME"),
                                  ".chmsee",
                                  "bookshelf",
                                  md5,
                                  NULL);
  g_debug("book dir = %s", chmfile->dir);

  /* If this chm file extracted before, load it's bookinfo */
  if (!g_file_test(chmfile->dir, G_FILE_TEST_IS_DIR)) {
    if (!extract_chm(filename, chmfile)) {
      g_debug("extract_chm failed: %s", filename);
      return NULL;
    }

    chmfile->filename = g_strdup(filename);
    g_debug("chmfile->filename = %s", chmfile->filename);

    chmfile_file_info(chmfile);
    save_fileinfo(chmfile);
  } else {
    load_fileinfo(chmfile);
  }

  g_debug("chmfile->hhc = %s", chmfile->hhc);
  g_debug("chmfile->hhk = %s", chmfile->hhk);
  g_debug("chmfile->home = %s", chmfile->home);
  g_debug("chmfile->title = %s", chmfile->title);
  g_debug("chmfile->endcoding = %s", chmfile->encoding);

  /* Parse hhc and store result to tree view */
  if (chmfile->hhc != NULL && g_ascii_strcasecmp(chmfile->hhc, "(null)") != 0) {
    gchar *hhc;

    hhc = g_strdup_printf("%s%s", chmfile->dir, chmfile->hhc);

    if (g_file_test(hhc, G_FILE_TEST_EXISTS)) {
      chmfile->link_tree = hhc_load(hhc, chmfile->encoding);
    } else {
      gchar *hhc_ncase;

      hhc_ncase = file_exist_ncase(hhc);
      chmfile->link_tree = hhc_load(hhc_ncase, chmfile->encoding);
      g_free(hhc_ncase);
    }

    g_free(hhc);
  } else {
    g_message(_("Can't found hhc file."));
  }

  /* Load bookmarks */
  bookmark_file = g_build_filename(chmfile->dir, CHMSEE_BOOKMARK_FILE, NULL);
  chmfile->bookmarks_list = bookmarks_load(bookmark_file);
  g_free(bookmark_file);

  return chmfile;
}
void
load_fileinfo(ChmFile *book)
{
  GList *pairs, *list;
  gchar *path;

  path = g_strdup_printf("%s/%s", book->dir, CHMSEE_BOOKINFO_FILE);

  g_debug("bookinfo path = %s", path);

  pairs = parse_config_file("bookinfo", path);

  for (list = pairs; list; list = list->next) {
    Item *item;

    item = list->data;

    if (strstr(item->id, "hhc")) {
      book->hhc = g_strdup(item->value);
      continue;
    }

    if (strstr(item->id, "hhk")) {
      book->hhk = g_strdup(item->value);
      continue;
    }

    if (strstr(item->id, "home")) {
      book->home = g_strdup(item->value);
      continue;
    }

    if (strstr(item->id, "title")) {
      book->title = g_strdup(item->value);
      continue;
    }

    if (strstr(item->id, "encoding")) {
      book->encoding = g_strdup(item->value);
      continue;
    }

    if (strstr(item->id, "variable_font")) {
      book->variable_font = g_strdup(item->value);
      continue;
    }

    if (strstr(item->id, "fixed_font")) {
      book->fixed_font = g_strdup(item->value);
      continue;
    }
  }

  free_config_list(pairs);
}

void
save_fileinfo(ChmFile *book)
{
  FILE *fd;
  gchar *path;

  path = g_build_filename(book->dir, CHMSEE_BOOKINFO_FILE, NULL);

  g_debug("save bookinfo path = %s", path);

  fd = fopen(path, "w");

  if (!fd) {
    g_print("Faild to open bookinfo file: %s", path);
  } else {
    save_option(fd, "hhc", book->hhc);
    save_option(fd, "hhk", book->hhk);
    save_option(fd, "home", book->home);
    save_option(fd, "title", book->title);
    save_option(fd, "encoding", book->encoding);
    save_option(fd, "variable_font", book->variable_font);
    save_option(fd, "fixed_font", book->fixed_font);

    fclose(fd);
  }
  g_free(path);
}

/* see http://code.google.com/p/chmsee/issues/detail?id=12 */
void extract_post_file_write(const gchar* fname) {
  gchar* basename = g_path_get_basename(fname);
  gchar* pos = strchr(basename, ';');
  if(pos) {
    gchar* dirname = g_path_get_dirname(fname);
    *pos = '\0';
    gchar* newfname = g_build_filename(dirname, basename, NULL);
    if(g_rename(fname, newfname) != 0) {
      g_error("rename \"%s\" to \"%s\" failed: %s", fname, newfname, strerror(errno));
    }
    g_free(dirname);
    g_free(newfname);
  }
  g_free(basename);
}
