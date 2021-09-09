/* -*- Mode: C; tab-width: 2;   indent-tabs-mode: space; c-basic-offset: 2 -*- */
/* vi: set ts=2 sw=2: */
/* worddic.c

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

/* This file contains the GUI part for WordDic */


#include "utils.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#include "worddic.h"
#include "constants.h"
#include "conf.h"
#include "dicfile.h"
#include "gjiten.h"
#include "dicutil.h"
//#include "pref.h"
#include "error.h"


typedef struct _GjWorddicWindowPrivate GjWorddicWindowPrivate;
struct _GjWorddicWindowPrivate
{
  GtkWidget       *hbox_options;
  GtkComboBox     *cbo_search_term;
  GtkWidget       *text_results_view;
  GtkTextBuffer   *text_results_buffer;
  GtkTextBuffer   *info_buffer;
  GtkWidget       *checkb_verb;
  GtkToolButton   *btn_options_show_hide;
  GtkWidget       *checkb_autoadjust;
  GtkWidget       *checkb_searchlimit;
  GtkWidget       *spinb_searchlimit;
  GtkWidget       *radiob_jpexact;
  GtkWidget       *radiob_startw;
  GtkWidget       *radiob_endw;
  GtkWidget       *radiob_any;
  GtkWidget       *radiob_engexact;
  GtkWidget       *radiob_words;
  GtkWidget       *radiob_partial;
  GtkWidget       *radiob_searchdic;
  GtkWidget       *radiob_searchall;
  GtkToolButton   *button_back;
  GtkToolButton   *button_forward;
  GtkTextIter      iter;
  GtkWidget       *appbar_mainwin;
  GtkListStore    *word_search_history_model;
  GtkComboBoxText *dicselection_menu;
  GtkTextTag      *tag_large_font;
  GdkCursor       *selection_cursor;
  GdkCursor       *regular_cursor;
  gboolean         is_cursor_regular;
};
G_DEFINE_TYPE_WITH_PRIVATE (GjWorddicWindow, gj_worddic_window,  GTK_TYPE_APPLICATION_WINDOW)

static void worddic_copy();
static void print_result(gchar *txt2print, int result_offset, gchar *searchstrg);
static void worddic_destroy_window();

extern GtkWidget *window_kanjidic;
extern GtkWidget *dialog_preferences;
extern GjitenConfig conf;
extern GjitenApp *gjitenApp;

static GjWorddicWindow *self = NULL;
static GjWorddicWindowPrivate *wordDic = NULL;

int word_matches;
gchar *vconj_types[40];
GSList *vinfl_list = NULL;
guint32 srchpos;
int engsrch, jpsrch;
int dicname_printed;
int append_to_history = TRUE;
gpointer current_glist_word = NULL;
// probably replace with `gtk_combo_box_get_active (wordDic->cbo_search_term)`
gint current_history_word_index = -1;



static void
worddic_copy()
{
  GtkWidget * focus = gtk_window_get_focus (GTK_WINDOW (self));
  if (focus){
    if (GTK_IS_EDITABLE (focus) || GTK_IS_TEXT_VIEW (focus))
    {
      g_signal_emit_by_name (focus, "copy-clipboard", NULL);
    }
  }
}



void
worddic_paste()
{
  gchar *selection = NULL;

  // First try the current selection
  selection = gtk_clipboard_wait_for_text (gtk_clipboard_get (GDK_SELECTION_PRIMARY));
  if (selection != NULL) gtk_combo_box_set_text (wordDic->cbo_search_term, selection);
  else {
    // if we didn't get anything, try the default clipboard
    selection = gtk_clipboard_wait_for_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
      if (selection != NULL) gtk_combo_box_set_text (wordDic->cbo_search_term, selection);
  }
}



// Load & initialize verb inflection details
static void
Verbinit()
{
  static int verbinit_done = FALSE;
  gchar *tmp_ptr;
  int vinfl_size = 0;
  struct stat vinfl_stat;
  gchar *vinfl_start, *vinfl_ptr, *vinfl_end;
  int fd = 0;
  int vinfl_part = 1;
  int conj_type = 40;
  struct vinfl_struct *tmp_vinfl_struct;
  GSList *tmp_list_ptr = NULL;

  if (verbinit_done == TRUE)
  {
    //printf ("Verbinit already done!\n");
    return;
  }

  if (stat (VINFL_FILENAME, &vinfl_stat) != 0)
  {
    printf ("**ERROR** %s: stat () \n", VINFL_FILENAME);
  }
  vinfl_size = vinfl_stat.st_size;
  fd = open (VINFL_FILENAME, O_RDONLY);
  if (fd == -1)
  {
    printf ("**ERROR** %s: open ()\n", VINFL_FILENAME);
  }
  // printf ("SIZE: %d\n", radkfile_size);
  vinfl_start = (gchar *) mmap (NULL, vinfl_size, PROT_READ, MAP_SHARED, fd, 0);
  if (vinfl_start == MAP_FAILED) error_show_and_quit ("mmap () failed for "VINFL_FILENAME"\n");

  //  printf ("STRLEN: %d\n", strlen (radkfile));

  vinfl_end = vinfl_start + strlen (vinfl_start);
  vinfl_ptr = vinfl_start;

  vinfl_part = 1;
  while ((vinfl_ptr < vinfl_end) && (vinfl_ptr != NULL))
  {
    if (*vinfl_ptr == '#') //find $ as first char on the line
    {
      vinfl_ptr = get_eof_line (vinfl_ptr, vinfl_end); //Goto next line
      continue;
    }
    if (*vinfl_ptr == '$') vinfl_part = 2;

    switch (vinfl_part)
    {
    case 1:
      if (g_ascii_isdigit (*vinfl_ptr) == TRUE)//Conjugation numbers
      {
        conj_type = atoi (vinfl_ptr);
        if ((conj_type < 0) || (conj_type > 39)) break;
        while (g_ascii_isdigit (*vinfl_ptr) == TRUE) vinfl_ptr = g_utf8_next_char (vinfl_ptr); //skip the number
        while (g_ascii_isspace (*vinfl_ptr) == TRUE) vinfl_ptr = g_utf8_next_char (vinfl_ptr); //skip the space
        tmp_ptr = vinfl_ptr; // beginning of conju  gation definition;
        vinfl_ptr = get_eof_line (vinfl_ptr, vinfl_end); //  find end of line
        vconj_types[conj_type] = g_strndup (tmp_ptr, vinfl_ptr - tmp_ptr -1);
        //printf ("%d : %s\n", conj_type, vconj_types[conj_  type]);
      }
      break;
    case 2:
      if (g_unichar_iswide (g_utf8_get_char (vinfl_ptr)) == FALSE)
      {
        vinfl_ptr =  get_eof_line (vinfl_ptr, vinfl_end);
        break;
      }
      tmp_vinfl_struct = (struct vinfl_struct *) malloc (sizeof (struct vinfl_struct));
      tmp_ptr = vinfl_ptr;
      while (g_unichar_iswide (g_utf8_get_char (vinfl_ptr)) == TRUE) {
        vinfl_ptr = g_utf8_next_char (vinfl_ptr); //skip the conjugation
      }
      tmp_vinfl_struct->conj = g_strndup (tmp_ptr, vinfl_ptr - tmp_ptr); //store the conjugation
      while (g_ascii_isspace (*vinfl_ptr) == TRUE) {
        vinfl_ptr = g_utf8_next_char (vinfl_ptr); //skip the space
      }
      tmp_ptr = vinfl_ptr;
      while (g_unichar_iswide (g_utf8_get_char (vinfl_ptr)) == TRUE) {
        vinfl_ptr = g_utf8_next_char (vinfl_ptr); //skip the inflection
      }
      tmp_vinfl_struct->infl = g_strndup (tmp_ptr, vinfl_ptr - tmp_ptr); //store the inflection
      while (g_ascii_isspace (*vinfl_ptr) == TRUE) {
        vinfl_ptr = g_utf8_next_char (vinfl_ptr); //skip the space
      }
      tmp_vinfl_struct->type = vconj_types[atoi (vinfl_ptr)];
      vinfl_ptr =  get_eof_line (vinfl_ptr, vinfl_end);
      //printf ("%s|%s|%s\n", tmp_vinfl_struct->conj, tmp_vinfl_struct->infl, tmp_vinfl_struct->type);
      tmp_list_ptr = g_slist_append (tmp_list_ptr, tmp_vinfl_struct);
      if (vinfl_list == NULL) vinfl_list = tmp_list_ptr;
      break;
    }
  }
  verbinit_done = TRUE;
}



