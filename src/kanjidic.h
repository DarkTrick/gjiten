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

struct _KanjiDic {
/* GTK variables */
  GtkWidget *window;
  GtkWidget *window_kanjinfo;
  GtkWidget *window_radicals;
  GtkWidget *combo_entry_key;
  GtkWidget *combo_entry_radical;
  GtkTextBuffer *text_kanjinfo_buffer;
  GtkWidget *text_kanjinfo_view;
  GtkTextIter kinfo_iter;
  GtkWidget *kanji_results_view;
  GtkTextBuffer *kanji_results_buffer;
  GtkTextIter kanji_results_iter;
  GtkWidget *appbar_kanji;
  GtkListStore * combo_entry_key_list;
  GtkListStore *combo_entry_radical_list;
  GtkWidget *spinb_strokenum;
  GtkWidget *spinb_plusmin;
  GtkWidget *label_plusmin;
  GtkWidget *checkb_ksearch;
  GtkWidget *checkb_radical;
  GtkWidget *checkb_stroke;
  GtkWidget *button_radtable;
  GtkWidget *button_clearrad;
  GtkWidget *button_cleark;
  GtkWidget *vbox_history;
  GtkWidget *scrolledwin_history;
  GSList *kanji_history_list;
  GtkTextTag *tag_large_font;
  GHashTable *rad_button_hash;
  GHashTable *kanji_info_hash;
  GHashTable *rad_info_hash;
  GList *rad_info_list;
};

KanjiDic *kanjidic_create();
void kanjidic_close();
void print_kanjinfo(gunichar kanji);
void kanjidic_apply_fonts();
void kanji_selected(gunichar kanji);

#endif
