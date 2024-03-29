/* -*- Mode: C; tab-width: 2;   indent-tabs-mode: space; c-basic-offset: 2 -*- */
/* vi: set ts=2 sw=2: */
/* kanjidic.c

   GJITEN : A GTK+/GNOME BASED JAPANESE DICTIONARY

   Copyright (C) 1999-2005 Botond Botyanszki <boti@rocketmail.com>
   xjdic code Copyright (C) 1998 Jim Breen

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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "constants.h"
#include "kanjidic.h"
#include "conf.h"
#include "dicfile.h"
#include "dicutil.h"
#include "worddic.h"
#include "gjiten.h"
#include "pref.h"
#include "gjiten.h"
#include "error.h"
#include "kanjidicconsts.h"
#include "utils.h"
#include "radicals.h"
#include "radicals_ui.h"

/*====== Prototypes========================================================*/
void get_rad_of_kanji (gunichar kanji, gchar *kanjidic_line);


typedef struct _GjKanjidicWindowPrivate GjKanjidicWindowPrivate;
struct _GjKanjidicWindowPrivate
{
  GtkWidget     *window_kanjinfo;
  GtkWidget     *window_radicals;
  GtkComboBox   *combo_entry_key;
  GtkComboBox   *combo_entry_radical;
  GtkTextBuffer *text_kanjinfo_buffer;
  GtkWidget     *text_kanjinfo_view;
  GtkWidget     *kanji_results_view;
  GtkTextBuffer *kanji_results_buffer;
  GtkWidget     *appbar_kanji;
  GtkListStore  *combo_entry_key_list;
  GtkListStore  *combo_entry_radical_list;
  GtkWidget  *spinb_strokenum;
  GtkWidget  *spinb_plusmin;
  GtkWidget  *label_plusmin;
  GtkWidget  *checkb_ksearch;
  GtkWidget  *checkb_radical;
  GtkWidget  *checkb_stroke;
  GtkWidget  *button_radtable;
  GtkWidget  *vbox_history;
  GtkWidget  *scrolledwin_history;
  GSList     *kanji_history_list;
  GtkTextTag *tag_large_font;
  GHashTable *rad_button_hash;
};
G_DEFINE_TYPE_WITH_PRIVATE (GjKanjidicWindow, gj_kanjidic_window,  GTK_TYPE_APPLICATION_WINDOW)


/* VARIABLES ************************/

gchar kanjiselected[2];
extern guint32 srchpos;

GList *klinklist = NULL, *tmpklinklist = NULL;

static GjKanjidicWindow *self = NULL;
static GjKanjidicWindowPrivate *kanjiDic = NULL;

extern gchar *strginfo[];
extern GjitenApp *gjitenApp;

#define TOOLTIP_SEARCH_BY_KEY _( \
"Search by anything that the Kanjidic file contains:\n" \
"- The kanji itself\n"                                  \
"- The reading (in kana)\n"                             \
"- English meaning\n"                                   \
"- Kanjidic code\n"                                     \
"- ...\n"                                               \
"(see help for more information)")


/* ************************************************************ */


/**
 *  Load radical data
 **/
Radicals *
radicals_the_instance()
{
  static Radicals * radicals = NULL;
  if (NULL == radicals) {
    radicals = radicals_new(gjitenApp->conf->unicode_radicals);
  }

  return radicals;
}



/**
 * Return value does not need to be freed
 **/