static inline void
print_matches_in(GjitenDicfile *dicfile)
{
  //Print dicfile name if all dictionaries are selected
  if ((dicname_printed == FALSE) && (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_searchall))))
  {
    gchar *tmpstr, *hl_start_ptr;
    gint hl_start = 0;
    gint hl_end = 0;

    tmpstr = g_strdup_printf (_("Matches in %s:\n"), dicfile->name);
    hl_start_ptr = strstr (tmpstr, dicfile->name);
    hl_start = hl_start_ptr == NULL ? 0 : hl_start_ptr - tmpstr;
    hl_end = hl_start + strlen (dicfile->name);

    if (hl_start > 0)
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter, tmpstr, hl_start);
    gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter,
                                             dicfile->name, -1, "brown_foreground", NULL);
    gtk_text_buffer_insert (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter, tmpstr + hl_end, -1);
    g_free (tmpstr);
    dicname_printed = TRUE;
  }
}



static void
print_verb_inflections(GjitenDicfile *dicfile,
                       gchar         *srchstrg)
{
  int srchresp, roff, rlen;
  guint32 oldrespos, respos;
  int gjit_search = SRCH_START;
  gchar repstr[1024];
  GSList *tmp_list_ptr;
  struct vinfl_struct *tmp_vinfl_struct;
  gchar *deinflected;
  int printit = TRUE;

  tmp_list_ptr = vinfl_list;
  if (vinfl_list == NULL)
  {
    //printf ("VINFL LIST == NULL\n");
    return;
  }

  deinflected = (gchar *) g_malloc (strlen (srchstrg) + 20);

  do {
    tmp_vinfl_struct = (struct vinfl_struct *) tmp_list_ptr->data;
    if (strg_end_compare (srchstrg, tmp_vinfl_struct->conj) == TRUE)
    {

      // create deinflected string
      strncpy (deinflected, srchstrg, strlen (srchstrg) - strlen (tmp_vinfl_struct->conj));
      strcpy (deinflected + strlen (srchstrg) - strlen (tmp_vinfl_struct->conj),
             tmp_vinfl_struct->infl);

      oldrespos = srchpos = 0;
      gjit_search = SRCH_START;
      do { // search loop
        oldrespos = respos;

        srchresp = search4string (gjit_search, dicfile, deinflected, &respos, &roff, &rlen, repstr);
        //    printf ("respos:%d, oldrespos:%d, roffset:%d, rlen:%d\nrepstr:%s\n", respos, oldrespos, roff, rlen, repstr);
        if (srchresp != SRCH_OK)  {
          break;   //No more matches
        }
        if (gjit_search == SRCH_START)
        {
          srchpos = respos;
          gjit_search = SRCH_CONT;
        }
        srchpos++;
        if (oldrespos == respos) continue;

        printit = TRUE;

        if (is_kanji_only (repstr) == TRUE)
          printit = FALSE;
        else if (strlen (tmp_vinfl_struct->conj) == strlen (srchstrg))
          printit = FALSE; // don't display if conjugation is the same length as the srchstrg
        else if (get_jp_match_type (repstr, deinflected, roff) != EXACT_MATCH)
          printit = FALSE; // Display only EXACT_MATCHes

        if (printit == TRUE)
        {
          print_matches_in (dicfile);
          gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter,
                                                   _("Possible inflected verb or adjective: "),
                                                   -1, "brown_foreground", NULL);
          gtk_text_buffer_insert (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter, tmp_vinfl_struct->type, -1);
          gtk_text_buffer_insert (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter, " " , -1);
          gtk_text_buffer_insert (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter, tmp_vinfl_struct->conj, -1);
          gtk_text_buffer_insert (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter, "\xe2\x86\x92", -1); //arrow
          gtk_text_buffer_insert (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter, tmp_vinfl_struct->infl, -1);
          gtk_text_buffer_insert (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter, "\n   ", -1);
          print_result (repstr, roff, deinflected);
          word_matches++;
        }
      } while (srchresp == SRCH_OK);
    }
  } while ((tmp_list_ptr = g_slist_next (tmp_list_ptr)) != NULL);

  g_free (deinflected);

}



