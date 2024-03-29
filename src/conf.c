/* -*- Mode: C; tab-width: 2;   indent-tabs-mode: space; c-basic-offset: 2 -*- */
/* vi: set ts=2 sw=2: */
/* conf.c

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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <gtk/gtk.h>

#include "conf.h"
#include "constants.h"
#include "config.h"
#include "error.h"
#include "dicutil.h"
#include <glib/gi18n.h>
#include <locale.h>
#include "utils.h"
#include "data_store.h"

// moved from gjiten.c
gchar *kanjidicstrg[] = { "kanji",  "radical", "strokes", "reading", "korean",
                          "pinyin", "english", "bushu",   "classic", "freq",    "jouyou",
                          "deroo",  "skip",    "fourc",   "hindex",  "nindex",  "vindex",
                          "iindex", "mnindex", "mpindex", "eindex",  "kindex",
                          "lindex", "oindex",  "cref",    "missc",   "unicode", "jisascii" };

GjitenConfig conf;







GjitenConfig *
gjitenconfig_new()
{
  GjitenConfig * self = g_new0 (GjitenConfig, 1);

  self->data_store = data_store_new ();
  return self;
}



void
gjitenconfig_free(GjitenConfig * self)
{
  g_free (self->kanjipad);
  g_free (self->cli_option_kanji_to_lookup);
  g_free (self->cli_option_word_to_lookup);
  g_free (self->largefont);
  g_free (self->normalfont);
  dicfile_list_free (self->dicfile_list);

  // can't use `g_strfreev` because `history` itself is on stack
  for (int i = 0; i <= HISTORY_MAX_WORDS; i++) {
    if (self->history[i] == NULL) break;
    g_free (self->history[i]);
  }

  data_store_free (self->data_store);
}


/**
 * Extract dictionary path and name of a dictionary
 * from `str`, which is the format stored inside
 * the config file
 *
 * Out-parameters must be freed
 **/
gboolean
_dictionary_string_extract_path_and_name(gchar *str,
                                         gchar ** o_path,
                                         gchar ** o_name)
{

  gchar ** arr = g_strsplit (str, "\n", 2);

  if (NULL == arr[0]  || NULL == arr[1])
    return FALSE;

  *o_path = arr[0];
  *o_name = arr[1];

  g_free (arr); // only cleanup array, not it's content!
  return TRUE;
}



