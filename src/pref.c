/* -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* vi: set ts=2 sw=2: */
/* pref.c

   GJITEN : A JAPANESE DICTIONARY FOR GNOME

   Copyright (C) 1999-2005 Botond Botyanszki <boti@rocketmail.com>

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
#include <gtk/gtk.h>

#include "constants.h"
#include "error.h"
#include "kanjidic.h"
#include "conf.h"
#include "worddic.h"
#include "pref.h"
#include "gjiten.h"
#include "dicutil.h"
#include <glib/gi18n.h>
#include <locale.h>


#define GETWIDGET(s) GTK_WIDGET (gtk_builder_get_object(builder, s))
static GtkBuilder * builder;

extern GjitenApp *gjitenApp;

GtkWidget *dialog_preferences = NULL;
static GtkTreeModel *model;
static GtkTreeIter iter;
static GtkWidget *checkb_prefs[KCFGNUM];
static GtkTreeView *treeview;
static GtkWidget *file_chooser_kanjidic;
static GtkWidget *file_chooser_kanjipad;

gchar *strginfo[] = { //FIXME: change this to EnumPair
	N_("Kanji"),
	N_("Radicals"),
	N_("Stroke count"),
	N_("Readings"),
	N_("Romanized Korean reading"),
	N_("Romanized Pinyin reading"),
	N_("English meaning"),
	N_("Bushu radical number"),
	N_("Classical radical number"),
	N_("Frequency number"),
	N_("Jouyou grade level"),
	N_("De Roo code"),
	N_("Skip pattern code"),
	N_("Four Corner code"),
	N_("\"New Jp-En Char Dictionary\" index"),
	N_("Index in Nelson's \"Modern Reader's Char Dict\""),
	N_("\"The New Nelson Char Dict\" index"),
	N_("Spahn&Hadamitzky index"),
	N_("Morohashi \"Daikanwajiten\" index"),
	N_("Morohashi \"Daikanwajiten\" volume.page number"),
	N_("\"A Guide To Remembering Jap. Chars\" index"),
	N_("Gakken Kanji Dictionary index"),
	N_("Index in \"Remembering The Kanji\""),
	N_("Index in \"Japanese Names\""),
	N_("Cross-reference code"),
	N_("Misclassification code"),
	N_("Unicode hex number of the kanji"),
	N_("ASCII JIS Code of Kanji")
};


enum {
  COL_DICPATH,
	COL_DICNAME,
  NUM_COLS
};


void font_set(GtkFontChooser *fontpicker, GtkWidget *entry) {
  const gchar *fontname;
  fontname = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(fontpicker));
  gtk_entry_set_text(GTK_ENTRY(entry), fontname);
}


static void add_dic_response_cb(GtkDialog *dialog, gint response, GtkFileChooser *fileentry) {

	GtkWidget *nameentry;
	GjitenDicfile dicfile;

  if (GTK_RESPONSE_NONE == response ||
      GTK_RESPONSE_REJECT == response ||
      GTK_RESPONSE_CANCEL == response ||
      GTK_RESPONSE_CLOSE == response ||
      GTK_RESPONSE_NO == response)
  {
    gtk_widget_hide (GTK_WIDGET (dialog));
  }
  else
  {
		nameentry = GETWIDGET("entry_dic_name");
		dicfile.name = g_strdup(gtk_entry_get_text(GTK_ENTRY(nameentry)));

		dicfile.path = gtk_file_chooser_get_filename(fileentry);

		if (dicfile.path != NULL) {
			if (dicfile_init(&dicfile) == FALSE) return;
			if (dicfile_is_utf8(&dicfile) == FALSE) {
				dicfile_close(&dicfile);
				return;
			}
			dicfile_close(&dicfile);
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_DICPATH, dicfile.path, -1);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_DICNAME, dicfile.name, -1);
		}
		else gjiten_print_error(_("Dictionary file not found!"));
	}
}

static void set_dic_name_cb(GtkFileChooser *fileentry, GtkEntry *entry) {

  gchar *filename, *dictf, *old;
	const gchar *entrytext;
  char * saveptr = NULL;

	GJITEN_DEBUG("set_dic_name_cb()\n");

  filename = gtk_file_chooser_get_filename(fileentry);
  old = dictf = strtok_r(filename, "/", &saveptr);
  while (dictf != NULL) {
    old = dictf;
    dictf = strtok_r(NULL, "/", &saveptr);
  }

	entrytext = gtk_entry_get_text(entry);
	GJITEN_DEBUG("Dicname old: %s, new: %s\n", entrytext, old);
	if ((entrytext == NULL) || (strlen(entrytext) == 0)) gtk_entry_set_text(entry, old);
  g_free (filename);
}

static void add_dict(GtkWidget *button, gpointer nothing) {

  GtkWidget *dialog_add_dic = NULL;
	GtkWidget *fileselector;
	GtkWidget *nameentry;
	GJITEN_DEBUG("add_dict()\n");

  dialog_add_dic = GETWIDGET ("dialog_add_dic");
  nameentry = GETWIDGET ("entry_dic_name");
  fileselector = GETWIDGET ("file_dic_path");
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fileselector), GJITEN_DICDIR);

  g_signal_connect(G_OBJECT(dialog_add_dic), "response", G_CALLBACK(add_dic_response_cb), fileselector);

  gtk_widget_show_all(dialog_add_dic);
  gtk_dialog_run (GTK_DIALOG (dialog_add_dic));
}

static void change_dic_response_cb(GtkDialog *dialog, gint response, GtkFileChooser *fileentry) {

  gchar *dicpath, *dicname;
	GtkWidget *nameentry;
  GtkTreeSelection *selection;

  if (response != GTK_RESPONSE_CLOSE) {
		nameentry = GETWIDGET ("entry_dic_name_change");
		dicname = g_strdup(gtk_entry_get_text(GTK_ENTRY(nameentry)));

		dicpath = gtk_file_chooser_get_filename(fileentry);
		if (dicpath != NULL) {
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
			if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE) return;
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_DICPATH, dicpath, -1);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_DICNAME, dicname, -1);
		}
		else gjiten_print_error(_("Dictionary file not found!"));
	}
	else gtk_widget_destroy(GTK_WIDGET(dialog));
}


static void change_dict(GtkWidget *button, gpointer treeview) {

	GtkWidget *fileselector;
	gchar *dicpath, *dicname;
	static GtkWidget *dialog = NULL;
	GtkWidget *nameentry;
  GtkTreeSelection *selection;

	//FIXME: segfault when called many times (click on row)

	if (GTK_IS_WIDGET(dialog) == TRUE) {
		gtk_widget_destroy(GTK_WIDGET(dialog));
		dialog = NULL;
	}

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE) {
		gjiten_print_error(_("Please select an entry!"));
		return;
	}

  dialog = GETWIDGET ("dialog_change_dic");

	nameentry = GETWIDGET ("entry_dic_name_change");
	gtk_tree_model_get(model, &iter, COL_DICPATH, &dicpath, -1);
	gtk_tree_model_get(model, &iter, COL_DICNAME, &dicname, -1);

	if (dicname != NULL) gtk_entry_set_text(GTK_ENTRY(nameentry), dicname);

	fileselector = GETWIDGET ("file_dic_path_change");
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(fileselector), dicpath);

	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(change_dic_response_cb), fileselector);

	gtk_widget_show_all(dialog);
}


static void down_dict(GtkWidget *button, gpointer treeview) {
  GtkTreeIter tmpiter;
  GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE) return;
	tmpiter = iter;
	if (gtk_tree_model_iter_next(model, &tmpiter) == FALSE) return;

	gtk_list_store_swap(GTK_LIST_STORE(model), &iter, &tmpiter);
}


static void up_dict(GtkWidget *button, gpointer treeview) {
  GtkTreeIter tmpiter;
  GtkTreeSelection *selection;
	GtkTreePath *treepath;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE) return;
	treepath = gtk_tree_model_get_path(model, &iter);
	if (gtk_tree_path_prev(treepath) == TRUE) {
		if (gtk_tree_model_get_iter(model, &tmpiter, treepath) == TRUE) {
			gtk_list_store_swap(GTK_LIST_STORE(model), &iter, &tmpiter);
		}
	}
	gtk_tree_path_free(treepath);
}


static void remove_dict(GtkWidget *button, gpointer treeview) {
  GtkTreeSelection *selection;
	GtkTreePath *treepath;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter) == FALSE) return;
	treepath = gtk_tree_model_get_path(model, &iter);
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	gtk_tree_selection_select_path(selection, treepath);
	gtk_tree_path_free(treepath);
}


void preferences_exit() {
  gtk_widget_destroy(dialog_preferences);
	dialog_preferences = NULL;
  g_object_unref(builder);
  builder = NULL;
}



void preferences_response_cb(GtkDialog *dialog, gint response, gpointer user_data) {
	int i;
	gboolean valid;
	GjitenDicfile *dicfile;
	gchar *kanjidic_path = NULL;
	gchar *kanjipad_path = NULL;

	if (response == GTK_RESPONSE_CANCEL) {
		preferences_exit();
		return;
	}

	if (response == GTK_RESPONSE_HELP) {
		gjiten_display_manual(dialog, NULL);
		return;
	}


	for (i = 0; i < KCFGNUM; i++) {
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkb_prefs[i]))) gjitenApp->conf->kdiccfg[i] = TRUE;
		else gjitenApp->conf->kdiccfg[i] = FALSE;
	}

	kanjidic_path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser_kanjidic));
	if ((kanjidic_path != NULL) && strlen(kanjidic_path))  {
		gjitenApp->conf->kanjidic->path = kanjidic_path;
	}

	kanjipad_path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser_kanjipad));
	if ((kanjipad_path != NULL) && strlen(kanjipad_path))  {
		gjitenApp->conf->kanjipad = kanjipad_path;
	}

	#define TOGGLE_BUTTON_ACTIVE(identifier) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GETWIDGET(identifier)))
	gjitenApp->conf->bigwords         = TOGGLE_BUTTON_ACTIVE("checkbutton_largefont_worddic");
	gjitenApp->conf->bigkanji         = TOGGLE_BUTTON_ACTIVE("checkbutton_largefont_kanjidic");
	gjitenApp->conf->gdk_use_xft      = TOGGLE_BUTTON_ACTIVE("checkbutton_use_xft");
	gjitenApp->conf->force_ja_JP      = TOGGLE_BUTTON_ACTIVE("checkbutton_lc_ctype");
	gjitenApp->conf->force_language_c = TOGGLE_BUTTON_ACTIVE("checkbutton_language_c");
	gjitenApp->conf->envvar_override 	= TOGGLE_BUTTON_ACTIVE("checkbutton_envvar_override");

	gjitenApp->conf->normalfont       = g_strdup(gtk_entry_get_text(GTK_ENTRY(GETWIDGET("entry_normal_font"))));
	gjitenApp->conf->largefont        = g_strdup(gtk_entry_get_text(GTK_ENTRY(GETWIDGET("entry_large_font"))));

	gjitenApp->conf->search_kata_on_hira = TOGGLE_BUTTON_ACTIVE("checkbutton_search_kata_on_hira");
	gjitenApp->conf->search_hira_on_kata = TOGGLE_BUTTON_ACTIVE("checkbutton_search_hira_on_kata");
	gjitenApp->conf->verb_deinflection 	 = TOGGLE_BUTTON_ACTIVE("checkbutton_verb_deinflection");
	gjitenApp->conf->unicode_radicals 	 = TOGGLE_BUTTON_ACTIVE("checkbutton_unicode_radicals");
	#undef TOGGLE_BUTTON_ACTIVE

	gjitenApp->conf->numofdics = 0;

	dicutil_unload_dic();
	dicfile_list_free(gjitenApp->conf->dicfile_list);
	gjitenApp->conf->dicfile_list = NULL;

	valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid == TRUE) {
		dicfile = g_new0(GjitenDicfile, 1);
		dicfile->status = DICFILE_NOT_INITIALIZED;
		gtk_tree_model_get(model, &iter, COL_DICPATH, &dicfile->path, COL_DICNAME, &dicfile->name, -1);
		gjitenApp->conf->dicfile_list = g_slist_append(gjitenApp->conf->dicfile_list, dicfile);
		valid = gtk_tree_model_iter_next(model, &iter);
	}

	conf_save(gjitenApp->conf);

	worddic_update_dic_menu();
	worddic_apply_fonts();
	kanjidic_apply_fonts();

	if (response == GTK_RESPONSE_OK) preferences_exit();
}

static void checkbutton_envvar_cb(GtkWidget *button) {
	gboolean enabled;

	enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_envvar_override")));

	gtk_widget_set_sensitive(GETWIDGET("label_envvar_warning"), enabled);
	gtk_widget_set_sensitive(GETWIDGET("checkbutton_use_xft"), enabled);
	gtk_widget_set_sensitive(GETWIDGET("checkbutton_lc_ctype"), enabled);
	gtk_widget_set_sensitive(GETWIDGET("checkbutton_language_c"), enabled);

}


void create_dialog_preferences() {
	GtkWidget *button;
	GtkWidget *tmpwidget;
  GtkWidget *fontpicker;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
	GSList *dicfile_node;
	GjitenDicfile *dicfile;

  int trow = 0, tcol = 0;

  if (dialog_preferences != NULL) return;

  builder = gtk_builder_new_from_file (GJITEN_DATADIR"/gjiten-settings.glade");

  dialog_preferences = GETWIDGET("gjiten_settings");

	// Set up the dicfile list
	treeview = GTK_TREE_VIEW(GETWIDGET("treeview_dics"));
  model = GTK_TREE_MODEL(gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT));
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), model);

	renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Dictionary file path"), renderer, "text", COL_DICPATH, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes(_("Dictionary name"), renderer, "text", COL_DICNAME, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

  g_signal_connect(G_OBJECT(treeview), "row_activated", G_CALLBACK(change_dict), NULL);

	dicfile_node = gjitenApp->conf->dicfile_list;
	while (dicfile_node != NULL) {
		if (dicfile_node->data == NULL) break;
		dicfile = dicfile_node->data;
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_DICPATH, dicfile->path, COL_DICNAME, dicfile->name, -1);
		dicfile_node = g_slist_next(dicfile_node);
  }

  button = GETWIDGET("button_adddic");
	g_signal_connect(G_OBJECT(button), "clicked",
									 G_CALLBACK(add_dict), NULL);

  button = GETWIDGET("button_updic");
	g_signal_connect(G_OBJECT(button), "clicked",
									 G_CALLBACK(up_dict), treeview);

  button = GETWIDGET("button_downdic");
	g_signal_connect(G_OBJECT(button), "clicked",
									 G_CALLBACK(down_dict), treeview);

  button = GETWIDGET("button_removedic");
	g_signal_connect(G_OBJECT(button), "clicked",
									 G_CALLBACK(remove_dict), treeview);

  button = GETWIDGET("button_changedic");
	g_signal_connect(G_OBJECT(button), "clicked",
									 G_CALLBACK(change_dict), treeview);


	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_search_kata_on_hira")), gjitenApp->conf->search_kata_on_hira);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_search_hira_on_kata")), gjitenApp->conf->search_hira_on_kata);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_verb_deinflection")), gjitenApp->conf->verb_deinflection);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_unicode_radicals")), gjitenApp->conf->unicode_radicals);

  tmpwidget = GETWIDGET("file_chooser_kanjidic_path");
  file_chooser_kanjidic = tmpwidget;
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (tmpwidget), GJITEN_DICDIR);
  if (gjitenApp->conf->kanjidic && gjitenApp->conf->kanjidic->path) {
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (tmpwidget), gjitenApp->conf->kanjidic->path);
	}

  tmpwidget = GETWIDGET("table_kanji_info");
  for (tcol = 0; (tcol * 14 + trow < KCFGNUM) || (tcol < 3); tcol++) {
    for (trow = 0; (tcol * 14 + trow < KCFGNUM) && (trow < 14); trow++) {
      checkb_prefs[tcol * 14 + trow] = gtk_check_button_new_with_label(_(strginfo[tcol * 14 + trow]));
      gtk_widget_show(checkb_prefs[tcol * 14 + trow]);
      gtk_grid_attach(GTK_GRID(tmpwidget), checkb_prefs[tcol * 14 + trow], tcol, trow, 1, 1);
      if (gjitenApp->conf->kdiccfg[tcol * 14 + trow] == TRUE)
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkb_prefs[tcol * 14 + trow]), TRUE);
    }
	}

  fontpicker = GETWIDGET ("font_picker_normal_font");
  gtk_widget_show(fontpicker);
  if (gjitenApp->conf->normalfont != NULL) {
    gtk_entry_set_text(GTK_ENTRY(GETWIDGET("entry_normal_font")), gjitenApp->conf->normalfont);
    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (fontpicker), gjitenApp->conf->normalfont);
  }
  g_signal_connect(G_OBJECT(fontpicker), "font-set", G_CALLBACK(font_set), (gpointer)GETWIDGET("entry_normal_font"));

  fontpicker = GETWIDGET ("font_picker_large_font");
  gtk_widget_show(fontpicker);
  if (gjitenApp->conf->largefont != NULL) {
    gtk_entry_set_text(GTK_ENTRY(GETWIDGET("entry_large_font")), gjitenApp->conf->largefont);
    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (fontpicker), gjitenApp->conf->largefont);
  }
  g_signal_connect(G_OBJECT(fontpicker), "font-set", G_CALLBACK(font_set), (gpointer)GETWIDGET("entry_large_font"));


	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_largefont_worddic")), gjitenApp->conf->bigwords);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_largefont_kanjidic")), gjitenApp->conf->bigkanji);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_envvar_override")), gjitenApp->conf->envvar_override);
	checkbutton_envvar_cb(NULL);

	g_signal_connect(GETWIDGET("checkbutton_envvar_override"), "toggled",
									 G_CALLBACK(checkbutton_envvar_cb), NULL);


	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_use_xft")), gjitenApp->conf->gdk_use_xft);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_lc_ctype")), gjitenApp->conf->force_ja_JP);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GETWIDGET("checkbutton_language_c")), gjitenApp->conf->force_language_c);


  tmpwidget = GETWIDGET("file_chooser_kanjipad_exe_path");
	file_chooser_kanjipad = tmpwidget;
	if (gjitenApp->conf->kanjipad != NULL) {
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (tmpwidget), gjitenApp->conf->kanjidic->path);
	}

  g_signal_connect(G_OBJECT(dialog_preferences), "response", G_CALLBACK(preferences_response_cb), NULL);


  gtk_widget_show_all(dialog_preferences);

}