static void
print_result(gchar *txt2print,
             int    result_offset,
             gchar *searchstrg)
{
  gchar *currentchar;
  gchar *kana_start;
  gchar *exp_start;
  GtkTextMark *linestart;
  GtkTextIter startmatch, endmatch;
  // for readability
  GtkTextBuffer * text_buffer_results = GTK_TEXT_BUFFER (wordDic->text_results_buffer);

  linestart = gtk_text_buffer_create_mark (text_buffer_results,"linestart",
                                          &wordDic->iter, TRUE);

  currentchar = txt2print;

  // find end of [KANJI]
  while (!((*currentchar == '[') || (*currentchar == '/')))
  {
    if ((size_t) (currentchar - txt2print) >= strlen (txt2print)) break;
    currentchar = g_utf8_next_char (currentchar);
  }
  currentchar = g_utf8_prev_char (currentchar); // go back to the space


  //print out japanese word ([KANJI])
  if (gjitenApp->conf->bigwords == FALSE)
  {
    gtk_text_buffer_insert (text_buffer_results, &wordDic->iter,
                          txt2print, currentchar - txt2print);
  }
  else {
    gtk_text_buffer_insert_with_tags_by_name (text_buffer_results, &wordDic->iter,
                                             txt2print, currentchar - txt2print, "largefont", NULL);
  }

  currentchar = g_utf8_next_char (currentchar);
  // Do we have a kana reading?
  if (*(currentchar) == '[')
  {
    gtk_text_buffer_insert (text_buffer_results, &wordDic->iter, " (", 2);
    currentchar = kana_start = g_utf8_next_char (currentchar);
    // find end of kana reading ( ']' )
    while (*(currentchar) != ']'){
      currentchar = g_utf8_next_char (currentchar);
    }
    // print out kana reading
    gtk_text_buffer_insert (text_buffer_results, &wordDic->iter,
                          kana_start, currentchar - kana_start);
    gtk_text_buffer_insert (text_buffer_results, &wordDic->iter, ") ", 2);
    currentchar += 3;
  }
  else
  {
    // No kana reading, just insert [space]
    gtk_text_buffer_insert (text_buffer_results, &wordDic->iter, " ", 1);
    currentchar++;
  }

  // print out the rest of the line
  while (currentchar < txt2print + strlen (txt2print))
  {
    if (*currentchar == '\n') break;
    exp_start = currentchar;
    while (!((*currentchar == '/') || (*currentchar == '\n'))) {
      currentchar = g_utf8_next_char (currentchar);
    }
    if (*currentchar == '\n') break;
    //print out expression
    gtk_text_buffer_insert (text_buffer_results, &wordDic->iter, exp_start,
                          currentchar - exp_start);
    gtk_text_buffer_insert (text_buffer_results, &wordDic->iter, "; ", 2);
    currentchar = g_utf8_next_char (currentchar);
  }

  // insert linebreak
  gtk_text_buffer_insert (text_buffer_results,&wordDic->iter, "\n", 1);

  // print a little distance between results
  gtk_text_buffer_insert_with_tags_by_name (text_buffer_results, &wordDic->iter,
                                           "\n",1, "small_distance", NULL);

  //find searchstrg matches in the line. we print and highlight it.
  gtk_text_buffer_get_iter_at_mark (text_buffer_results, &endmatch, linestart);

  while (gtk_text_iter_forward_search (&endmatch, searchstrg, 0,
                                      &startmatch, &endmatch, &wordDic->iter) == TRUE)
  {
    gtk_text_buffer_apply_tag_by_name (text_buffer_results, "blue_foreground",
                                      &startmatch, &endmatch);
  }
}



static void
search_in_dictfile_and_print(GjitenDicfile *dicfile,
                             gchar         *srchstrg)
{
  gint srchresp, roff, rlen;
  gchar repstr[1024];
  guint32 respos, oldrespos;
  gint printit;
  gint match_criteria = EXACT_MATCH;
  gchar *currchar;
  gint match_type = ANY_MATCH;
  gint gjit_search = SRCH_START;

  //Detect Japanese
  engsrch = TRUE;
  jpsrch = FALSE;
  currchar = srchstrg;
  do {
    if (g_unichar_iswide (g_utf8_get_char (currchar)) == TRUE) //FIXME: this doesn't detect all Japanese
    {
      engsrch = FALSE;
      jpsrch = TRUE;
      break;
    }
  } while ((currchar = g_utf8_find_next_char (currchar, srchstrg + strlen (srchstrg))) != NULL);

  // Verb deinfelction
  if (gjitenApp->conf->verb_deinflection == TRUE) print_verb_inflections (dicfile, srchstrg);

  if (jpsrch == TRUE)
  {
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_jpexact))) match_criteria = EXACT_MATCH;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_startw))) match_criteria = START_WITH_MATCH;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_endw))) match_criteria = END_WITH_MATCH;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_any))) match_criteria = ANY_MATCH;
  }
  else
  {
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_engexact))) match_criteria = EXACT_MATCH;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_words))) match_criteria = WORD_MATCH;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_partial))) match_criteria = ANY_MATCH;
  }

  oldrespos = srchpos = 0;

  do
  { // search loop
    oldrespos = respos;

    srchresp = search4string (gjit_search, dicfile, srchstrg, &respos, &roff, &rlen, repstr);

    if (srchresp != SRCH_OK) {
      return;   //No more matches
    }

    if (gjit_search == SRCH_START)
    {
      srchpos = respos;
      gjit_search = SRCH_CONT;
    }

    srchpos++;
    if (oldrespos == respos) continue;

    // Check match type and search options
    printit = FALSE;
    if (jpsrch)
    {
      match_type = get_jp_match_type (repstr, srchstrg, roff);
      switch (match_criteria)
      {
      case EXACT_MATCH :
        if (match_type == EXACT_MATCH) printit = TRUE;
        break;
      case START_WITH_MATCH:
        if ((match_type == START_WITH_MATCH) || (match_type == EXACT_MATCH)) printit = TRUE;
        break;
      case END_WITH_MATCH:
        if ((match_type == END_WITH_MATCH) || (match_type == EXACT_MATCH)) printit = TRUE;
        break;
      case ANY_MATCH:
        printit = TRUE;
        break;
      }
    }
    else
    { //Non-japanese search
      switch (match_criteria)
      {
      case EXACT_MATCH:
        //Must lie between two '/' delimiters
        if ((repstr[roff - 1] == '/') && (repstr[roff + strlen (srchstrg)] == '/')) printit = TRUE;
        //take "/(n) expression/" into accont
        else if ((repstr[roff - 2] == ')') && (repstr[roff + strlen (srchstrg)] == '/')) printit = TRUE;
        //also match verbs starting with a 'to'. eg: "/(n) to do_something/"
        else if ((repstr[roff - 2] == 'o') && (repstr[roff - 3] == 't') && (repstr[roff + strlen (srchstrg)] == '/')
                 && ((repstr[roff - 5] == ')') || (repstr[roff - 4] == '/'))) printit = TRUE;
        break;
      case WORD_MATCH:
        if ((g_unichar_isalpha (g_utf8_get_char (repstr + roff + strlen (srchstrg))) == FALSE)  &&
            (g_unichar_isalpha (g_utf8_get_char (repstr + roff - 1)) == FALSE))
        {

          //  printf ("---------\n");
          //  printf ("%s", repstr);
          //  if (g_unichar_isalpha (g_utf8_get_char (repstr + roff - 1)) == FALSE)  printf ("beg:%s", repstr + roff - 1);
          //  if (g_unichar_isalpha (g_utf8_get_char (repstr + roff + strlen (srchstrg))) == FALSE)
          //  printf ("end:%s", repstr + roff + strlen (srchstrg));
          //  printf ("---------\n");

          printit = TRUE;
        }
        break;
      case ANY_MATCH:
        printit = TRUE;
        break;
      }
    }

    if (printit)
    {

      //printf ("offset: %d: ", roff);
      //printf ("jptype: %d\n", match_type);
      //printf ("criteria: %d\n", match_criteria);


      print_matches_in (dicfile);
      print_result (repstr, roff, srchstrg);
      //get_kanji_and_reading (repstr); FIXME
      word_matches++;
    }
    if ((gjitenApp->conf->searchlimit_enabled == TRUE) && (word_matches >= gjitenApp->conf->maxwordmatches))
    {
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (wordDic->text_results_buffer), &wordDic->iter, _("Results truncated"), -1, "red_foreground", NULL);
      return;
    }
  } while (srchresp == SRCH_OK);
}