GjitenConfig *
gjitenconfig_new_and_init()
{
  gchar *dicprefix = DICTPREFIX;
  gchar *tmpstrg;
  gchar *tmpptr, *endptr;
  int i;
  GjitenDicfile *dicfile;
  GjitenConfig *conf;

  conf = gjitenconfig_new ();

  DataStore * store = conf->data_store;
  data_store_load_from_disk (store);

  #define store_get_boolean(KEY) data_store_get_boolean (store, SECTION_GENERAL, KEY)
  #define store_get_int(KEY)     data_store_get_int     (store, SECTION_GENERAL, KEY)
  #define store_get_string(KEY)  data_store_get_string  (store, SECTION_GENERAL, KEY)


  conf->version = store_get_string ("version");
  conf->worddic_options_show = store_get_boolean ("worddic_options_show");
  conf->autoadjust_enabled = store_get_boolean ("autoadjust_enabled");

  conf->bigwords = store_get_boolean ("bigwords");
  conf->bigkanji = store_get_boolean ("bigkanji");
  conf->largefont = store_get_string ("largefont");
  conf->normalfont = store_get_string ("normalfont");
  conf->gdk_use_xft = store_get_boolean ("gdk_use_xft");
  conf->force_ja_JP = store_get_boolean ("force_ja_JP");
  conf->force_language_c = store_get_boolean ("force_language_c");
  conf->envvar_override = store_get_boolean ("envvar_override");

  conf->searchlimit_enabled = store_get_boolean ("searchlimit_enabled");
  conf->maxwordmatches = store_get_int ("maxwordmatches");
  if (conf->maxwordmatches < 1) {
    conf->searchlimit_enabled = FALSE;
    conf->maxwordmatches = 100;
  }

  conf->dictpath = store_get_string ("dictpath");
  conf->menubar = store_get_boolean ("menubar");
  conf->toolbar = store_get_boolean ("toolbar");

  conf->search_kata_on_hira = store_get_boolean ("search_kata_on_hira");
  conf->search_hira_on_kata = store_get_boolean ("search_hira_on_kata");
  conf->verb_deinflection = store_get_boolean ("deinflection_enabled");

  if (conf->kanjidic == NULL) conf->kanjidic = g_new0 (GjitenDicfile, 1);
  conf->kanjidic->path = data_store_get_string (store, SECTION_KANJIDIC, "kanjidicfile");
  if ( (conf->kanjidic->path == NULL) || (strlen (conf->kanjidic->path)) == 0) {
    conf->kanjidic->path = GJITEN_DICDIR"/kanjidic";
  }
  conf->unicode_radicals = data_store_get_boolean (store, SECTION_KANJIDIC, "unicode_radicals");


  conf->kanjipad = store_get_string ("kanjipad");
  if (conf->kanjipad == NULL) conf->kanjipad = g_strdup ("");

  conf->numofdics = store_get_int ("numofdics");

  if (conf->dicfile_list != NULL) {
    dicutil_unload_dic ();
    dicfile_list_free (conf->dicfile_list);
    conf->dicfile_list = NULL;
  }

  if (conf->numofdics != 0) {
    //Load dicfiles from old style config [compatibility with older versions]
    for (i = 0; i < conf->numofdics; i++) {
      //if (i == MAXDICFILES - 1) break;
      tmpstrg = g_strdup_printf ("%s%d", dicprefix, i);
      gchar * dicname = NULL;
      gchar * dicpath = NULL;
      dicname = store_get_string (tmpstrg);
      if (conf->dictpath[strlen (conf->dictpath - 1)] == '/') {
        dicpath = g_strdup_printf ("%s%s", conf->dictpath, dicname);
      }
      else {
        dicpath = g_strdup_printf ("%s/%s", conf->dictpath, dicname);
      }
      dicfile = dicfile_new (dicname, dicpath);
      conf->dicfile_list = g_slist_append (conf->dicfile_list, dicfile);
      g_free (tmpstrg);
    }
  }
  else { //new config
    gchar **gconf_diclist = NULL;
    gchar **diclist = NULL;

    gconf_diclist = data_store_get_string_array (store, SECTION_GENERAL, "dictionary_list", NULL);
    diclist = gconf_diclist;
    if (diclist != NULL)
    {
      for (gint i = 0; diclist[i] != NULL; ++i)
      {
        gchar * dicpath = NULL;
        gchar * dicname = NULL;
        if (_dictionary_string_extract_path_and_name (diclist[i], &dicpath, &dicname))
        {
          GjitenDicfile *dicfile= dicfile_new (dicname, dicpath);
          conf->dicfile_list = g_slist_append (conf->dicfile_list, dicfile);
        }
      }
    }
    g_strfreev (gconf_diclist);
  }
  if (conf->dicfile_list != NULL) conf->selected_dic = conf->dicfile_list->data;

  //Load kanji info settings
  for (i = 0; i < KCFGNUM; i++) {
    if (data_store_get_boolean (store, SECTION_KANJIDIC, kanjidicstrg[i])) {
      conf->kdiccfg[i] = TRUE;
      // printf ("%s : %d\n",kanjidicstrg[i], conf->kdiccfg[i]);
    }
    else conf->kdiccfg[i] = FALSE;
  }

  //Load gjiten search history
  {
    // Be sure: init all to NULL
    for (i = 0; i <= HISTORY_MAX_WORDS; ++i) {
      conf->history[i] = NULL;
    }

    // get persistent values and save them
    gsize num_entries = 0;
    gchar ** history_array = data_store_get_string_array (store, SECTION_GENERAL, "word_search_history", &num_entries);
    for (i = 0; i < MIN (num_entries, HISTORY_MAX_WORDS); ++i) {
      conf->history[i] = history_array[i];
    }

    // We can only hold HISTORY_MAX_WORDS history entries.
    // Free values, if there were more in the list from
    // persistent storage
    for (i = HISTORY_MAX_WORDS; i < num_entries; ++i){
      g_free (history_array[i]);
    }

    g_free (history_array);
  }

  return conf;

  #undef store_get_boolean
  #undef store_get_int
  #undef store_get_string
}