gchar *
do_kdicline(gchar *kstr)
{
  static gchar *kdic_line = NULL;  /*size = KCFGNUM * KBUFSIZE */
  char tmpstr[KBUFSIZE];
  int i, pos;

  if (kdic_line == NULL) kdic_line = (gchar *)g_malloc (KCFGNUM * KBUFSIZE);
  if (kdic_line == NULL) error_show_and_quit ("Couldn't allocate memory\n");

  for (i = 0; i < KCFGNUM * KBUFSIZE; i++) { //clear it
    kdic_line[i] = 0;
  }

  g_unichar_to_utf8(g_utf8_get_char (kstr), &kdic_line[KANJI * KBUFSIZE] ); //KANJI
  get_rad_of_kanji (g_utf8_get_char (kdic_line + KANJI * KBUFSIZE), kdic_line); //RADICAL

  get_word (kdic_line + JIS * KBUFSIZE, kstr, KBUFSIZE, 3);
  pos = 7;

  while (pos != 0) {
    pos = get_word (tmpstr, kstr, sizeof (tmpstr), pos);

    if ((tmpstr[0] >> 7)) {       // jap char   //FIXME
       if (strlen (kdic_line + READING * KBUFSIZE) != 0) {
        strncat (kdic_line + READING * KBUFSIZE, ", ", KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
      }
      strncat (kdic_line + READING * KBUFSIZE, tmpstr, KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
    }
    else switch (tmpstr[0]) {
        case '-' :
          if (strlen (kdic_line + READING * KBUFSIZE) != 0) {
            strncat (kdic_line + READING * KBUFSIZE, ", ", KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
          }
          strncat (kdic_line + READING * KBUFSIZE, tmpstr, KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
          break;

        case 'T':
          if (tmpstr[1] == '1') {
            if (strlen (kdic_line + READING * KBUFSIZE) != 0) {
              strncat (kdic_line + READING * KBUFSIZE, ", ", KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
              strncat (kdic_line + READING * KBUFSIZE, _("Name readings:"), KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
            }
            else {
              strncat (kdic_line + READING * KBUFSIZE, _("Name readings:"), KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
            }
            pos = get_word (tmpstr, kstr, sizeof (tmpstr), pos);
            strncat (kdic_line + READING * KBUFSIZE, tmpstr, KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
            break;
          }
          if (tmpstr[1] == '2') {
            if (strlen (kdic_line + READING * KBUFSIZE) != 0) {
              strncat (kdic_line + READING * KBUFSIZE, ", Radical Name: ", KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
            }
            else {
              strncat (kdic_line + READING * KBUFSIZE, _("Radical name:"), KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
            }
            pos = get_word (tmpstr, kstr, sizeof (tmpstr), pos);
            strncat (kdic_line + READING * KBUFSIZE, tmpstr, KBUFSIZE - strlen (kdic_line + READING * KBUFSIZE) - 1);
            break;
          }

        case '{': // english meaning
          if (strlen (kdic_line + ENGLISH * KBUFSIZE) != 0 ) {
            strncat (kdic_line + ENGLISH * KBUFSIZE, " ", KBUFSIZE - strlen (kdic_line + ENGLISH * KBUFSIZE) - 1);
          }
          strncat (kdic_line + ENGLISH * KBUFSIZE, tmpstr + 1, KBUFSIZE - strlen (kdic_line + ENGLISH * KBUFSIZE) - 1);
          strncat (kdic_line + ENGLISH * KBUFSIZE, ";", KBUFSIZE - strlen (kdic_line + ENGLISH * KBUFSIZE) - 1); // put endmark: ;
          break;

        case 'B':
          strncpy (kdic_line + BUSHU * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'C':
          strncpy (kdic_line + CLASSIC * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'F':
          strncpy (kdic_line + FREQ * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'G':
          strncpy (kdic_line + JOUYOU * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'H':
          strncpy (kdic_line + HINDEX * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'N':
          strncpy (kdic_line + NINDEX * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'V':
          strncpy (kdic_line + VINDEX * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'D':
          strncpy (kdic_line + DEROO * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'P':
          strncpy (kdic_line + SKIP * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'S':
          if (strlen (kdic_line + STROKES * KBUFSIZE) == 0) {
            strncpy (kdic_line + STROKES * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          }
          else {
            strncat (kdic_line + STROKES * KBUFSIZE, _(", Common miscount: "), KBUFSIZE - strlen (kdic_line + STROKES * KBUFSIZE) - 1);
            strncat (kdic_line + STROKES * KBUFSIZE, tmpstr + 1, KBUFSIZE - strlen (kdic_line + STROKES * KBUFSIZE) - 1);
          }
          break;

        case 'U':
          strncpy (kdic_line + UNI * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'I':
          strncpy (kdic_line + IINDEX * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'Q':
          strncpy (kdic_line + FOURC * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'M':
          if (tmpstr[1] == 'N') strncpy (kdic_line + MNINDEX * KBUFSIZE, tmpstr + 2, KBUFSIZE);
          else if (tmpstr[1] == 'P') strncpy (kdic_line + MPINDEX * KBUFSIZE, tmpstr + 2, KBUFSIZE);
          break;

        case 'E':
          strncpy (kdic_line + EINDEX * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'K':
          strncpy (kdic_line + KINDEX * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'L':
          strncpy (kdic_line + LINDEX * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'O':
          strncpy (kdic_line + OINDEX * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'W':
          strncpy (kdic_line + KOREAN * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'Y':
          strncpy (kdic_line + PINYIN * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'X':
          strncpy (kdic_line + CREF * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;

        case 'Z':
          strncpy (kdic_line + MISSC * KBUFSIZE, tmpstr + 1, KBUFSIZE);
          break;
    }
  }

  return kdic_line;
}


void
show_kanjiinfo(gunichar kanji)
{
  GtkTextIter kinfo_iter;
  gint roff, rlen;
  gchar repstr[1024];
  guint32 respos;
  gchar kanjistr[6];

  for (gint i = 0; i < 6; i++) kanjistr[i] = 0;
  g_unichar_to_utf8(kanji, kanjistr);

  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (kanjiDic->text_kanjinfo_buffer), "", 0);
  gtk_text_buffer_get_start_iter (kanjiDic->text_kanjinfo_buffer, &kinfo_iter);

  srchpos = 0;
  search4string(SRCH_START, gjitenApp->conf->kanjidic, kanjistr, &respos, &roff, &rlen, repstr);
  gchar * kdictline = do_kdicline (repstr);


  for (gint i = 0; i < KCFGNUM; i++)
    if (gjitenApp->conf->kdiccfg[i] == TRUE) {
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (kanjiDic->text_kanjinfo_buffer),
                                               &kinfo_iter, _(strginfo[i]), -1, "blue_foreground", NULL);
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (kanjiDic->text_kanjinfo_buffer), &kinfo_iter,
                                               ": ", -1, "blue_foreground", NULL);
      if (i == KANJI)
      {
        if (gjitenApp->conf->bigkanji == FALSE)
        {
          gtk_text_buffer_insert (GTK_TEXT_BUFFER (kanjiDic->text_kanjinfo_buffer), &kinfo_iter, kdictline + i * KBUFSIZE, -1);
        }
        else
        {
          gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (kanjiDic->text_kanjinfo_buffer),
                                                   &kinfo_iter, kdictline + i * KBUFSIZE, -1, "largefont", NULL);
        }
      }
      else {
        gtk_text_buffer_insert (GTK_TEXT_BUFFER (kanjiDic->text_kanjinfo_buffer), &kinfo_iter, kdictline + i * KBUFSIZE, -1);
      }
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (kanjiDic->text_kanjinfo_buffer), &kinfo_iter, "\n", -1);
    }
}


/* compares two lists and combines the matching kanji into one */
void
klists_merge(void)
{
  GList *ptr1, *ptr2, *nextptr;
  int found;

  ptr1 = klinklist;
  while (ptr1 != NULL) {
    nextptr = g_list_next (ptr1);
    found = FALSE;
    ptr2 = tmpklinklist;
    while (ptr2 != NULL) {
      if (POINTER_TO_UNICHAR (ptr1->data) == POINTER_TO_UNICHAR (ptr2->data)) {
        found = TRUE;
        break;
      }
      ptr2 = g_list_next (ptr2);
    }
    if (found == FALSE) {
      klinklist = g_list_remove (klinklist, ptr1->data);
    }
    ptr1 = nextptr;
  }
  g_list_free (tmpklinklist);
  tmpklinklist = NULL;
}

void
findk_by_key(gchar  *srchkey,
             GList **list)
{
  gint srch_resp, roff, rlen;
  gchar repstr[1024];
  guint32 respos, oldrespos;
  gint loopnum = 0;

  srchpos = 0;
  srch_resp = search4string (SRCH_START, gjitenApp->conf->kanjidic, srchkey, &respos, &roff, &rlen, repstr);
  //printf ("F: Returning:srch_resp:%d\n respos:%ld\n roff:%d rlen:%d\n repstr:%s\n", srch_resp,respos,roff,rlen,repstr);
  if (srch_resp != SRCH_OK) return; // (FALSE);
  oldrespos = srchpos = respos;
  *list = g_list_prepend (*list, TO_POINTER (g_utf8_get_char (repstr)));

  while (roff != 0) {
    oldrespos = respos;
    srchpos++;
    loopnum++;
    srch_resp = search4string (SRCH_CONT, gjitenApp->conf->kanjidic, srchkey, &respos, &roff, &rlen, repstr);
    //printf ("srch_resp:%d\n respos:%ld\n roff:%d rlen:%d\n repstr:%s\n",srch_resp,respos,roff,rlen,repstr);
    if (srch_resp != SRCH_OK) break;
    if (oldrespos == respos) continue;
    *list = g_list_prepend (*list, TO_POINTER (g_utf8_get_char (repstr)));
  }
}

void
findk_by_stroke(int     stroke,
                int     plusmin,
                GList **list)
{
  static char srchkey[10];
  int i, lowerlim, upperlim;

  upperlim = stroke + plusmin;
  if (upperlim > 30) upperlim = 30;
  lowerlim = stroke - plusmin;
  if (lowerlim < 1) lowerlim = 1;

  for (i = lowerlim; i <= upperlim ; i++) {
    snprintf (srchkey, 10, " S%d ", i);
    findk_by_key (srchkey, list);
  }
}

void
findk_by_radical(gchar *radstrg)
{
  gint i, radnum;
  RadInfo *rad_info;
  GList *kanji_info_list;
  gchar *tmprad;
  gchar *radstr_ptr;

  if (g_utf8_strlen (radstrg, -1) == 0) return;

  radstr_ptr = radstrg;
  rad_info = g_hash_table_lookup (radicals_the_instance ()->rad_info_hash, TO_POINTER (g_utf8_get_char (radstr_ptr)));
  if (rad_info == NULL) {
    error_show (NULL,_("Invalid radical!\n"));
    return;
  }

  for (kanji_info_list = rad_info->kanji_info_list;
       kanji_info_list != NULL;
       kanji_info_list = g_list_next (kanji_info_list)) {
    klinklist = g_list_prepend (klinklist, TO_POINTER ((KanjiInfo *) kanji_info_list->data)->kanji);
  }

  // if we have more than 1 radical
  radnum = g_utf8_strlen (radstrg, -1);
  if (radnum > 1) {
    for (i = 1; i <= radnum; i++) {
      rad_info = g_hash_table_lookup (radicals_the_instance ()->rad_info_hash, TO_POINTER (g_utf8_get_char (radstr_ptr)));
      if (rad_info == NULL) {
        tmprad = g_strndup (radstr_ptr, sizeof (gunichar));
        error_show (NULL,_("I don't seem to recognize this radical: '%s'.\n"), tmprad);
        g_free (tmprad);
        return;
      }
      for (kanji_info_list = rad_info->kanji_info_list;
           kanji_info_list != NULL;
           kanji_info_list = g_list_next (kanji_info_list)) {
        tmpklinklist = g_list_prepend (tmpklinklist, TO_POINTER ((KanjiInfo *) kanji_info_list->data)->kanji);
      }
      klists_merge ();
      radstr_ptr = g_utf8_next_char (radstr_ptr);
    }
  }
}

void
set_radical_button_sensitive(gpointer radical,
                             RadInfo *rad_info,
                             gpointer user_data)
{
  GtkWidget *rad_button = g_hash_table_lookup (kanjiDic->rad_button_hash, radical);
  if (GTK_IS_WIDGET (rad_button) && (gtk_widget_get_sensitive (rad_button) != TRUE)) {
    gtk_widget_set_sensitive (rad_button, TRUE);
  }
}

void
set_radical_button_unsensitive(gunichar   radical,
                               GtkWidget *rad_button,
                               gboolean   sensitive)
{
  if (GTK_IS_WIDGET (rad_button) && (gtk_widget_get_sensitive (rad_button) != sensitive)) {
    gtk_widget_set_sensitive (rad_button, sensitive);
  }
}


void
on_kanji_search()
{
  static gchar *kentry, *radentry;
  int found;
  int stroke, plus_min;
  GList *node_ptr;
  gchar kappbarmsg[100];
  int push;
  GtkWidget *kanji_result_button;
  gchar kanji_result_str[6];
  gchar kanji_result_labelstr[100];
  GtkWidget *kanji_result_label;
  GtkTextChildAnchor *kanji_results_anchor;
  gint result_num = 0;
  GList *rad_info_list = NULL;
  GList *kanji_list = NULL;
  GList *kanji_list_ptr = NULL;
  GHashTable *rad_info_hash = NULL;
  KanjiInfo *kanji_info;
  GtkTextIter    kanji_results_iter;

  gtk_label_set_text (GTK_LABEL (kanjiDic->appbar_kanji), _("Searching..."));
  kappbarmsg[0] = 0;

  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (kanjiDic->kanji_results_buffer), "", 0);
  gtk_text_buffer_get_start_iter (kanjiDic->kanji_results_buffer, &kanji_results_iter);

  push = TRUE;
  if (kentry != NULL) { //Check if we need to save the key entry in the history
    if (strcmp (kentry, gtk_combo_box_get_text (kanjiDic->combo_entry_key)) == 0) {
      push = FALSE;
      g_free (kentry);
    }
  }
  kentry = g_strdup (gtk_combo_box_get_text (kanjiDic->combo_entry_key));
  if (kentry != NULL) {
    if ((strlen (kentry) > 0) && (push == TRUE) ) {
      gtk_list_store_string_prepend (kanjiDic->combo_entry_key_list, kentry);
    }
  }
  push = TRUE;
  if (radentry != NULL) { //Check if we need to save the radical entry in the history
    if (strcmp (radentry, gtk_combo_box_get_text (kanjiDic->combo_entry_radical)) == 0) {
      push = FALSE;
      g_free (radentry);
    }
  }

  radentry = g_strdup (gtk_combo_box_get_text (kanjiDic->combo_entry_radical));
  if (radentry) {
    if ((strlen (radentry) > 0) && push) {
      gtk_list_store_string_prepend (kanjiDic->combo_entry_radical_list, radentry);
    }
  }

  stroke = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (kanjiDic->spinb_strokenum));
  plus_min = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (kanjiDic->spinb_plusmin));

  if (klinklist != NULL) g_list_free (klinklist);
  tmpklinklist = NULL;
  klinklist = NULL;
  found = TRUE;

  if (GTK_IS_WIDGET (kanjiDic->window_radicals)) {
    g_hash_table_foreach (kanjiDic->rad_button_hash, (GHFunc) set_radical_button_unsensitive, (gpointer) TRUE);
  }

  //FIND BY RADICAL
  if ((gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (kanjiDic->checkb_radical))) && (g_utf8_strlen (radentry, -1) > 0)) {
    findk_by_radical (radentry);
    if (klinklist == NULL) {
      gtk_label_set_text (GTK_LABEL (kanjiDic->appbar_kanji),_("No such kanji with this radical combination."));
      return;
    }
  }

  //FIND BY STROKE
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (kanjiDic->checkb_stroke))) {
    if ((stroke < 1) || (stroke > 30)) {
        gtk_label_set_text (GTK_LABEL (kanjiDic->appbar_kanji),_("Invalid stroke count :-P "));
        return;
    }
    if (klinklist == NULL) {
      findk_by_stroke (stroke, plus_min, &klinklist);  // this should! give results
      if (klinklist == NULL ) {
        gtk_label_set_text (GTK_LABEL (kanjiDic->appbar_kanji),_("Stroke search didn't find any match :-O "));
        return;
      }
    }
    else {
      findk_by_stroke (stroke, plus_min, &tmpklinklist);
      klists_merge ();
      if (klinklist == NULL) {
        found = FALSE;
        gtk_label_set_text (GTK_LABEL (kanjiDic->appbar_kanji),
                                _("No such kanji with this stroke/radical combination."));
        return;
      }
    }
  }

  //FIND BY KEY
  if ((found) && (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (kanjiDic->checkb_ksearch))) && (strlen (kentry) >= 1)) {
    if (klinklist == NULL) findk_by_key (kentry, &klinklist);
    else {
      findk_by_key (kentry, &tmpklinklist);
      klists_merge ();
    }
    if (klinklist == NULL) {
      gtk_label_set_text (GTK_LABEL (kanjiDic->appbar_kanji), _("No Matches found."));
      return;
    }
  }

  result_num = g_list_length (klinklist);
  snprintf (kappbarmsg, 100, _("Kanji found: %d"), result_num);
  gtk_label_set_text (GTK_LABEL (kanjiDic->appbar_kanji), kappbarmsg);

  if (result_num == 1) show_kanjiinfo (POINTER_TO_UNICHAR (klinklist->data));


  // PRINT OUT KANJI FOUND
  node_ptr = klinklist;
  while (node_ptr != NULL) {
    kanji_list = g_list_prepend (kanji_list, node_ptr->data);
    memset (kanji_result_str, 0, sizeof (kanji_result_str));
    g_unichar_to_utf8(POINTER_TO_UNICHAR (node_ptr->data), kanji_result_str);
    //printf ("%s\n", kanji_result_str);
    g_snprintf (kanji_result_labelstr, 100, "<span size=\"xx-large\">%s</span>", kanji_result_str);
    kanji_results_anchor = gtk_text_buffer_create_child_anchor (kanjiDic->kanji_results_buffer, &kanji_results_iter);

    if (gjitenApp->conf->bigkanji == TRUE) {
      kanji_result_label = gtk_label_new (NULL);
      gtk_label_set_markup (GTK_LABEL (kanji_result_label), kanji_result_labelstr);
      gtk_widget_show (kanji_result_label);
      kanji_result_button = gtk_button_new ();
      gtk_container_add (GTK_CONTAINER (kanji_result_button), kanji_result_label);
    }
    else {
     kanji_result_label = gtk_label_new (kanji_result_str);
     gtk_widget_show (kanji_result_label);
     kanji_result_button = gtk_button_new ();
     gtk_container_add (GTK_CONTAINER (kanji_result_button), kanji_result_label);
    }
    gtk_widget_style_add_class (kanji_result_label, "normalfont");

    gtk_widget_show (kanji_result_button);
    g_signal_connect_swapped (G_OBJECT (kanji_result_button), "clicked", G_CALLBACK (kanji_selected),
                             node_ptr->data);
    gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (kanjiDic->kanji_results_view), kanji_result_button, kanji_results_anchor);
    node_ptr = g_list_next (node_ptr);
  }

  // find all different radicals in all the kanji found
  if (GTK_IS_WIDGET (kanjiDic->window_radicals)) {
    if (kanji_list != NULL) {
      rad_info_hash = g_hash_table_new (NULL, NULL);
      for (kanji_list_ptr = kanji_list;
           kanji_list_ptr != NULL;
           kanji_list_ptr = g_list_next (kanji_list_ptr)) {
        kanji_info = g_hash_table_lookup (radicals_the_instance ()->kanji_info_hash, kanji_list_ptr->data);
        for (rad_info_list = kanji_info->rad_info_list;
             rad_info_list != NULL;
             rad_info_list = g_list_next (rad_info_list)) {
          g_hash_table_insert (rad_info_hash, TO_POINTER ((RadInfo *) rad_info_list->data)->radical, rad_info_list->data);
        }
      }
      g_hash_table_foreach (kanjiDic->rad_button_hash, (GHFunc) set_radical_button_unsensitive, (gpointer) FALSE);

      g_hash_table_foreach (rad_info_hash, (GHFunc) set_radical_button_sensitive, NULL);
      g_hash_table_destroy (rad_info_hash);
      g_list_free (kanji_list);
    }
    else { // none found, so set everything sensitive
      g_hash_table_foreach (kanjiDic->rad_button_hash, (GHFunc) set_radical_button_unsensitive, (gpointer) TRUE);
    }
  }
}


int
kanjidic_radical_selected(gunichar radical)
{
  int i, j;
  gchar radical_selected[6];
  gchar tmpchar[6];
  const gchar *radline_ptr;
  gchar *newradline;
  int removed;
  int radline_length = 0;

  memset (radical_selected, 0, sizeof (radical_selected));
  g_unichar_to_utf8(radical, radical_selected);

  radline_ptr = gtk_combo_box_get_text (kanjiDic->combo_entry_radical);
  newradline = g_strndup (radline_ptr, strlen (radline_ptr) + 6); //Enough space for one more character
  radline_length = g_utf8_strlen (newradline, -1);

  for (i = 0; i < (int) (strlen (newradline) + 6); i++) newradline[i] = 0; //clear newradline

  removed = FALSE;
  for (i = 0; i < radline_length; i++) {  //Check if we already have the radical in the line
    if (g_utf8_get_char (radline_ptr) != radical) {
      for (j = 0; j < 6; j++) tmpchar[j] = 0;
      g_unichar_to_utf8(g_utf8_get_char (radline_ptr), tmpchar);
      strncat (newradline, tmpchar, 5);
    }
    else removed = TRUE;  //if it's there then remove it
    radline_ptr = g_utf8_next_char (radline_ptr);
  }

  if (removed == FALSE) strncat (newradline, radical_selected, 5); //Add the radical to the line
  gtk_combo_box_set_text (kanjiDic->combo_entry_radical, newradline);

  g_free (newradline);

  on_kanji_search ();

  return 0;
}

void history_add(gunichar unicharkanji)
{
  GtkWidget *history_kanji_button;
  GtkWidget *history_kanji_label;
  gchar history_kanji_labelstr[100];
  gchar kanji[6];
  gunichar *unichar_list_elem;
  int i;

  if (kanjiDic->kanji_history_list != NULL) {
    if ((*((gunichar *)(kanjiDic->kanji_history_list->data))) == unicharkanji) return;
  }
  unichar_list_elem = (gunichar *) malloc (sizeof (gunichar));
  *unichar_list_elem = unicharkanji;

  for (i = 0; i < 6; i++) kanji[i] = 0;
  g_unichar_to_utf8(unicharkanji, kanji);

  kanjiDic->kanji_history_list = g_slist_prepend (kanjiDic->kanji_history_list, unichar_list_elem);

  if (gjitenApp->conf->bigkanji == TRUE) {
    g_snprintf (history_kanji_labelstr, 100, "<span size=\"large\">%s</span>", kanji);
    history_kanji_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (history_kanji_label), history_kanji_labelstr);
    gtk_widget_show (history_kanji_label);
    history_kanji_button = gtk_button_new ();
    gtk_container_add (GTK_CONTAINER (history_kanji_button), history_kanji_label);
  }
  else {
      history_kanji_label = gtk_label_new (kanji);
      gtk_widget_show (history_kanji_label);
      history_kanji_button = gtk_button_new ();
      gtk_container_add (GTK_CONTAINER (history_kanji_button), history_kanji_label);
  }
  gtk_widget_style_add_class (history_kanji_label, "normalfont");


  g_signal_connect_swapped (G_OBJECT (history_kanji_button), "clicked", G_CALLBACK (kanji_selected),
         TO_POINTER (*unichar_list_elem));
  gtk_box_pack_start (GTK_BOX (kanjiDic->vbox_history), history_kanji_button, FALSE, FALSE, 0);
  gtk_box_reorder_child (GTK_BOX (kanjiDic->vbox_history), history_kanji_button, 0);
  gtk_widget_show (history_kanji_button);

  if (gtk_widget_get_visible (kanjiDic->scrolledwin_history) != TRUE)
    gtk_widget_show (kanjiDic->scrolledwin_history);
}


void
kanji_selected(gunichar kanji)
{
  show_kanjiinfo (kanji);
  history_add (kanji);
  GJITEN_DEBUG ("KANJI_SELECTED\n");
}



GtkComboBox *
createStringComboBox()
{
  GtkListStore * model;
  GtkComboBox  * comboBox;

  model = gtk_list_store_string_new ();
  comboBox = GTK_COMBO_BOX (gtk_combo_box_new_with_model_and_entry (GTK_TREE_MODEL (model)));
  g_object_unref (model);
  gtk_combo_box_set_entry_text_column (comboBox, 0);
  gtk_combo_box_set_id_column (comboBox, 0);

  return comboBox;
}



//get all radicals that the kanji has
void
get_rad_of_kanji(gunichar kanji, gchar *_kanjidic_line)
{
  gchar *kdicline_ptr;
  KanjiInfo *kanji_info;
  GList *rad_info_list;

  kdicline_ptr = _kanjidic_line + RADICAL * KBUFSIZE;

  kanji_info = g_hash_table_lookup (radicals_the_instance ()->kanji_info_hash, TO_CONST_POINTER (kanji));
  if (kanji_info != NULL) {
    for (rad_info_list = kanji_info->rad_info_list;
         rad_info_list != NULL;
         rad_info_list = g_list_next (rad_info_list)) {
      memset (kdicline_ptr, 0, 6);
      if (kdicline_ptr >= _kanjidic_line + RADICAL * KBUFSIZE + KBUFSIZE - 7) return;
      g_unichar_to_utf8(((RadInfo *) rad_info_list->data)->radical, kdicline_ptr);
      kdicline_ptr = g_utf8_next_char (kdicline_ptr);
      g_unichar_to_utf8(' ', kdicline_ptr);
      kdicline_ptr = g_utf8_next_char (kdicline_ptr);
    }
  }
}



/**
 * Returns:
 *  -1 => error
 *   0 => nothing was found
 *   1 => something was found
 **/
gint
kanjidic_lookup (const gchar*  kanji)
{
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (kanjiDic->checkb_stroke), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (kanjiDic->checkb_radical), FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (kanjiDic->checkb_ksearch), TRUE);
  gtk_combo_box_set_text (kanjiDic->combo_entry_key, kanji);

  on_kanji_search();

  return 1;
}


static void
radicals_window_close()
{
  if (kanjiDic)
    kanjiDic->window_radicals = NULL;
}




static GtkWidget *
show_window_radicals()
{
  if (kanjiDic->window_radicals != NULL) {
    gtk_window_present (GTK_WINDOW (kanjiDic->window_radicals));
    return kanjiDic->window_radicals;
  }

  kanjiDic->window_radicals =
  radicals_window_new (self, radicals_the_instance (),
                      &(kanjiDic->rad_button_hash));

  g_signal_connect (kanjiDic->window_radicals, "destroy", G_CALLBACK (radicals_window_close), NULL);

  return kanjiDic->window_radicals;
}



void
kanjidic_destruct()
{
  // Avoid recursion
  g_object_ref_sink (self);
  gtk_widget_destroy (GTK_WIDGET (self));

  g_slist_free_full (kanjiDic->kanji_history_list, free);

  // don't free elements: contain information of rad_info
  if (kanjiDic->rad_button_hash)
    g_hash_table_destroy (kanjiDic->rad_button_hash);

}



void
kanjidic_close()
{
  GJITEN_DEBUG ("KANJIDIC_CLOSE\n");
  if (kanjiDic != NULL)
  {
    // destroy radical selection window, if it's still open
    if (kanjiDic->window_radicals)
      gtk_widget_destroy (kanjiDic->window_radicals);

    kanjidic_destruct ();

    kanjiDic = NULL;
    self = NULL;
    gjitenApp->kanjidic = NULL;
  }
  gjiten_quit_if_all_windows_closed ();
}



void
shade_kanjidic_widgets()
{
  gboolean active = FALSE;

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (kanjiDic->checkb_stroke));
  gtk_widget_set_sensitive (GTK_WIDGET (kanjiDic->spinb_strokenum), active);
  gtk_widget_set_sensitive (GTK_WIDGET (kanjiDic->spinb_plusmin), active);
  gtk_widget_set_sensitive (GTK_WIDGET (kanjiDic->label_plusmin), active);

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (kanjiDic->checkb_radical));
  gtk_widget_set_sensitive (GTK_WIDGET (kanjiDic->button_radtable), active);
  gtk_widget_set_sensitive (GTK_WIDGET (kanjiDic->combo_entry_radical), active);

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (kanjiDic->checkb_ksearch));
  gtk_widget_set_sensitive (GTK_WIDGET (kanjiDic->combo_entry_key), active);
}


void
history_init()
{
  GSList *tmp_list_ptr = NULL;
  GtkWidget *history_kanji_button;
  GtkWidget *history_kanji_label;
  gchar history_kanji_labelstr[100];
  gchar kanji[6];
  int i;

  tmp_list_ptr = kanjiDic->kanji_history_list;

  while (tmp_list_ptr != NULL) {
    for (i = 0; i < 6; i++) kanji[i] = 0;
    g_unichar_to_utf8((*(gunichar *)tmp_list_ptr->data), kanji);

    if (gjitenApp->conf->bigkanji == TRUE) {
      g_snprintf (history_kanji_labelstr, 100, "<span size=\"large\">%s</span>", kanji);
      history_kanji_label = gtk_label_new (NULL);
      gtk_label_set_markup (GTK_LABEL (history_kanji_label), history_kanji_labelstr);
      gtk_widget_show (history_kanji_label);
      history_kanji_button = gtk_button_new ();
      gtk_container_add (GTK_CONTAINER (history_kanji_button), history_kanji_label);
    }
    else {
      history_kanji_label = gtk_label_new (kanji);
      gtk_widget_show (history_kanji_label);
      history_kanji_button = gtk_button_new ();
      gtk_container_add (GTK_CONTAINER (history_kanji_button), history_kanji_label);
    }
    gtk_widget_style_add_class (history_kanji_label, "normalfont");

    g_signal_connect_swapped (G_OBJECT (history_kanji_button), "clicked", G_CALLBACK (kanji_selected),
           (gpointer)(tmp_list_ptr->data));
    gtk_box_pack_start (GTK_BOX (kanjiDic->vbox_history), history_kanji_button, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (kanjiDic->vbox_history), history_kanji_button, 0);
    gtk_widget_show (history_kanji_button);
    if (gtk_widget_get_visible (kanjiDic->scrolledwin_history) != TRUE)
      gtk_widget_show (kanjiDic->scrolledwin_history);
    tmp_list_ptr = g_slist_next (tmp_list_ptr);
  }
}

void
kanjidic_apply_fonts()
{
  if (kanjiDic == NULL) return;

  if ((gjitenApp->conf->largefont == NULL) || (strlen (gjitenApp->conf->largefont) == 0)) {
    if (kanjiDic->tag_large_font != NULL) {
      g_object_set (kanjiDic->tag_large_font, "size", 20 * PANGO_SCALE, NULL);
    }
    else {
      kanjiDic->tag_large_font = gtk_text_buffer_create_tag (kanjiDic->text_kanjinfo_buffer, "largefont", "size", 20 * PANGO_SCALE, NULL);
    }
  }
  else {
    if (kanjiDic->tag_large_font != NULL) {
      g_object_set (kanjiDic->tag_large_font, "font", gjitenApp->conf->largefont, NULL);
    }
     else {
      kanjiDic->tag_large_font = gtk_text_buffer_create_tag (kanjiDic->text_kanjinfo_buffer, "largefont", "font", gjitenApp->conf->largefont, NULL);
    }
  }
}

void
clear_radical_entry_box(gpointer entrybox)
{
  gtk_entry_set_text (GTK_ENTRY (entrybox), "");
  if (GTK_IS_WIDGET (kanjiDic->window_radicals)) {
    g_hash_table_foreach (kanjiDic->rad_button_hash, (GHFunc) set_radical_button_unsensitive, (gpointer) TRUE);
  }
}


GjKanjidicWindow *
kanjidic_create()
{
  //TODO:improve: remove function and use gj_worddic_window_new instead
  GtkApplication * app;
  app = GTK_APPLICATION (g_application_get_default ());

  if (NULL == self)
  {
    gj_kanjidic_window_new (app);
  }

  return self;
}

static void
_create_gui(GjKanjidicWindow* self)
{
  GtkWidget *vbox_maink;
  GtkWidget *hbox_spinb;
  GtkWidget *table_koptions;
  GtkContainer *toolbar;
  GtkWidget *frame_koptions;
  GtkAdjustment *spinb_strokenum_adj;
  GtkAdjustment *spinb_plusmin_adj;
  GtkWidget *hseparator;
  GtkWidget *frame_kresults;
  GtkWidget *scrolledwin_kresults;
  GtkWidget *scrolledwin_kinfo;
  GtkWidget *hbox;
  GtkWidget *frame_kinfo;
  GtkWidget *tmpimage;
  GtkWidget *vpane;

  GjKanjidicWindowPrivate * kanjiDic = gj_kanjidic_window_get_instance_private (self);

  gtk_window_set_title (GTK_WINDOW (self), APPLICATION_NAME " - KanjiDic");
  gtk_widget_get_can_default (GTK_WIDGET (self));
  g_signal_connect (G_OBJECT (self), "destroy", G_CALLBACK (kanjidic_close), NULL);
  gtk_window_set_default_size (GTK_WINDOW (self), 500, 500);

  gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self),
                                           gjitenApp->conf->menubar);


  vbox_maink = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (self), vbox_maink);

  if (gjitenApp->conf->toolbar) {
    toolbar = GTK_CONTAINER (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10));


    gtk_container_add (GTK_CONTAINER (vbox_maink), GTK_WIDGET (toolbar));

    tmpimage = gj_toolbutton_image_new_from_icon_name ("worddic-symbolic");
    gj_container_append_item (GTK_CONTAINER (toolbar), _("WordDic"),
                                             _("Launch WordDic"), "WordDic", tmpimage,
                                             G_CALLBACK (gjiten_start_worddic), GTK_APPLICATION (g_application_get_default ()));

    tmpimage = gj_toolbutton_image_new_from_icon_name ("kanjipad-symbolic");
    gj_container_append_item (GTK_CONTAINER (toolbar), _("KanjiPad"),
                _("Launch KanjiPad"), "KanjiPad", tmpimage,
                 G_CALLBACK (gjiten_start_kanjipad), NULL);
  }

  frame_koptions = gtk_frame_new (_("Kanji Search Options"));
  gtkx_widget_css_class_add (frame_koptions, "frame_with_space");
  gtk_box_pack_start (GTK_BOX (vbox_maink), frame_koptions, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_koptions), 2);

  table_koptions = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (table_koptions), 6);
  gtk_grid_set_column_spacing (GTK_GRID (table_koptions), 3);
  gtk_container_add (GTK_CONTAINER (frame_koptions), table_koptions);

  kanjiDic->checkb_stroke = gtk_check_button_new_with_mnemonic (_("Search By _Strokes:"));
  gtk_grid_attach (GTK_GRID (table_koptions), kanjiDic->checkb_stroke, 0, 0, 1, 1);
  g_signal_connect (G_OBJECT (kanjiDic->checkb_stroke), "toggled",
                   G_CALLBACK (shade_kanjidic_widgets), NULL);

  kanjiDic->checkb_radical = gtk_check_button_new_with_mnemonic (_("Search By _Radical:"));
  gtk_grid_attach (GTK_GRID (table_koptions), kanjiDic->checkb_radical, 0, 1, 1, 1);
  g_signal_connect (G_OBJECT (kanjiDic->checkb_radical), "toggled",
          G_CALLBACK (shade_kanjidic_widgets), NULL);

  kanjiDic->checkb_ksearch = gtk_check_button_new_with_mnemonic (_("Search By _Key:"));
  gtk_grid_attach (GTK_GRID (table_koptions), kanjiDic->checkb_ksearch, 0, 2, 1, 1);
  g_signal_connect (G_OBJECT (kanjiDic->checkb_ksearch), "toggled",
          G_CALLBACK (shade_kanjidic_widgets), NULL);
  gtk_widget_set_tooltip_text (GTK_WIDGET (kanjiDic->checkb_ksearch), TOOLTIP_SEARCH_BY_KEY);

  hbox_spinb = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_grid_attach (GTK_GRID (table_koptions), hbox_spinb, 1, 0, 1, 1);


  spinb_strokenum_adj = gtk_adjustment_new (1, 1, 30, 1, 2, 0);
  kanjiDic->spinb_strokenum = gtk_spin_button_new (GTK_ADJUSTMENT (spinb_strokenum_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox_spinb), kanjiDic->spinb_strokenum, FALSE, FALSE, 0);

  kanjiDic->label_plusmin = gtk_label_new ("+/-");
  gtk_box_pack_start (GTK_BOX (hbox_spinb), kanjiDic->label_plusmin, FALSE, FALSE, 0);

  spinb_plusmin_adj = gtk_adjustment_new (0, 0, 10, 1, 10, 0);
  kanjiDic->spinb_plusmin = gtk_spin_button_new (GTK_ADJUSTMENT (spinb_plusmin_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox_spinb), kanjiDic->spinb_plusmin, FALSE, FALSE, 0);

  kanjiDic->button_radtable = gtk_button_new_with_mnemonic (_("Radica_ls"));
  {
    gtk_grid_attach (GTK_GRID (table_koptions), kanjiDic->button_radtable, 2, 1, 1, 1);
    g_signal_connect (G_OBJECT (kanjiDic->button_radtable), "clicked",
            G_CALLBACK (show_window_radicals), NULL);
  }

  kanjiDic->combo_entry_radical = createStringComboBox ();
  {
    kanjiDic->combo_entry_radical_list = GTK_LIST_STORE (gtk_combo_box_get_model (kanjiDic->combo_entry_radical));

    gtk_widget_style_add_class (gtk_bin_get_child (GTK_BIN (kanjiDic->combo_entry_radical)), "normalfont");
    gtk_grid_attach (GTK_GRID (table_koptions),
                    GTK_WIDGET (kanjiDic->combo_entry_radical), 1, 1, 1, 1);
    g_signal_connect (G_OBJECT (gtk_bin_get_child (GTK_BIN (kanjiDic->combo_entry_radical))),
            "activate", G_CALLBACK (on_kanji_search), NULL);
  }

  kanjiDic->combo_entry_key = createStringComboBox ();
  {
    kanjiDic->combo_entry_key_list = GTK_LIST_STORE (gtk_combo_box_get_model (kanjiDic->combo_entry_key));

    gtk_widget_style_add_class (GTK_WIDGET (gtk_bin_get_child (GTK_BIN (kanjiDic->combo_entry_key))), "normalfont");
    gtk_grid_attach (GTK_GRID (table_koptions),
                    GTK_WIDGET (kanjiDic->combo_entry_key),
                    1, 2, 1, 1);
    g_signal_connect (G_OBJECT ( (gtk_bin_get_child (GTK_BIN (kanjiDic->combo_entry_key)))),
            "activate", G_CALLBACK (on_kanji_search), NULL);
    gtk_widget_set_tooltip_text (GTK_WIDGET (kanjiDic->combo_entry_key), TOOLTIP_SEARCH_BY_KEY);
  }

  frame_kresults = gtk_frame_new (_("Search Results :"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame_kresults), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (frame_kresults), 2);

  scrolledwin_kresults = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwin_kresults),
         GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwin_kresults), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame_kresults), scrolledwin_kresults);

  kanjiDic->kanji_results_view = gtk_text_view_new ();
  {
    gtk_widget_style_add_class (kanjiDic->kanji_results_view, "normalfont");
    kanjiDic->kanji_results_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (kanjiDic->kanji_results_view));
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (kanjiDic->kanji_results_view), GTK_WRAP_CHAR);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (kanjiDic->kanji_results_view), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (kanjiDic->kanji_results_view), FALSE);
    gtk_container_add (GTK_CONTAINER (scrolledwin_kresults), kanjiDic->kanji_results_view);
    gtk_widget_set_size_request (kanjiDic->kanji_results_view, -1, 66);
  }

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  kanjiDic->text_kanjinfo_view = gtk_text_view_new ();
  {
    gtk_widget_style_add_class (kanjiDic->text_kanjinfo_view, "normalfont");
    kanjiDic->text_kanjinfo_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (kanjiDic->text_kanjinfo_view));
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (kanjiDic->text_kanjinfo_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (kanjiDic->text_kanjinfo_view), FALSE);
  }

  frame_kinfo = gtk_frame_new (_("Kanji Info :"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame_kinfo), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (frame_kinfo), 2);
  gtk_container_add (GTK_CONTAINER (frame_kinfo), hbox);

  scrolledwin_kinfo = gtk_scrolled_window_new (NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwin_kinfo),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwin_kinfo), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), scrolledwin_kinfo, TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (scrolledwin_kinfo), kanjiDic->text_kanjinfo_view);

  kanjiDic->vbox_history = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  history_init ();

  kanjiDic->scrolledwin_history = gtkx_scrollable_window_vertical_new (kanjiDic->vbox_history,NULL);
  gtkx_widget_css_class_add (kanjiDic->scrolledwin_history, "minislider");
  gtk_box_pack_start (GTK_BOX (hbox), kanjiDic->scrolledwin_history, FALSE, TRUE, 0);

  vpane = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  gtk_paned_add1(GTK_PANED (vpane), frame_kresults);
  gtk_paned_add2(GTK_PANED (vpane), frame_kinfo);
  gtk_box_pack_start (GTK_BOX (vbox_maink), vpane, TRUE, TRUE, 0);

  kanjiDic->appbar_kanji = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (vbox_maink), kanjiDic->appbar_kanji, FALSE, FALSE, 0);

  shade_kanjidic_widgets ();

  g_signal_connect (G_OBJECT (GTK_ADJUSTMENT (spinb_strokenum_adj)),
          "value_changed", G_CALLBACK (on_kanji_search), NULL);
  g_signal_connect (G_OBJECT (GTK_ADJUSTMENT (spinb_plusmin_adj)),
          "value_changed", G_CALLBACK (on_kanji_search), NULL);

  gtk_text_buffer_create_tag (kanjiDic->text_kanjinfo_buffer, "blue_foreground", "foreground", "blue", NULL);

  kanjidic_apply_fonts ();

  g_menu_setup_default_actions_copy_paste (GTK_WINDOW (self));

  gtk_widget_show_all (GTK_WIDGET (self));
}