int
lower_search_option()
{
  if (!(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->checkb_autoadjust))))
    return FALSE;
  if (jpsrch) //Japanese string
  {
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_any)))
      return FALSE;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_jpexact)))
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wordDic->radiob_startw), TRUE);
      return TRUE;
    }
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_startw)))
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wordDic->radiob_endw), TRUE);
      return TRUE;
    }
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_endw)))
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wordDic->radiob_any), TRUE);
      return TRUE;
    }
  }
  else if (engsrch)//English
  {
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_partial)))
      return FALSE;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_engexact)))
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wordDic->radiob_words), TRUE);
      return TRUE;
    }
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_words)))
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wordDic->radiob_partial), TRUE);
      return TRUE;
    }
  }
  return FALSE;
}



static void
worddic_hira_kata_search(GjitenDicfile *dicfile,
                         gchar         *srchstrg)
{
  gchar *hirakata;
  if (gjitenApp->conf->search_kata_on_hira)
  {
    if (isKatakanaString (srchstrg) == TRUE)
    {
      hirakata = kata2hira (srchstrg);
      search_in_dictfile_and_print (dicfile, hirakata);
      g_free (hirakata);
    }
  }
  if (gjitenApp->conf->search_hira_on_kata)
  {
    if (isHiraganaString (srchstrg) == TRUE)
    {
      hirakata = hira2kata (srchstrg);
      search_in_dictfile_and_print (dicfile, hirakata);
      g_free (hirakata);
    }
  }
}



static void
worddic_search(gchar *srchstrg)
{
  const int LEN = 50;
  gchar appbarmsg[LEN];
  int truncated;
  GjitenDicfile *dicfile;
  GSList *dicfile_node;

  word_matches = 0;

  if (gjitenApp->conf->dicfile_list == NULL)
  {
    snprintf (appbarmsg, LEN, _("No dicfiles specified! Set your preferences first."));
    gtk_label_set_text (GTK_LABEL (wordDic->appbar_mainwin),appbarmsg);
    return;
  }

  // remove leading and trailing spaces
  while (g_ascii_isspace (srchstrg[0])) srchstrg++;
  while (g_ascii_isspace (srchstrg[strlen (srchstrg)-1]) != 0) srchstrg[strlen (srchstrg)-1] = 0;

  if (strlen (srchstrg) == 0) return;
  gtk_combo_box_set_text (wordDic->cbo_search_term, srchstrg);

  truncated = 0;
  while (TRUE)
  {
    // search in all dictionaries
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_searchall)))
    {
      dicfile_node = gjitenApp->conf->dicfile_list;
      while (dicfile_node != NULL)
      {
        if (dicfile_node->data != NULL)
        {
          dicfile = dicfile_node->data;
          dicname_printed = FALSE;
          search_in_dictfile_and_print (dicfile, srchstrg);
          worddic_hira_kata_search (dicfile, srchstrg);
          if ((gjitenApp->conf->searchlimit_enabled == TRUE) && (word_matches >= gjitenApp->conf->maxwordmatches)) break;
        }
        dicfile_node = g_slist_next (dicfile_node);
      }
    }
    // search only in selected dictionary
    else
    {
      search_in_dictfile_and_print (gjitenApp->conf->selected_dic, srchstrg);
      worddic_hira_kata_search (gjitenApp->conf->selected_dic, srchstrg);
    }

    if ((gjitenApp->conf->searchlimit_enabled == TRUE) && (word_matches >= gjitenApp->conf->maxwordmatches)) truncated = 1;
    if (word_matches > 0) break;  // No need to search anymore
    if (lower_search_option () == FALSE) break;
  }

  if (word_matches)
  {
    if (truncated) snprintf (appbarmsg, LEN, _("Matches found (truncated): %d"), word_matches);
    else snprintf (appbarmsg, LEN, _("Matches found: %d"), word_matches);
    gtk_label_set_text (GTK_LABEL (wordDic->appbar_mainwin), appbarmsg);
  }
  else gtk_label_set_text (GTK_LABEL (wordDic->appbar_mainwin), _("No match found."));
}



static void
_button_back_maybe_activate()
{
  gint curSelected = gtk_combo_box_get_active (GTK_COMBO_BOX (wordDic->cbo_search_term));
  gint numElements = gtk_combo_box_length (GTK_COMBO_BOX (wordDic->cbo_search_term));

  if (curSelected < numElements-1 ||
      curSelected == -1)
  {
    gtk_widget_set_sensitive (GTK_WIDGET (wordDic->button_back), TRUE);
  }
  else
  {
    gtk_widget_set_sensitive (GTK_WIDGET (wordDic->button_back), FALSE);
  }
  return;
}



static void
_button_next_maybe_activate()
{
  gint curSelected = gtk_combo_box_get_active (GTK_COMBO_BOX (wordDic->cbo_search_term));
  if (curSelected > 0)
  {
    gtk_widget_set_sensitive (GTK_WIDGET (wordDic->button_forward), TRUE);
  }
  else
  {
    gtk_widget_set_sensitive (GTK_WIDGET (wordDic->button_forward), FALSE);
  }
}



void
on_search_clicked()
{
  static gchar *new_entry_text = NULL;

  gdk_window_set_cursor (gtk_text_view_get_window (GTK_TEXT_VIEW (wordDic->text_results_view), GTK_TEXT_WINDOW_TEXT), wordDic->regular_cursor);
  wordDic->is_cursor_regular = TRUE;

  new_entry_text = g_strdup (gtk_combo_box_get_text (wordDic->cbo_search_term));
  if (gx_utf8_validate (new_entry_text, -1, NULL) == FALSE)
{
    error_show (GTK_WINDOW (self), _("Invalid input: non-utf8\n"));
    g_free (new_entry_text);
    return;
  }
  if (strlen (new_entry_text) == 0) return;
  if (append_to_history == TRUE)
  {
      current_glist_word = new_entry_text;
      gtk_list_store_string_prepend (wordDic->word_search_history_model, new_entry_text);
  }

  _button_back_maybe_activate ();
  _button_next_maybe_activate ();

  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (wordDic->text_results_buffer), "", 0);
  gtk_text_buffer_get_start_iter (wordDic->text_results_buffer, &wordDic->iter);

  gtk_label_set_text (GTK_LABEL (wordDic->appbar_mainwin), _("Searching..."));

  worddic_search (new_entry_text);

  g_free (new_entry_text);
}



static void
on_forward_clicked()
{
  append_to_history = FALSE;
  gtk_combo_box_previous (GTK_COMBO_BOX (wordDic->cbo_search_term));
  on_search_clicked ();
  append_to_history = TRUE;
}



static void
on_back_clicked()
{
  append_to_history = FALSE;
  gtk_combo_box_next (GTK_COMBO_BOX (wordDic->cbo_search_term));
  on_search_clicked ();
  append_to_history = TRUE;
}