void
gjitenconfig_save(GjitenConfig *conf)
{
  int i;
  gchar *confpath, *tmpstrg;
  GSList *gconf_diclist = NULL;
  GSList *diclist;
  GjitenDicfile *dicfile;

  DataStore * store = conf->data_store;

  #define store_set_boolean(KEY, VALUE) data_store_set_boolean (store, SECTION_GENERAL, KEY, VALUE)
  #define store_set_int(KEY, VALUE)     data_store_set_int     (store, SECTION_GENERAL, KEY, VALUE)
  #define store_set_string(KEY, VALUE)  data_store_set_string  (store, SECTION_GENERAL, KEY, VALUE)

  store_set_string ("version", VERSION);
  //Save kanjidic display options
  for (i = 0; i < KCFGNUM; i++) {
    confpath = g_strdup_printf ("%s%s", SECTION_KANJIDIC, kanjidicstrg[i]);
    data_store_set_boolean (store, SECTION_KANJIDIC, confpath, conf->kdiccfg[i]);
    g_free (confpath);
  }


  store_set_boolean ("menubar", conf->menubar);
  store_set_boolean ("toolbar", conf->toolbar);
  store_set_string ("dictpath", conf->dictpath == NULL ? g_strdup ("") : conf->dictpath);
  data_store_set_string (store, SECTION_KANJIDIC, "kanjidicfile", conf->kanjidic->path);
  data_store_set_boolean (store, SECTION_KANJIDIC,   "unicode_radicals", conf->unicode_radicals);

  if (conf->kanjipad == NULL) conf->kanjipad = g_strdup ("");
  store_set_string ("kanjipad", conf->kanjipad);

  //Deprecated dictionary file number, zero it out.
  //store_set_int ("numofdics", conf->numofdics);
  store_set_int ("numofdics", 0);

  store_set_boolean ("bigwords", conf->bigwords);
  store_set_boolean ("bigkanji", conf->bigkanji);
  store_set_string ("largefont", conf->largefont == NULL ? g_strdup ("") : conf->largefont);
  store_set_string ("normalfont", conf->normalfont == NULL ? g_strdup ("") : conf->normalfont);
  store_set_boolean ("gdk_use_xft", conf->gdk_use_xft);
  store_set_boolean ("force_ja_JP", conf->force_ja_JP);
  store_set_boolean ("force_language_c", conf->force_language_c);
  store_set_boolean ("envvar_override", conf->envvar_override);

  store_set_boolean ("search_kata_on_hira", conf->search_kata_on_hira);
  store_set_boolean ("search_hira_on_kata", conf->search_hira_on_kata);
  store_set_boolean ("deinflection_enabled", conf->verb_deinflection);

  //Save dicfiles [path and name seperated with linebreak]
  {
    GSList * list = conf->dicfile_list;

    gint size = g_slist_length (list) + 1;
    gchar ** array = g_malloc (sizeof (gchar*) * size);


    int i = 0;
    while (list){
      if (NULL != list->data)
      {
        dicfile = list->data;
        tmpstrg = g_strdup_printf ("%s\n%s", dicfile->path, dicfile->name);
        array[i] = tmpstrg;
        ++i;
      }
      list = g_slist_next (list);
    }
    array[i] = NULL;

    data_store_set_string_array (conf->data_store, SECTION_GENERAL,
                                 "dictionary_list",
                                 (const gchar **)(array), size);
    g_strfreev (array);
  }


  data_store_save_to_disk (store);

  #undef store_set_boolean
  #undef store_set_int
  #undef store_set_string
}



void
gjitenconfig_save_history(GtkListStore *history,
                  GjitenConfig *conf)
{
  int i;

  gint num_entries = gtk_list_store_length (history);

  if (history != NULL) {
    gchar ** history_array = g_malloc (sizeof (gchar*) * (num_entries+1));

    // TreeModel-to-StringArray
    {
      GtkTreeIter iter;
      gboolean iter_valid;

      iter_valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (history), &iter);
      for (i = 0; i <= HISTORY_MAX_WORDS; i++) {
        if (iter_valid == FALSE) break;
          history_array[i] = gtk_list_store_string_get (history, &iter);
          iter_valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (history), &iter);
      }
      // NULL-terminate array
      history_array[i] = NULL;
    }

    data_store_set_string_array (conf->data_store, SECTION_GENERAL,
                                "word_search_history",
                                (const gchar **)history_array, num_entries);
    g_strfreev (history_array);

    data_store_save_to_disk (conf->data_store);
  }
}



void
gjitenconfig_save_options(GjitenConfig *conf)
{
    DataStore *store = conf->data_store;
    data_store_set_boolean (store, SECTION_GENERAL, "worddic_options_show", conf->worddic_options_show);
    data_store_set_boolean (store, SECTION_GENERAL, "autoadjust_enabled", conf->autoadjust_enabled);
    data_store_set_boolean (store, SECTION_GENERAL, "searchlimit_enabled", conf->searchlimit_enabled);
    data_store_set_int (store, SECTION_GENERAL, "maxwordmatches", conf->maxwordmatches);

    data_store_save_to_disk (store);
}