static void
gj_kanjidic_window_class_init (GjKanjidicWindowClass* klass)
{
}



static void
gj_kanjidic_window_init (GjKanjidicWindow* self)
{
  // init variables

  // init private variables:
  // GjKanjidicWindowPrivate * priv = gj_kanjidic_window_get_instance_private(self);
}



GtkWidget*
gj_kanjidic_window_new (GtkApplication * app)
{
  // for now we must make it Singleton here in ctor, because
  //  calls within _create_gui expect it to be.
  self = GJ_KANJIDIC_WINDOW ((g_object_new (gj_kanjidic_window_get_type(), "application", app, NULL)));

  kanjiDic = gj_kanjidic_window_get_instance_private (self);
  _create_gui (GJ_KANJIDIC_WINDOW (self));
  gj_window_set_icon_default (GTK_WINDOW (self));

  // Kanjidic uses int <-> uintptr_t <-> void* conversions.
  // In case a system would not be able to handle it, let
  // the user know about.
  if (sizeof (gunichar) > sizeof (uintptr_t) ||
      sizeof (gunichar) > sizeof (gpointer))
  {
    error_show (GTK_WINDOW (self), _("Kanjidict is incompatible with your system. It could work, but it could also not work. \n\n Technical Details:\n uintptr_t and void* are smaller than integers."));
  }


  return GTK_WIDGET (self);
}