static void
on_dicselection_clicked(GtkWidget *widget,
                        gpointer  *null)
{
  g_return_if_fail (GTK_IS_COMBO_BOX (widget));

  // get current dictionary from combo box index
  GtkComboBox *box = GTK_COMBO_BOX (widget);
  gint active_index = gtk_combo_box_get_active (box);
  GSList * dicfile_node = g_slist_nth (gjitenApp->conf->dicfile_list, active_index);
  // set current dictionary
  if (active_index > -1)
    gjitenApp->conf->selected_dic = dicfile_node->data;
  else
    gjitenApp->conf->selected_dic = NULL;
}



static void
checkb_searchlimit_toggled()
{
  int state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->checkb_searchlimit));
  if (wordDic->spinb_searchlimit != NULL) gtk_widget_set_sensitive (wordDic->spinb_searchlimit, state);
  gjitenApp->conf->searchlimit_enabled = state;
  if (gjitenApp->conf->maxwordmatches == 0) gjitenApp->conf->searchlimit_enabled = FALSE;
}



static void
shade_worddic_widgets()
{
  if ((wordDic->dicselection_menu != NULL) && (wordDic->radiob_searchdic != NULL))
    gtk_widget_set_sensitive (GTK_WIDGET (wordDic->dicselection_menu), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->radiob_searchdic)));

  if (wordDic->checkb_autoadjust != NULL)
    gjitenApp->conf->autoadjust_enabled = (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->checkb_autoadjust)));
}



static void
get_searchlimit()
{
  gjitenApp->conf->maxwordmatches = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (wordDic->spinb_searchlimit));
  if (gjitenApp->conf->maxwordmatches == 0) gjitenApp->conf->searchlimit_enabled = FALSE;
}



/**
 * Automatically set focus to search entry if any key was pressed
 **/
static gboolean
set_focus_on_entry(GtkWidget   *window,
                   GdkEventKey *key,
                   GtkWidget   *entry)
{
  //Only set focus on the entry for real input
  if (key->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD3_MASK | GDK_MOD4_MASK)) return FALSE;
  if ((key->keyval >= GDK_KEY_exclam && key->keyval <= GDK_KEY_overline) ||
      (key->keyval >= GDK_KEY_space && key->keyval <= GDK_KEY_9))
  {
    if (gtk_widget_has_focus (entry) != TRUE)
    {
      gtk_widget_grab_focus (entry);
    }
  }
  return FALSE;
}



static void
worddic_init_history(GtkListStore *history)
{
  gint i;

  for (i = 0; i <= HISTORY_MAX_WORDS; i++)
  {
    if (gjitenApp->conf->history[i] == NULL) break;
    if (gx_utf8_validate (gjitenApp->conf->history[i], -1, NULL) == TRUE)
      gtk_list_store_string_append (history, g_strdup (gjitenApp->conf->history[i]));
    //   printf ("Read: %s: %s\n", historystr, tmpptr);
  }
}



void
worddic_close()
{
  GJITEN_DEBUG ("WORDDIC_CLOSE\n");
  if (wordDic != NULL)
  {
    gjitenconfig_save_history (wordDic->word_search_history_model, gjitenApp->conf);
    g_object_ref_sink (self);
    wordDic = NULL;
    self = NULL;
    gjitenApp->worddic = NULL;
  }
  gjiten_quit_if_all_windows_closed ();
}


static void
_btn_options_show_hide_update_ui(gboolean status)
{
  GtkToolButton * button = GTK_TOOL_BUTTON (wordDic->btn_options_show_hide);

  if (status == TRUE)
  {
    gtk_widget_set_tooltip_text (GTK_WIDGET (button), _("Hide options"));
    gtk_tool_button_set_label (button, "▲");
    gtk_tool_button_set_icon_name (button, "gj_arrow_up-symbolic");
  }
  else
  {
    gtk_widget_set_tooltip_text (GTK_WIDGET (button), _("Show options"));
    gtk_tool_button_set_label (button, "▼");
    gtk_tool_button_set_icon_name (button, "gj_arrow_down-symbolic");
  }
}


static void
worddic_show_hide_options()
{
  GJITEN_DEBUG ("worddic_show_hide_options ()\n");
  if (gtk_widget_get_visible (wordDic->hbox_options) == TRUE)
  {
    gtk_widget_hide (wordDic->hbox_options);
  }
  else
  {
    gtk_widget_show (wordDic->hbox_options);
  }
  _btn_options_show_hide_update_ui (gtk_widget_get_visible (wordDic->hbox_options));
}



void
worddic_update_dic_menu()
{
  GSList *dicfile_node;
  GtkWidget *menu_dictfiles_item;
  GjitenDicfile *dicfile;

  if (wordDic == NULL) return;

  GJITEN_DEBUG ("worddic_update_dic_menu ()\n");


  gtk_combo_box_text_remove_all (wordDic->dicselection_menu);

  dicfile_node = gjitenApp->conf->dicfile_list;
  while (dicfile_node != NULL)
  {
    if (dicfile_node->data != NULL)
    {
      dicfile = dicfile_node->data;
      gtk_combo_box_text_append_text (wordDic->dicselection_menu, dicfile->name);
    }
    else
    {
      gtk_combo_box_text_append_text (wordDic->dicselection_menu, "");
    }

    dicfile_node = g_slist_next (dicfile_node);
  }
  gtk_widget_show (GTK_WIDGET (wordDic->dicselection_menu));

  // set default selection:
  {
    gint active_index = g_slist_index (gjitenApp->conf->dicfile_list ,
                                      gjitenApp->conf->selected_dic);
    if (-1 == active_index)
    {
      GJITEN_DEBUG ("DEBUG: Dictionary combo box: No active dictionary found. Use first entry.\n");
      active_index = 0;
    }
    gtk_combo_box_set_active (GTK_COMBO_BOX (wordDic->dicselection_menu), active_index);
  }
}



void
worddic_apply_fonts()
{

  if (wordDic == NULL) return;

  if ((gjitenApp->conf->largefont == NULL) || (strlen (gjitenApp->conf->largefont) == 0))
  {
    if (wordDic->tag_large_font != NULL)
    {
      g_object_set (wordDic->tag_large_font, "size", 20 * PANGO_SCALE, NULL);
    }
    else {
      wordDic->tag_large_font = gtk_text_buffer_create_tag (wordDic->text_results_buffer, "largefont", "size", 20 * PANGO_SCALE, NULL);
    }
  }
  else {
    if (wordDic->tag_large_font != NULL)
    {
      g_object_set (wordDic->tag_large_font, "font", gjitenApp->conf->largefont, NULL);
    }
     else {
      wordDic->tag_large_font = gtk_text_buffer_create_tag (wordDic->text_results_buffer, "largefont", "font", gjitenApp->conf->largefont, NULL);
    }
  }
}



/*
 * Update the cursor image if the pointer is above a kanji.
 */
