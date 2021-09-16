/* -*- Mode: C; tab-width: 2;   indent-tabs-mode: space; c-basic-offset: 2 -*- */
/* kanjidic.h

   GJITEN : A GTK+/GNOME BASED JAPANESE DICTIONARY

   Copyright (C) 1999 - 2003 Botond Botyanszki <boti@rocketmail.com>

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
#ifndef __KANJIDIC_H__
#define __KANJIDIC_H__

#include <gtk/gtk.h>

#include "kanjidicconsts.h"

#define KBUFSIZE 500


typedef struct _KanjiDic KanjiDic;

typedef struct _RadInfo {
  gunichar radical;
  gint strokes;
  GList *kanji_info_list;
} RadInfo;

typedef struct _KanjiInfo {
  gunichar kanji;
  GList *rad_info_list;
} KanjiInfo;



G_BEGIN_DECLS

#define TYPE_GJ_KANJIDIC_WINDOW gj_kanjidic_window_get_type ()



G_DECLARE_DERIVABLE_TYPE (GjKanjidicWindow, gj_kanjidic_window, GJ, KANJIDIC_WINDOW, GtkApplicationWindow)
struct _GjKanjidicWindowClass
{
  GtkApplicationWindowClass parent_class;
};

GjKanjidicWindow *kanjidic_create();
void              kanjidic_close();
GtkWidget*        gj_kanjidic_window_new (GtkApplication * app);
gint              kanjidic_lookup(const gchar* kanji);
void              kanjidic_apply_fonts();
void              kanji_selected(gunichar kanji);
int               kanjidic_radical_selected(gunichar radical);

G_END_DECLS


#endif
