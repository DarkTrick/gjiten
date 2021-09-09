/* -*- Mode: C; tab-width: 2;   indent-tabs-mode: space; c-basic-offset: 2 -*- */
/* vi: set ts=2 sw=2: */
/* error.c

   GJITEN : A GTK+/GNOME BASED JAPANESE DICTIONARY

   Copyright (C) 1999 - 2005 Botond Botyanszki <boti@rocketmail.com>

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


#include <stdarg.h>
#include <gtk/gtk.h>
#include <stdlib.h>

#include "error.h"
#include "constants.h"

static gchar *gjiten_errors;


/**
 *  Show non-modal error message
 **/
static int
_show_error(GtkWindow  *parent_nullable,
            const char *format, va_list args)
{
  GtkWidget *dialog;
  gint ret = -1;
  gchar *pstr;

  pstr = g_strdup_vprintf (format, args);

  if (pstr != NULL)
  {
    GtkDialogFlags destroy_style = GTK_DIALOG_DESTROY_WITH_PARENT;
    if (NULL == parent_nullable)
      destroy_style = GTK_DIALOG_MODAL;

    dialog = gtk_message_dialog_new (parent_nullable,
                                     destroy_style,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_OK,
                                     "%s", pstr );

    // Keep the application alive as long as the message is shown
    gtk_window_set_application (GTK_WINDOW (dialog),
                       GTK_APPLICATION (g_application_get_default ()));

    gtk_window_set_title (GTK_WINDOW (dialog), APPLICATION_NAME);

    g_signal_connect_swapped (G_OBJECT (dialog), "response",
                             G_CALLBACK (gtk_widget_destroy),
                             G_OBJECT (dialog));

    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    gtk_widget_show_all (dialog);

    g_free (pstr);
  }
  return ret;
}



/**
 *  Show non-modal error message,
 *  window-bound (parent window given)
 **/
int
error_show(GtkWindow  *parent,
                  const char *format, ... )
{

  va_list args;
  int ret = -1;

  va_start (args, format);
  ret = _show_error (parent, format, args);
  va_end (args);

  return ret;
}




void
error_show_and_quit(const char *fmt, ... )
{
  va_list args;
  gchar *pstr;

  va_start (args, fmt);
  pstr = g_strdup_vprintf (fmt, args);
  va_end (args);

  error_show (NULL, pstr);
  exit (1);
}