static gboolean
result_view_motion(GtkWidget      *text_view,
                   GdkEventMotion *event)
{
  gint x, y;
  GtkTextIter mouse_iter;
  gunichar kanji;
  gint trailing;

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
                                        GTK_TEXT_WINDOW_WIDGET,
                                        event->x, event->y, &x, &y);

  gtk_text_view_get_iter_at_position (GTK_TEXT_VIEW (text_view), &mouse_iter, &trailing, x , y);
  kanji = gtk_text_iter_get_char (&mouse_iter);

  // Change the cursor if necessary
  if ((unichar_isKanjiChar (kanji) == TRUE))
  {
    gdk_window_set_cursor (gtk_text_view_get_window (GTK_TEXT_VIEW (text_view), GTK_TEXT_WINDOW_TEXT), wordDic->selection_cursor);
    wordDic->is_cursor_regular = FALSE;
  }
  else if (wordDic->is_cursor_regular == FALSE)
  {
    gdk_window_set_cursor (gtk_text_view_get_window (GTK_TEXT_VIEW (text_view), GTK_TEXT_WINDOW_TEXT), wordDic->regular_cursor);
    wordDic->is_cursor_regular = TRUE;
  }

  return FALSE;
}



static gboolean
kanji_clicked(GtkWidget       *text_view,
              GdkEventButton  *event,
              gpointer         user_data)
{
  GtkTextIter mouse_iter;
  gint x, y;
  gint trailing;
  gunichar kanji;

  if (event->button != 1) return FALSE;

  if (gtk_text_buffer_get_selection_bounds (wordDic->text_results_buffer, NULL, NULL) == TRUE )
  {
    // don't look up kanji if it is in a selection
    return FALSE;
  }

  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
                                        GTK_TEXT_WINDOW_WIDGET,
                                        event->x, event->y, &x, &y);

  gtk_text_view_get_iter_at_position (GTK_TEXT_VIEW (text_view), &mouse_iter, &trailing, x, y);
  kanji = gtk_text_iter_get_char (&mouse_iter);
  if ((kanji != 0xFFFC) && (kanji != 0) && (unichar_isKanjiChar (kanji) == TRUE))
  {
    gjiten_start_kanjidic_with_search (kanji);
  }

  return FALSE;
}



static void
_search_history_buttons_set_sensitivity()
{
  _button_back_maybe_activate ();
  _button_next_maybe_activate ();
}

void
cbo_search_term_on_changed (GtkComboBox *widget,
               gpointer     user_data)
{
  // If user selected a different item
  // from the ComboBox
  gint curSelected = gtk_combo_box_get_active (GTK_COMBO_BOX (wordDic->cbo_search_term));
  if (-1 != curSelected)
    on_search_clicked();
}




GjWorddicWindow *
worddic_create()
{
  //TODO:improve: remove function and use gj_worddic_window_new instead
  GtkApplication * app;
  app = GTK_APPLICATION (g_application_get_default ());

  if (NULL == self)
  {
    gj_worddic_window_new (app);
  }

  return self;
}



