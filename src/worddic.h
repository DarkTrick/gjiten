/* -*- Mode: C; tab-width: 2;   indent-tabs-mode: space; c-basic-offset: 2 -*- */
/* vi: set ts=2 sw=2: */
/* gjiten.c

   GJITEN : A GTK+/GNOME BASED JAPANESE DICTIONARY

   Copyright (C) 1999 - 2005 Botond Botyanszki <boti@rocketmail.com>
                 2021 DarkTrick - 69f925915ed0193a3b841aeec09451df2326f104

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

#ifndef __GJ_WORDDIC_WINDOW_H__
#define __GJ_WORDDIC_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TYPE_GJ_WORDDIC_WINDOW gj_worddic_window_get_type ()




G_DECLARE_DERIVABLE_TYPE (GjWorddicWindow, gj_worddic_window, GJ, WORDDIC_WINDOW, GtkApplicationWindow)
struct _GjWorddicWindowClass
{
  GtkApplicationWindowClass parent_class;
};



/* verb deinflection */
struct vinfl_struct {
  gchar *conj;
  gchar *infl;
  gchar *type;
};



GjWorddicWindow* worddic_create();
void             worddic_close();
GtkWidget*       gj_worddic_window_new(GtkApplication * app);
void worddic_paste();
void on_search_clicked();
void worddic_update_dic_menu();
void worddic_apply_fonts();
void enable_quick_lookup_mode();

void worddic_lookup_word(gchar * word_to_lookup);

G_END_DECLS

#endif /* include-protector */