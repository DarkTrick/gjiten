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
#include "utils.h" // for _() // TODO: remove, after _() works properly


// moved from gjiten.c
gchar *kanjidicstrg[] = { "kanji",  "radical", "strokes", "reading", "korean",
                          "pinyin", "english", "bushu",   "classic", "freq",    "jouyou",
                          "deroo",  "skip",    "fourc",   "hindex",  "nindex",  "vindex",
                          "iindex", "mnindex", "mpindex", "eindex",  "kindex",
                          "lindex", "oindex",  "cref",    "missc",   "unicode", "jisascii" };

GjitenConfig conf;


#define STORE_ROOT       "net.sf.gjiten."
//#define STORE_ROOT_PATH  "/net/sf/gjiten/"
#define STORE_ROOT_PATH  STORE_ROOT
#define DICTPREFIX       STORE_ROOT "dic"
#define GNOMECFG         STORE_ROOT "kanjidic/"

#define MATCH(KEY, VALUE) if (g_strcmp0 (key, KEY) == 0){ return VALUE; } \
                          if (g_strcmp0 (key, STORE_ROOT_PATH KEY) == 0){ return VALUE; } \
                          if (g_strcmp0 (key, GNOMECFG KEY) == 0){ return VALUE; } \

/**
 * Struct for storing dictionary file information.
 * Pointers are owned.
 **/
// TODO:0 remove again and use GjitenDicfile directly
//        also remove functions
//        instead: create gjitendicfile_new (path, name)
typedef struct
{
  gchar * path;
  gchar * name;
}  DicFile;


/**
 * Param `path`: will be owned by struct
 * Param `name`: will be owned by struct
 **/
DicFile *
dicfile_new(gchar * path,
            gchar * name)
{
  DicFile * self = g_new (DicFile, 1);
  self->path = path;
  self->name = name;

  return self;
}

/**
 *  Free memory of members and itself
 **/
void
dicfile_delete(DicFile * self)
{
  if (NULL == self)
    return;

  if (NULL != self->path)
    g_free (self->path);
  if (NULL != self->name)
    g_free (self->name);

  self->path = NULL;
  self->name = NULL;

  g_free (self);
  self = NULL;
}



/**
 *  Persistent store interface functions
 *  (currently mocked)
 **/
gchar *
store_get_string (const gchar * key)
{
  MATCH("dictpath", "/usr/share/gjiten/dics/")
  MATCH("kanjipad", "/usr/bin/kanjipad")
  MATCH("largefont", "14px Sans")
  MATCH("normalfont", "22px Sans")
  MATCH("version", "2.6")

  #define HISTORY(N) MATCH ("history" N, "testhistory" N);
  HISTORY ("0"); HISTORY ("1"); HISTORY ("2"); HISTORY ("3"); HISTORY ("4"); HISTORY ("5"); HISTORY ("6");
  HISTORY ("7"); HISTORY ("8"); HISTORY ("9"); HISTORY ("10"); HISTORY ("11");
  HISTORY ("12"); HISTORY ("13"); HISTORY ("14"); HISTORY ("15");
  HISTORY ("16"); HISTORY ("17"); HISTORY ("18"); HISTORY ("19");
  HISTORY ("20"); HISTORY ("21"); HISTORY ("22"); HISTORY ("23");
  HISTORY ("24"); HISTORY ("25"); HISTORY ("26"); HISTORY ("27");
  HISTORY ("28"); HISTORY ("29"); HISTORY ("30"); HISTORY ("31");
  HISTORY ("32"); HISTORY ("33"); HISTORY ("34"); HISTORY ("35");
  HISTORY ("36"); HISTORY ("37"); HISTORY ("38"); HISTORY ("39");
  HISTORY ("40"); HISTORY ("41"); HISTORY ("42"); HISTORY ("43");
  HISTORY ("44"); HISTORY ("45"); HISTORY ("46"); HISTORY ("47");
  HISTORY ("48"); HISTORY ("49"); HISTORY ("50")


  // kanjidict
  MATCH ("kanjidicfile", "/usr/share/gjiten/dics/kanjidic");

  g_print ("store_get_string: default value was triggered:");
  g_print ("  Key: %s\n", key);
  // default
  return "default_string_from_conf";
}