static void
_create_gui (GjWorddicWindow* self)
{
  GtkWidget *vbox_main;
  GtkBox *toolbar;
  GtkWidget *button_clear;
  GtkWidget *frame_japopt;
  GtkWidget *vbox_japopt;
  GSList *vbox_japopt_group = NULL;
  GSList *dicssearch_group = NULL;
  GtkWidget *frame_engopt;
  GtkWidget *vbox_engopt;
  GSList *vbox_engopt_group = NULL;
  GtkWidget *grid;
  GtkWidget *frame_gopt;
  GtkWidget *hbox_searchlimit;
  GtkWidget *hbox_entry;
  GtkWidget *label_enter;
  GtkWidget *button_search;
  GtkWidget *frame_results;
  GtkWidget *vbox_results;
  GtkWidget *scrolledwin_results;
  GtkAdjustment *spinb_searchlimit_adj;
  GtkWidget *tmpimage;

  GjWorddicWindowPrivate * wordDic = gj_worddic_window_get_instance_private (self);

  {
    GdkPixbuf *cursor_pixbuf;
    cursor_pixbuf = gdk_pixbuf_new_from_resource (RESOURCE_PATH "cursors/left_ptr_question.png",NULL);
    wordDic->selection_cursor = gdk_cursor_new_from_pixbuf (gdk_display_get_default (), cursor_pixbuf, 0, 0);
    wordDic->regular_cursor = gdk_cursor_new_for_display (gdk_display_get_default (), GDK_XTERM);
    wordDic->is_cursor_regular = TRUE;
  }

  wordDic->spinb_searchlimit = NULL;
  wordDic->radiob_searchdic = NULL;
  wordDic->checkb_autoadjust = NULL;
  wordDic->checkb_verb = NULL;

  wordDic->word_search_history_model = gtk_list_store_string_new ();
  worddic_init_history (wordDic->word_search_history_model);
  Verbinit ();

  // setup window
  {
    gtk_window_set_title (GTK_WINDOW (self), _("Gjiten - WordDic"));
    gtk_widget_get_can_default (GTK_WIDGET (self));
    g_signal_connect (G_OBJECT (self), "destroy", G_CALLBACK (worddic_close), NULL);
    gtk_window_set_default_size (GTK_WINDOW (self), 500, 500);
  }

  vbox_main = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_show (vbox_main);
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (vbox_main));

  // setup toolbar
  {
    toolbar = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10));
    gtk_container_add (GTK_CONTAINER (vbox_main), GTK_WIDGET (toolbar));

    wordDic->button_back = gj_container_append_stock (GTK_CONTAINER (toolbar), "go-previous-symbolic",
                                                    _("Previous search result"), "Back",
                                                    on_back_clicked, NULL);
    gtk_widget_set_sensitive (GTK_WIDGET (wordDic->button_back), FALSE);

    wordDic->button_forward = gj_container_append_stock (GTK_CONTAINER (toolbar), "go-next-symbolic",
                                                      _("Next search result"), "Forward",
                                                      on_forward_clicked, NULL);
    gtk_widget_set_sensitive (GTK_WIDGET (wordDic->button_back), FALSE);

    tmpimage = gtk_image_new_from_icon_name ("kanjidic-symbolic", GTK_ICON_SIZE_INVALID /*ignored*/);
    gj_container_append_item (GTK_CONTAINER (toolbar), _("KanjiDic"),
                          _("Launch KanjiDic"), "KanjiDic", tmpimage,
                          G_CALLBACK (gjiten_start_kanjidic), GTK_APPLICATION (g_application_get_default ()));

    tmpimage = gtk_image_new_from_icon_name ("kanjipad-symbolic", GTK_ICON_SIZE_INVALID /*ignored*/);
    gj_container_append_item (GTK_CONTAINER (toolbar), _("KanjiPad"),
                          _("Launch KanjiPad"), "KanjiPad", tmpimage,
                            G_CALLBACK (gjiten_start_kanjipad), NULL);

    {
      GtkToolButton * hid = GTK_TOOL_BUTTON (gtk_tool_button_new (NULL, ""));

      g_signal_connect (hid, "clicked", G_CALLBACK (worddic_show_hide_options), NULL);
      gtk_box_pack_end (GTK_BOX (toolbar), GTK_WIDGET (hid), FALSE, FALSE, 0);
      gtk_widget_set_halign (GTK_WIDGET (hid), GTK_ALIGN_END);

      wordDic->btn_options_show_hide = hid;
      _btn_options_show_hide_update_ui (TRUE);
    }
  }


  wordDic->hbox_options = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  {
    gtk_box_pack_start (GTK_BOX (vbox_main), wordDic->hbox_options, FALSE, TRUE, 0);

    frame_japopt = gtk_frame_new (_("Japanese Search Options: "));
    gtk_box_pack_start (GTK_BOX (wordDic->hbox_options), frame_japopt, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_japopt), 5);

    vbox_japopt = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (frame_japopt), vbox_japopt);

    wordDic->radiob_jpexact = gtk_radio_button_new_with_mnemonic (vbox_japopt_group, _("E_xact Matches"));
    vbox_japopt_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (wordDic->radiob_jpexact));
    gtk_box_pack_start (GTK_BOX (vbox_japopt), wordDic->radiob_jpexact, FALSE, FALSE, 0);

    wordDic->radiob_startw = gtk_radio_button_new_with_mnemonic (vbox_japopt_group, _("_Start With Expression"));
    vbox_japopt_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (wordDic->radiob_startw));
    gtk_box_pack_start (GTK_BOX (vbox_japopt), wordDic->radiob_startw, FALSE, FALSE, 0);

    wordDic->radiob_endw = gtk_radio_button_new_with_mnemonic (vbox_japopt_group, _("E_nd With Expression"));
    vbox_japopt_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (wordDic->radiob_endw));
    gtk_box_pack_start (GTK_BOX (vbox_japopt), wordDic->radiob_endw, FALSE, FALSE, 0);

    wordDic->radiob_any = gtk_radio_button_new_with_mnemonic (vbox_japopt_group, _("_Any Matches"));
    vbox_japopt_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (wordDic->radiob_any));
    gtk_box_pack_start (GTK_BOX (vbox_japopt), wordDic->radiob_any, FALSE, FALSE, 0);

    frame_engopt = gtk_frame_new (_("English Search Options: "));
    gtk_box_pack_start (GTK_BOX (wordDic->hbox_options), frame_engopt, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_engopt), 5);

    vbox_engopt = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (frame_engopt), vbox_engopt);

    wordDic->radiob_engexact = gtk_radio_button_new_with_mnemonic (vbox_engopt_group, _("Wh_ole Expressions"));
    vbox_engopt_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (wordDic->radiob_engexact));
    gtk_box_pack_start (GTK_BOX (vbox_engopt), wordDic->radiob_engexact, FALSE, FALSE, 0);

    wordDic->radiob_words = gtk_radio_button_new_with_mnemonic (vbox_engopt_group, _("_Whole Words"));
    vbox_engopt_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (wordDic->radiob_words));
    gtk_box_pack_start (GTK_BOX (vbox_engopt), wordDic->radiob_words, FALSE, FALSE, 0);

    wordDic->radiob_partial = gtk_radio_button_new_with_mnemonic (vbox_engopt_group, _("Any _Matches"));
    vbox_engopt_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (wordDic->radiob_partial));
    gtk_box_pack_start (GTK_BOX (vbox_engopt), wordDic->radiob_partial, FALSE, FALSE, 0);

    frame_gopt = gtk_frame_new (_("General Options: "));
    gtk_box_pack_start (GTK_BOX (wordDic->hbox_options), frame_gopt, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_gopt), 5);

    grid = gtk_grid_new ();
    gtk_container_add (GTK_CONTAINER (frame_gopt), grid);

    wordDic->radiob_searchdic = gtk_radio_button_new_with_mnemonic (dicssearch_group, _("Search _Dic:"));
    dicssearch_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (wordDic->radiob_searchdic));
    gtk_grid_attach (GTK_GRID (grid), wordDic->radiob_searchdic, 0, 0, 1, 1);

    g_signal_connect_swapped (G_OBJECT (wordDic->radiob_searchdic), "clicked",
                            G_CALLBACK (shade_worddic_widgets), NULL);
  }



  // DICTFILE SELECTION MENU

  wordDic->dicselection_menu = GTK_COMBO_BOX_TEXT (gtk_combo_box_text_new ());
  g_signal_connect (wordDic->dicselection_menu, "changed", G_CALLBACK (on_dicselection_clicked), NULL);
  worddic_update_dic_menu ();

  gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (wordDic->dicselection_menu), 1, 0, 1, 1);

  wordDic->radiob_searchall = gtk_radio_button_new_with_mnemonic (dicssearch_group, _("Sea_rch All Dictionaries"));
  gtk_grid_attach (GTK_GRID (grid), wordDic->radiob_searchall, 0, 1, 2, 1);
  g_signal_connect_swapped (G_OBJECT (wordDic->radiob_searchall), "clicked",
                           G_CALLBACK (shade_worddic_widgets), NULL);

  wordDic->checkb_autoadjust = gtk_check_button_new_with_mnemonic (_("A_uto Adjust Options"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wordDic->checkb_autoadjust), TRUE);
  gtk_grid_attach (GTK_GRID (grid), wordDic->checkb_autoadjust, 0, 2, 1, 1);
  g_signal_connect (G_OBJECT (wordDic->checkb_autoadjust), "toggled",
                   G_CALLBACK (shade_worddic_widgets), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wordDic->checkb_autoadjust), gjitenApp->conf->autoadjust_enabled);

  hbox_searchlimit = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_grid_attach (GTK_GRID (grid), hbox_searchlimit, 0, 3, 2, 1);
  wordDic->checkb_searchlimit = gtk_check_button_new_with_mnemonic (_("_Limit Results:"));
  gtk_box_pack_start (GTK_BOX (hbox_searchlimit), wordDic->checkb_searchlimit, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (wordDic->checkb_searchlimit), "toggled",
                   G_CALLBACK (checkb_searchlimit_toggled), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wordDic->checkb_searchlimit), gjitenApp->conf->searchlimit_enabled);

  spinb_searchlimit_adj = gtk_adjustment_new (gjitenApp->conf->maxwordmatches, 1, G_MAXFLOAT, 1, 2, 0);
  wordDic->spinb_searchlimit = gtk_spin_button_new (GTK_ADJUSTMENT (spinb_searchlimit_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox_searchlimit), wordDic->spinb_searchlimit, FALSE, FALSE, 0);
  gtk_widget_set_sensitive (wordDic->spinb_searchlimit, (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wordDic->checkb_searchlimit))));
  g_signal_connect (G_OBJECT (spinb_searchlimit_adj), "value_changed",
                   G_CALLBACK (get_searchlimit), NULL);

  hbox_entry = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (vbox_main), hbox_entry, FALSE, TRUE, 14);
  gtk_container_set_border_width (GTK_CONTAINER (hbox_entry), 3);

  label_enter = gtk_label_new (_("Enter expression :"));
  gtk_box_pack_start (GTK_BOX (hbox_entry), label_enter, FALSE, TRUE, 7);
  gtk_label_set_justify (GTK_LABEL (label_enter), GTK_JUSTIFY_RIGHT);
  gtk_label_set_xalign (GTK_LABEL (label_enter), 1);
  gtk_label_set_yalign (GTK_LABEL (label_enter), 0.5);

  wordDic->cbo_search_term = GTK_COMBO_BOX (gtk_combo_box_new_with_model_and_entry (GTK_TREE_MODEL (wordDic->word_search_history_model)));
  g_object_unref (wordDic->word_search_history_model);
  gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (wordDic->cbo_search_term), 0);
  gtk_combo_box_set_id_column (GTK_COMBO_BOX (wordDic->cbo_search_term), 0);
  gtk_box_pack_start (GTK_BOX (hbox_entry), GTK_WIDGET (wordDic->cbo_search_term), TRUE, TRUE, 0);
  gtk_widget_style_add_class (GTK_WIDGET (gtk_bin_get_child (
                              GTK_BIN (wordDic->cbo_search_term))), "normalfont");

    g_signal_connect (G_OBJECT (wordDic->cbo_search_term), "changed",
                    G_CALLBACK (cbo_search_term_on_changed), self);
  g_signal_connect (gtk_bin_get_child (GTK_BIN (wordDic->cbo_search_term)),
									 "activate", G_CALLBACK(on_search_clicked), NULL);
  g_signal_connect (G_OBJECT (self), "key_press_event",
                    G_CALLBACK (set_focus_on_entry), gtk_bin_get_child (GTK_BIN (wordDic->cbo_search_term)));


  _search_history_buttons_set_sensitivity ();

  //setup search term input
  {
    GtkEntry * entry = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (wordDic->cbo_search_term)));
    gtk_widget_set_can_default (GTK_WIDGET (entry), TRUE);
    gtk_widget_grab_focus (GTK_WIDGET (entry));
    gtk_widget_grab_default (GTK_WIDGET (entry));
  }

  button_search = gtk_button_new_with_label (_("Search"));
  gtk_box_pack_start (GTK_BOX (hbox_entry), button_search, FALSE, FALSE, 7);
  g_signal_connect (G_OBJECT (button_search), "clicked", G_CALLBACK (on_search_clicked), NULL);

  button_clear = gtk_button_new_with_mnemonic (_("_Clear"));
  gtk_box_pack_start (GTK_BOX (hbox_entry), button_clear, FALSE, FALSE, 0);
  g_signal_connect_swapped (G_OBJECT (button_clear), "clicked",
                           G_CALLBACK (gtk_entry_clear_callback),
                           G_OBJECT (gtk_bin_get_child (GTK_BIN (wordDic->cbo_search_term))));

  frame_results = gtk_frame_new (_("Search results :"));
  gtk_box_pack_start (GTK_BOX (vbox_main), frame_results, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_results), 5);
  gtk_frame_set_label_align (GTK_FRAME (frame_results), 0.03, 0.5);

  vbox_results = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (frame_results), vbox_results);

  wordDic->text_results_view = gtk_text_view_new ();
  gtk_widget_style_add_class (GTK_WIDGET (wordDic->text_results_view), "normalfont");
  wordDic->text_results_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (wordDic->text_results_view));
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (wordDic->text_results_view), GTK_WRAP_WORD);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (wordDic->text_results_view), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (wordDic->text_results_view), FALSE);

  // enable clickable kanji
  g_signal_connect (G_OBJECT (wordDic->text_results_view), "button-release-event", G_CALLBACK (kanji_clicked), NULL);
  g_signal_connect (G_OBJECT (wordDic->text_results_view), "motion-notify-event", G_CALLBACK (result_view_motion), NULL);

  //set up fonts and tags
  gtk_text_buffer_create_tag (wordDic->text_results_buffer, "small_distance", "size-points", 2.0, NULL);
  gtk_text_buffer_create_tag (wordDic->text_results_buffer, "blue_foreground", "foreground", "blue", NULL);
  gtk_text_buffer_create_tag (wordDic->text_results_buffer, "red_foreground", "foreground", "red", NULL);
  gtk_text_buffer_create_tag (wordDic->text_results_buffer, "brown_foreground", "foreground", "brown", NULL);

  worddic_apply_fonts ();

  scrolledwin_results = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwin_results), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (scrolledwin_results), wordDic->text_results_view);
  gtk_box_pack_start (GTK_BOX (vbox_results), scrolledwin_results, TRUE, TRUE, 0);

  wordDic->appbar_mainwin = gtk_label_new ("");
  gtk_label_set_xalign (GTK_LABEL (wordDic->appbar_mainwin), 0);
  gtk_box_pack_end (GTK_BOX (vbox_results), wordDic->appbar_mainwin, FALSE, FALSE, 0);
}



