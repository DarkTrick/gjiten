/* -*- Mode: C; tab-width: 2;   indent-tabs-mode: space; c-basic-offset: 2 -*- */
/* vi: set ts=2 sw=2: */
/* dicfile.c

   GJITEN : A JAPANESE DICTIONARY FOR GNOME

   Copyright (C) 1999-2005 Botond Botyanszki <boti at rocketmail dot com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published  by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <unistd.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

#include "dicfile.h"
#include "error.h"
#include "utils.h"
#include <glib/gi18n.h>
#include <locale.h>



static gboolean
dicfile_is_utf8(GjitenDicfile *dicfile)
{
  gchar *testbuffer;
  gint pos, bytesread;

  if (dicfile->file > 0) {
    testbuffer = (gchar *) g_malloc (3000);
    bytesread = read (dicfile->file, testbuffer, 3000); // read a chunk into buffer
    pos = bytesread - 1;
    while (testbuffer[pos] != '\n') pos--;
    if (gx_utf8_validate (testbuffer, pos, NULL) == FALSE) {
      return FALSE;
    }
    g_free (testbuffer);
  }
  return TRUE;
}



gboolean
dicfile_check_all(GSList *dicfile_list)
{
  GSList *node;
  GjitenDicfile *dicfile;
  gboolean retval = TRUE;

  GJITEN_DEBUG ("dicfile_check_all()\n");

  node = dicfile_list;
  while (node != NULL) {
    if (node->data != NULL) {
      dicfile = node->data;
      const gchar * error = dicfile_init (dicfile);
      if (error != FALSE){
        error_show (NULL, error);
        retval = FALSE;
      }
      if (dicfile_is_utf8 (dicfile) == FALSE) {
        error_show (NULL,_("Dictionary file is non-UTF: %s\nPlease convert it to UTF-8. See the docs for more."), dicfile->path);
        dicfile_close (dicfile);
        retval = FALSE;
      }
      dicfile_close (dicfile);
    }
    node = g_slist_next (node);
  }
  GJITEN_DEBUG (" retval: %d\n", retval);
  return retval;
}



/**
 * Return:
 *  ok: NULL
 *  error: error string (must not be freed)
 **/
const gchar *
dicfile_init(GjitenDicfile *dicfile)
{

  if (dicfile->status != DICFILE_OK) {
    dicfile->file = open (dicfile->path, O_RDONLY);

    if (dicfile->file == -1) {
      dicfile->status = DICFILE_BAD;
      return _("Sorry, I could not load your dictionary.");
    }
    else {
      if (stat (dicfile->path, &dicfile->stat) != 0) {
        printf ("**ERROR** %s: stat() \n", dicfile->path);
        dicfile->status = DICFILE_BAD;
        return _("Sorry, I could not load your dictionary.");
      }
      else {
        dicfile->size = dicfile->stat.st_size;
      }
    }
    dicfile->status = DICFILE_OK;
  }
  return NULL;
}



void
dicfile_close(GjitenDicfile *dicfile)
{

  if (dicfile->file > 0) {
    close (dicfile->file);
  }
  dicfile->status = DICFILE_NOT_INITIALIZED;
}



/**
 * Returns:
 *    ok: NULL
 *    error: string describing the error
 *           (string must not be freed)
 **/
const gchar *
dicfile_is_valid(GjitenDicfile *self)
{
  const gchar * error = NULL;

  if (self->path == NULL)
    return _("Please select a dictionary file.");

  if (FALSE == g_file_test (self->path,G_FILE_TEST_EXISTS))
    return _("Dictionary file not found. ");

  error = dicfile_init (self);
  if (error != NULL)
    return error;

  if (dicfile_is_utf8 (self) == FALSE)
  {
    dicfile_close (self);
    return _("Dictionary file is not in UTF-8 format. \nPlease convert it to UTF-8 format. See the docs for more information.");
  }

  dicfile_close (self);

  return NULL;
}

GjitenDicfile *
dicfile_new(gchar *name,
            gchar *path)
{
  GjitenDicfile *self = g_new0 (GjitenDicfile, 1);
  self->name = name;
  self->path = path;

  return self;
}



void
dicfile_free(GjitenDicfile *self)
{
  if (NULL == self)
    return;

  dicfile_close (self);

  if (NULL != self->path) {
    g_free (self->path);
    self->path = NULL;
  }

  if (NULL != self->name) {
    g_free (self->name);
    self->name = NULL;
  }

  g_free (self);
  self = NULL;
}



void
dicfile_list_free(GSList *dicfile_list)
{
  g_slist_free_full (dicfile_list,
                    (GDestroyNotify)dicfile_free);
}