gboolean
store_get_boolean(const gchar * key)
{
  // gjiten
  MATCH ("autoadjust_enabled", TRUE);
  MATCH ("bigkanji", FALSE);
  MATCH ("bigwords", FALSE);
  MATCH ("deinflection_enabled", FALSE);
  MATCH ("envvar_override", FALSE);
  MATCH ("force_ja_JP", FALSE);
  MATCH ("force_language_c", FALSE);
  MATCH ("gdk_use_xft", FALSE);
  MATCH ("search_hira_on_kata", FALSE);
  MATCH ("search_kata_on_hira", TRUE);
  MATCH ("searchlimit_enabled", FALSE);

  MATCH ("toolbar", TRUE);
  MATCH ("menubar", TRUE);

  // kanjidict
  MATCH ("bushu", FALSE);
  MATCH ("classic", FALSE);
  MATCH ("cref", FALSE);
  MATCH ("deroo", FALSE);
  MATCH ("eindex", FALSE);
  MATCH ("english", TRUE);
  MATCH ("fourc", FALSE);
  MATCH ("freq", TRUE);
  MATCH ("hindex", FALSE);
  MATCH ("iindex", FALSE);
  MATCH ("jisascii", FALSE);
  MATCH ("jouyou", TRUE);
  MATCH ("kanji", TRUE);
  MATCH ("kindex", FALSE);
  MATCH ("korean", TRUE);
  MATCH ("lindex", FALSE);
  MATCH ("missc", FALSE);
  MATCH ("mnindex", FALSE);
  MATCH ("mpindex", FALSE);
  MATCH ("nindex", FALSE);
  MATCH ("oindex", FALSE);
  MATCH ("pinyin", TRUE);
  MATCH ("radical", TRUE);
  MATCH ("reading", TRUE);
  MATCH ("skip", FALSE);
  MATCH ("strokes", TRUE);
  MATCH ("unicode", FALSE);
  MATCH ("unicode_radicals", TRUE);
  MATCH ("vindex", FALSE);

  g_print ("store_get_boolean: default value was triggered:");
  g_print ("  Key: %s\n", key);
  return TRUE;
}

gint
store_get_int (const gchar * key)
{
  MATCH ("maxwordmatches", 100);
  MATCH ("numofdics", 0); // prevent old style dictionary loading

  g_print ("store_get_int: default value was triggered:");
  g_print ("  Key: %s\n", key);
  return -1;
}


GSList * store_get_list   (const gchar * key)
{
  g_print ("store_get_list: default value was triggered:");
  g_print ("  Key: %s\n", key);

  return NULL;
}

void store_set_string (const gchar *b, gchar   *key){}
void store_set_boolean(const gchar *b, gboolean key){}
void store_set_int    (const gchar *b, gint     key){}
void store_set_list   (const gchar *b, GSList  *key){}

// -----------------------------------------------------------------------

// @return: TRUE, if init was successfull
gboolean store_init(){return TRUE;}
gboolean store_uninit(){return TRUE;}


GSList *
get_dict_lists()
{
  GSList * list = g_slist_alloc();
  DicFile * dic = dicfile_new (g_strdup("/usr/share/gjiten/dics/edict"),
                              g_strdup("edict"));
  list->data = dic;
  list->next = NULL;
  return list;
}



