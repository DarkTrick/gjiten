/* -*- Mode: C; tab-width: 2;   indent-tabs-mode: space; c-basic-offset: 2 -*- */
/* vi: set ts=2 sw=2: */
/* conf.h

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

#ifndef __CONF_H__
#define __CONF_H__


#include <pango/pango-font.h>

#include "kanjidicconsts.h"
#include "constants.h"
#include "dicfile.h"
#include "data_store.h"

typedef struct _GjitenConfig GjitenConfig;

// forward decls

#define HISTORY_MAX_WORDS 50

struct _GjitenConfig {
  gchar *version;

  GjitenDicfile *kanjidic;
  GSList *dicfile_list;

  DataStore * data_store;

  char *history[60];
  gboolean toolbar;
  gboolean menubar;
  gboolean kdiccfg[KCFGNUM];
  gboolean bigwords;
  gboolean bigkanji;
  gboolean override_env;
  gchar *kanjipad;

  gboolean cli_option_startkanjidic;
  gchar *cli_option_kanji_to_lookup;
  gchar *cli_option_word_to_lookup;
  gboolean cli_option_clip_kanji_lookup;
  gboolean cli_option_clip_word_lookup;
  gboolean cli_option_quick_lookup_mode;
  gboolean cli_option_show_version;

  int maxwordmatches;
  gchar *largefont;
  gchar *normalfont;
  gboolean gdk_use_xft;
  gboolean force_ja_JP;
  gboolean force_language_c;
  gboolean envvar_override;
  gboolean search_kata_on_hira;
  gboolean search_hira_on_kata;
  gboolean verb_deinflection;

  gboolean searchlimit_enabled;
  gboolean worddic_options_show;
  gboolean autoadjust_enabled;

  gboolean unicode_radicals;

  GjitenDicfile *selected_dic;
  GjitenDicfile *mmaped_dicfile;
  PangoFontDescription *normalfont_desc;

  /* DEPRECATED */
  char *dictpath;
  int numofdics;

};


GjitenConfig *gjitenconfig_new_and_init();
void gjitenconfig_free(        GjitenConfig *self);

void gjitenconfig_save(        GjitenConfig *conf);
void gjitenconfig_save_history(GtkListStore *history,
                               GjitenConfig *conf);
void gjitenconfig_save_options(GjitenConfig *conf);


#endif