/**
 * TODO:refactor: both lines are called several times in this code.
 *                replace their call with this function later.
 **/
void
worddic_lookup_word(gchar * cli_option_word_to_lookup)
{
  gtk_combo_box_set_text (wordDic->cbo_search_term, gjitenApp->conf->cli_option_word_to_lookup);
  on_search_clicked ();
}



static void
gj_worddic_window_class_init(GjWorddicWindowClass* klass)
{
}



static void
gj_worddic_window_init(GjWorddicWindow* self)
{
  // init variables

  // init private variables:
  // GjWorddicWindowPrivate * priv = gj_worddic_window_get_instance_private (self);
}



static void
_menu_item_copy_clicked(GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       gtk_application)
{
  worddic_copy ();
}



static void
_menu_item_paste_clicked(GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       gtk_application)
{
  worddic_paste ();
}



static void
_menu_setup_actions(GjWorddicWindow *self)
{
  GActionEntry actions[] = {
    {.name="copy",  .activate=_menu_item_copy_clicked, NULL, NULL, NULL },
    {.name="paste",  .activate=_menu_item_paste_clicked, NULL, NULL, NULL },
  };

  GSimpleActionGroup *action_group = g_simple_action_group_new ();
  g_action_map_add_action_entries (G_ACTION_MAP (action_group), actions, G_N_ELEMENTS (actions), NULL);
  gtk_widget_insert_action_group (GTK_WIDGET (self), "window", G_ACTION_GROUP (action_group));
}



GtkWidget*
gj_worddic_window_new(GtkApplication * app)
{
  // for now we must make it Singleton here in ctor, because
  //  calls within _create_gui expect it to be.
  self = GJ_WORDDIC_WINDOW ((g_object_new (gj_worddic_window_get_type (), "application", app, NULL)));
  _menu_setup_actions (self);

  wordDic = gj_worddic_window_get_instance_private (self);
  _create_gui (GJ_WORDDIC_WINDOW (self));
  setWindowIcon (GTK_WINDOW (self), GJITEN_WINDOW_ICON);

  return GTK_WIDGET (self);
}