GjitenConfig *conf_load()
{
  gchar *dicprefix = DICTPREFIX;
  gchar *tmpstrg;
  gchar historystr[31];
  gchar *tmpptr, *endptr;
  gchar *gnomekcfg = GNOMECFG;
  int i;
  GjitenDicfile *dicfile;
  GSList *gconf_diclist = NULL;
  GSList *diclist;
  GjitenConfig *conf;

  conf = g_new0(GjitenConfig, 1);

  conf->version = store_get_string("version");

  //if (conf->version == NULL) { // FIXME: gconf schema
  //  conf->kdiccfg[KANJI] = TRUE;
  //  conf->kdiccfg[RADICAL] = TRUE;
  //  conf->kdiccfg[STROKES] = TRUE;
  //  conf->kdiccfg[READING] = TRUE;
  //  conf->kdiccfg[ENGLISH] = TRUE;
  //  conf->kdiccfg[FREQ] = TRUE;
  //  conf->kdiccfg[JOUYOU] = TRUE;
  //  conf->kdiccfg[CREF] = TRUE;
  //  conf->toolbar = TRUE;
  //  conf->menubar = TRUE;
  //  conf->force_ja_JP = TRUE;
  //  conf->force_language_c = TRUE;
  //  if (conf->kanjidic == NULL) conf->kanjidic = g_new0(GjitenDicfile, 1);
  //  conf->kanjidic->path = GJITEN_DICDIR"/kanjidic";
  //  conf->dictpath = GJITEN_DICDIR;
  //  conf->searchlimit_enabled = FALSE;
  //  conf->maxwordmatches = DEFMAXWORDMATCHES;
  //  conf->autoadjust_enabled = TRUE;
  //  return conf;
  //}


  conf->autoadjust_enabled = store_get_boolean("autoadjust_enabled");

  conf->bigwords = store_get_boolean("bigwords");
  conf->bigkanji = store_get_boolean("bigkanji");
  conf->largefont = store_get_string("largefont");
  conf->normalfont = store_get_string("normalfont");
  conf->gdk_use_xft = store_get_boolean("gdk_use_xft");
  conf->force_ja_JP = store_get_boolean("force_ja_JP");
  conf->force_language_c = store_get_boolean("force_language_c");
  conf->envvar_override = store_get_boolean("envvar_override");

  conf->searchlimit_enabled = store_get_boolean("searchlimit_enabled");
  conf->maxwordmatches = store_get_int("maxwordmatches");
  if (conf->maxwordmatches < 1) {
    conf->searchlimit_enabled = FALSE;
    conf->maxwordmatches = 100;
  }

  conf->dictpath = store_get_string("dictpath");
  conf->menubar = store_get_boolean("menubar");
  conf->toolbar = store_get_boolean("toolbar");

  conf->search_kata_on_hira = store_get_boolean("search_kata_on_hira");
  conf->search_hira_on_kata = store_get_boolean("search_hira_on_kata");
  conf->verb_deinflection = store_get_boolean("deinflection_enabled");

  if (conf->kanjidic == NULL) conf->kanjidic = g_new0(GjitenDicfile, 1);
  conf->kanjidic->path = store_get_string(GNOMECFG "kanjidicfile");
  if ((conf->kanjidic->path == NULL) || (strlen(conf->kanjidic->path)) == 0) {
    conf->kanjidic->path = GJITEN_DICDIR"/kanjidic";
  }
  conf->unicode_radicals = store_get_boolean(GNOMECFG "unicode_radicals");


  conf->kanjipad = store_get_string("kanjipad");
  if (conf->kanjipad == NULL) conf->kanjipad = "";

  conf->numofdics = store_get_int("numofdics");

  if (conf->dicfile_list != NULL) {
    dicutil_unload_dic();
    dicfile_list_free(conf->dicfile_list);
    conf->dicfile_list = NULL;
  }

  if (conf->numofdics != 0) {
    //Load dicfiles from old style config [compatibility with older versions]
    for (i = 0; i < conf->numofdics; i++) {
      //if (i == MAXDICFILES - 1) break;
      tmpstrg = g_strdup_printf("%s%d", dicprefix, i);
      dicfile = g_new0(GjitenDicfile, 1);
      dicfile->name = store_get_string(tmpstrg);
      if (conf->dictpath[strlen(conf->dictpath - 1)] == '/') {
        dicfile->path = g_strdup_printf("%s%s", conf->dictpath, dicfile->name);
      }
      else {
        dicfile->path = g_strdup_printf("%s/%s", conf->dictpath, dicfile->name);
      }
      conf->dicfile_list = g_slist_append(conf->dicfile_list, dicfile);
      g_free(tmpstrg);
    }
  }
  else { //new config
    gconf_diclist = get_dict_lists();
    diclist = gconf_diclist;
    while (diclist != NULL) {
      if (diclist->data == NULL) break;
      DicFile * dicfile2 = (DicFile*)diclist->data;
      //tmpstrg = diclist->data;
      dicfile = g_new0(GjitenDicfile, 1);
      dicfile->path = g_strdup ( dicfile2->path);
      dicfile->name = g_strdup ( dicfile2->name);
      conf->dicfile_list = g_slist_append(conf->dicfile_list, dicfile);

      diclist = g_slist_next(diclist);
      dicfile_delete (dicfile2);
    }
  }
  if (conf->dicfile_list != NULL) conf->selected_dic = conf->dicfile_list->data;

  //Load kanji info settings
  for (i = 0; i < KCFGNUM; i++) {
    tmpptr = g_strdup_printf("%s%s", gnomekcfg, kanjidicstrg[i]);
    if (store_get_boolean(tmpptr)) {
      conf->kdiccfg[i] = TRUE;
      // printf("%s : %d\n",kanjidicstrg[i], conf->kdiccfg[i]);
    }
    else conf->kdiccfg[i] = FALSE;
    g_free(tmpptr);
  }

  //Load gjiten search history
  for (i = 0; i <= 50; i++) {
    snprintf(historystr, 31, "history%d", i);
    conf->history[i] = store_get_string(historystr);
  }

  return conf;
}

void conf_save(GjitenConfig *conf) {
  gchar *gnomekcfg = GNOMECFG;
  int i;
  gchar *confpath, *tmpstrg;
  GSList *gconf_diclist = NULL;
  GSList *diclist;
  GjitenDicfile *dicfile;

  store_set_string("version", VERSION);
  //Save kanjidic display options
  for (i = 0; i < KCFGNUM; i++) {
    confpath = g_strdup_printf("%s%s", gnomekcfg, kanjidicstrg[i]);
    store_set_boolean(confpath, conf->kdiccfg[i]);
    g_free(confpath);
  }


  store_set_boolean("menubar", conf->menubar);
  store_set_boolean("toolbar", conf->toolbar);
  store_set_string("dictpath", conf->dictpath == NULL ? "" : conf->dictpath);
  store_set_string(GNOMECFG "kanjidicfile", conf->kanjidic->path);
  store_set_boolean(GNOMECFG "unicode_radicals", conf->unicode_radicals);

  if (conf->kanjipad == NULL) conf->kanjipad = "";
  store_set_string("kanjipad", conf->kanjipad);

  //Deprecated dictionary file number, zero it out.
  //store_set_int("numofdics", conf->numofdics);
  store_set_int("numofdics", 0);

  store_set_boolean("bigwords", conf->bigwords);
  store_set_boolean("bigkanji", conf->bigkanji);
  store_set_string("largefont", conf->largefont == NULL ? "" : conf->largefont);
  store_set_string("normalfont", conf->normalfont == NULL ? "" : conf->normalfont);
  store_set_boolean("gdk_use_xft", conf->gdk_use_xft);
  store_set_boolean("force_ja_JP", conf->force_ja_JP);
  store_set_boolean("force_language_c", conf->force_language_c);
  store_set_boolean("envvar_override", conf->envvar_override);

  store_set_boolean("search_kata_on_hira", conf->search_kata_on_hira);
  store_set_boolean("search_hira_on_kata", conf->search_hira_on_kata);
  store_set_boolean("deinflection_enabled", conf->verb_deinflection);

  //Save dicfiles [path and name seperated with linebreak]
  /* TODO:impl
  gconfList = gconf_value_new(GCONF_VALUE_LIST);
  diclist = conf->dicfile_list;
  while (diclist != NULL) {
    if (diclist->data == NULL) break;
    dicfile = diclist->data;
    tmpstrg = g_strdup_printf("%s\n%s", dicfile->path, dicfile->name);
    gconf_diclist = g_slist_append(gconf_diclist, tmpstrg);
    diclist = g_slist_next(diclist);
  }
  gconf_value_set_list_type(gconfList);
  store_set_list("dictionary_list", gconf_diclist);
  //*/
}
void conf_save_history(GtkListStore *history, GjitenConfig *conf) {
  char historystr[40];
  int i;
  if (history != NULL) {
    GtkTreeIter iter;
    gboolean iter_valid;
    gchar *tmp;

    iter_valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (history), &iter);
    for (i = 0; i <= 50; i++) {
      if (iter_valid == FALSE) break;
      snprintf(historystr, 31, STORE_ROOT_PATH "history%d", i);
      tmp = gtk_list_store_get_string (history, &iter);
      store_set_string(historystr, tmp);
      g_free (tmp);
      iter_valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (history), &iter);
    }
  }
}

void conf_save_options(GjitenConfig *conf) {
    store_set_boolean("autoadjust_enabled", conf->autoadjust_enabled);
    store_set_boolean("searchlimit_enabled", conf->searchlimit_enabled);
    store_set_int("maxwordmatches", conf->maxwordmatches);
}

gboolean conf_init_handler() {

  if (store_init() != TRUE) {
    gjiten_print_error(_("Could initialize persistent data store.\n"));
    return FALSE;
  }
  return TRUE;
}

void conf_close_handler() {
  store_uninit();
}
