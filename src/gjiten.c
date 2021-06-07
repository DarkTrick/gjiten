/* -*- Mode: C; tab-width: 2;   indent-tabs-mode: space; c-basic-offset: 2 -*- */
/* vi: set ts=2 sw=2: */
/* gjiten.c

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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "error.h"
#include "constants.h"
#include "conf.h"
#include "dicfile.h"
#include "worddic.h"
#include "kanjidic.h"
#include "pref.h"
#include "gjiten.h"
#include "dicutil.h"
#include "utils.h"

GjitenApp *gjitenApp = NULL;


/***************** VARIABLES ***********************/

void init_old();
gchar *clipboard_text = NULL;


enum {
  KANJIDIC_KEY        = -1,
  WORD_LOOKUP_KEY     = -2,
  KANJI_LOOKUP_KEY    = -3,
  CLIP_KANJI_KEY      = -4,
  CLIP_WORD_KEY       = -5,
  QUICK_LOOKUP_KEY    = -6
};


void
gjiten_init_cmd_params (GApplication *app, GjitenConfig *conf)
{
  const GOptionEntry cmd_params[] =
  {
    {
      .long_name = "kanjidic",
      .short_name = 'k',
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_NONE,
      .arg_data = &(conf->startkanjidic),
      .description = N_("Start up Kanjidic instead of Word dictionary"),
      .arg_description = NULL,
    },
    {
      .long_name = "word-lookup",
      .short_name = 'w',
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_STRING,
      .arg_data = &(conf->word_to_lookup),
      .description = N_("Look up WORD in first dictionary"),
      .arg_description = N_("WORD")
    },
    {
      .long_name = "kanji-lookup",
      .short_name = 'l',
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_STRING,
      .arg_data = &(conf->kanji_to_lookup),
      .description = N_("Look up KANJI in kanji dictionary"),
      .arg_description = N_("KANJI")
    },
    {
      .long_name = "clip-kanji",
      .short_name = 'c',
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_NONE,
      .arg_data = &(conf->clip_kanji_lookup),
      .description = N_("Look up kanji from clipboard"),
      .arg_description = NULL,
    },
    {
      .long_name = "clip-word",
      .short_name = 'v',
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_NONE,
      .arg_data = &(conf->clip_word_lookup),
      .description = N_("Look up word from clipboard"),
      .arg_description = NULL,
    },
    {
      .long_name = "quick-lookup",
      .short_name = '\0',
      .flags = G_OPTION_FLAG_NONE,
      .arg = G_OPTION_ARG_NONE,
      .arg_data = &(conf->quick_lookup_mode),
      .description = N_("Start in quick-lookup-mode: Terminate on Escape or clicking somewhere else."),
      .arg_description = NULL,
    },
    {NULL}
  };

  g_application_add_main_option_entries (G_APPLICATION (app), cmd_params);
}



/**
 * Cleanly close gjiten from anywhere in the code
**/
void gjiten_exit()
{
  if ((gjitenApp->worddic == NULL) && (gjitenApp->kanjidic == NULL))
  {
    GJITEN_DEBUG("gjiten_exit()\n");
    conf_save_options(gjitenApp->conf);
    dicutil_unload_dic();
    dicfile_list_free(gjitenApp->conf->dicfile_list);
    conf_close_handler();


    GApplication * app = g_application_get_default ();
    g_application_quit (app);
  }
}


/**
 * Cleanly close gjiten from anywhere in the code
 **/
void gjiten_quit()
{
  // Close all windows (gjiten_exit() will be called inside there)
  kanjidic_close();
  worddic_close();
}


void gjiten_start_kanjipad() {
  FILE *kanjipad_binary;
  char *kpad_cmd;
  int32_t len;

  kanjipad_binary = fopen(gjitenApp->conf->kanjipad, "r");
  if (kanjipad_binary == NULL) {
    gjiten_print_error(_("Couldn't find the KanjiPad executable!\n"
                         "Please make sure you have it installed on your system \n"
                         "and set the correct path to it in the Preferences.\n"
                         "See the Documentation for more details about KanjiPad."));
  }
  else {
    len = strlen(gjitenApp->conf->kanjipad) + 2;
    fclose(kanjipad_binary);
    kpad_cmd = g_malloc(len);
    strncpy(kpad_cmd, gjitenApp->conf->kanjipad, len);
    strncat(kpad_cmd, "&", 1);
    int unused = system(kpad_cmd); // FIXME
    g_free(kpad_cmd);
  }
}

gboolean
gnome_help_display (const char *file_name,
                    const char *link_id,
                    GError **error)
{
  return FALSE;
}


void gjiten_display_manual(GtkWidget *widget, void *data)
{
  GError *err = NULL;

  gboolean retval = FALSE;
  retval = gtk_show_uri_on_window ( GTK_WINDOW (widget),
                                    "ghelp:gjiten",
                                    GDK_CURRENT_TIME,
                                    &err);

  if (retval == FALSE)
  {
    GtkWidget *dialog;

    char * message = _("(unknown)");
    if (err)
      message = err->message;

    dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    _("Could not display help: %s"),
                                    message);

    g_signal_connect(G_OBJECT(dialog), "response",
                      G_CALLBACK(gtk_widget_destroy),
                      NULL);

    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    gtk_widget_show(dialog);

    if (err)
      g_error_free(err);
  }
}


void gjiten_create_about() {
  const gchar *authors[] = { "Botond Botyanszki <boti@rocketmail.com>, DarkTrick", NULL };
  const gchar *documenters[] = { NULL };
  const gchar *translator = _("TRANSLATORS! PUT YOUR NAME HERE");
  GdkPixbuf *pixbuf = NULL;

  pixbuf = gdk_pixbuf_new_from_file(PIXMAPDIR"/gjiten/gjiten-logo.png", NULL);

  if (strncmp(translator, "translated_by", 13) == 0) translator = NULL;

  /*
    _("Released under the terms of the GNU GPL.\n"
    "Check out http://gjiten.sourceforge.net for updates"),
  */
  {

    GtkAboutDialog * about = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
    gtk_about_dialog_set_program_name       (about, "gjiten");
    gtk_about_dialog_set_version            (about, VERSION);
    gtk_about_dialog_set_copyright          (about, "Copyright \xc2\xa9 1999-2005 Botond Botyanszki\nCopyright \xc2\xa9 2019-2021 DarkTrick");
    gtk_about_dialog_set_comments           (about, _("gjiten is a Japanese dictionary for Gnome"));
    gtk_about_dialog_set_authors            (about, (const char **)authors);
    gtk_about_dialog_set_documenters        (about, (const char **)documenters);
    gtk_about_dialog_set_translator_credits (about, (const char *)translator);
    gtk_about_dialog_set_logo               (about, pixbuf);

    gtk_window_set_destroy_with_parent (GTK_WINDOW(about), TRUE);
    if (pixbuf != NULL)  g_object_unref (pixbuf);

    g_signal_connect(G_OBJECT(about), "destroy", G_CALLBACK(gtk_widget_destroyed), &about);
    gtk_widget_show(GTK_WIDGET (about));
  }

}



static void
_create_submenu (const gchar *name,
                       GMenu *content,
                       GMenu *parent)
{
  GMenuItem * button = g_menu_item_new (name, "unused");
  g_menu_item_set_submenu (button, G_MENU_MODEL (content));
  g_menu_append_item (parent, button);
}

static void
_action_start_kanjipad(GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       gtk_application)
{
  gjiten_start_kanjidic (GTK_APPLICATION (gtk_application));
}

static void
_action_start_worddic (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       gtk_application)
{
  gjiten_start_worddic (GTK_APPLICATION (gtk_application));
}


static void
_action_display_manual (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       gtk_application)
{
  gjiten_display_manual (NULL, NULL);
}

static void
_gjiten_create_menu(GtkApplication *app)
{
  GMenu * menubar = g_menu_new();

  // ---- create containers for menus ----

  {
    GMenu * content = g_menu_new();
    g_menu_append (content, _("_Quit"), "app.quit");
    _create_submenu (_("_File"), content, menubar);
  }

  {
    GMenu * content = g_menu_new();
    g_menu_append (content, _("_Copy"),        "edit.copy"        );
    g_menu_append (content, _("_Paste"),       "edit.paste"       );
    g_menu_append (content, _("_Preferences"), "app.preferences" );
    _create_submenu (_("_Edit"), content, menubar);
  }

  {
    GMenu * content = g_menu_new();
    g_menu_append (content, _("_Word Dictionary"),  "app.startWorddic" );
    g_menu_append (content, _("_Kanji Dictionary"), "app.startKanjidic" );
    g_menu_append (content, _("Kanji _Pad"),        "app.startKanjipad" );
    _create_submenu (_("_Tools"), content, menubar);
  }

  {
    GMenu * content = g_menu_new();
    g_menu_append (content, _("_Manual"), "app.showManual" );
    g_menu_append (content, _("_About"),  "app.showAbout" );
    _create_submenu (_("_Help"), content, menubar);
  }

  gtk_application_set_menubar (GTK_APPLICATION (g_application_get_default()), G_MENU_MODEL (menubar));

  g_object_unref (menubar);

  // link action names (above) to callback functions
  {
    GActionEntry actions[] = {
        {.name="quit",          .activate=gjiten_quit },
        {.name="preferences",   .activate=create_dialog_preferences },
        {.name="startKanjipad", .activate=gjiten_start_kanjipad },
        {.name="showAbout",     .activate=gjiten_create_about },
      };

    g_action_map_add_action_entries (G_ACTION_MAP (app), actions, G_N_ELEMENTS (actions), NULL);
  }
  // separate processing for functions with parameters
  {
    GActionEntry actions[] = {
      {.name="startWorddic",  .activate=_action_start_worddic },
      {.name="startKanjidic", .activate=_action_start_kanjipad },
      {.name="showManual",    .activate=_action_display_manual },
    };

    g_action_map_add_action_entries (G_ACTION_MAP (app), actions, G_N_ELEMENTS (actions), app);
  }
}



void
gjiten_start_worddic(GtkApplication *app){

  if (gjitenApp->worddic != NULL) {
    gtk_window_present(GTK_WINDOW(gjitenApp->worddic));
  }

  GjWorddicWindow *window = worddic_create (app);
  gjitenApp->worddic = window;
  gtk_widget_show_all ((GtkWidget*)window);
}



void
gjiten_start_kanjidic(GtkApplication *app)
{
  if (gjitenApp->kanjidic != NULL) {
    gtk_window_present(GTK_WINDOW(gjitenApp->kanjidic));
  }

  gjitenApp->kanjidic =  kanjidic_create ();
  GtkWindow *window = GTK_WINDOW (kanjidic_create ()->window);
  gtk_widget_show_all (GTK_WIDGET (gjitenApp->kanjidic->window));
}


void
gjiten_apply_fonts (GjitenApp * gjitenApp)
{
  // apply css styles
  if (gjitenApp->conf->normalfont != NULL)
  {
    gchar * css_font = g_pango_font_convert_to_css (gjitenApp->conf->normalfont);
    GString * css = g_string_new ("");
    g_string_printf (css, "font: %s;", css_font);
    set_global_css ("normalfont", css->str);
    g_string_free (css, TRUE);
    g_free (css_font);
  }

  // apply tag styles
  worddic_apply_fonts();
  kanjidic_apply_fonts();
}



void
gjiten_activate (GtkApplication *app,
                 gpointer        user_data)
{
  _gjiten_create_menu (GTK_APPLICATION (app));

  gjiten_apply_fonts (gjitenApp);

  // the following is for clipboard lookup.
  if ((gjitenApp->conf->clip_kanji_lookup == TRUE) || (gjitenApp->conf->clip_word_lookup == TRUE)) {
    if (gjitenApp->conf->clip_word_lookup) {
      gjiten_start_worddic (app);
      worddic_paste();
      on_search_clicked();
    }
    else {
      if (gjitenApp->conf->clip_kanji_lookup){
        clipboard_text = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY));
        // validate
        // FIXME: try to convert EUC-JP to UTF8 if it's non-utf8
        if (g_utf8_validate(clipboard_text, -1, NULL) == FALSE) {
          gjiten_print_error(_("Unable to look up kanji: NON-UTF8 string received from clipboard!\n"));
          exit(0);
        }
        else if (isKanjiChar(g_utf8_get_char(clipboard_text)) == FALSE) {
          gjiten_print_error(_("Non-kanji string received from clipboard: %s\n"), clipboard_text);
          exit(0);
        }
        else {
          if (gjitenApp->kanjidic == NULL) gjiten_start_kanjidic (app);
          print_kanjinfo(g_utf8_get_char(clipboard_text));
        }
      }
    }
  }


  {
    if (gjitenApp->conf->startkanjidic) {
      gjiten_start_kanjidic (app);
    }
    else
      if (gjitenApp->conf->word_to_lookup) {
        gjiten_start_worddic (app);
        worddic_lookup_word (gjitenApp->conf->word_to_lookup);
      }
      else if (gjitenApp->conf->kanji_to_lookup != NULL) {
        if (g_utf8_validate(gjitenApp->conf->kanji_to_lookup, -1, NULL) == FALSE) {
          gjiten_print_error(_("Unable to look up kanji: NON-UTF8 string received from clipboard!\n"));
          exit(0); // FIXME
        }
        else if (isKanjiChar(g_utf8_get_char(gjitenApp->conf->kanji_to_lookup)) == FALSE) {
          gjiten_print_error(_("Non-kanji string received from clipboard: %s\n"), gjitenApp->conf->kanji_to_lookup);
          exit(0); // FIXME
        }
        else {
          if (gjitenApp->kanjidic == NULL) gjiten_start_kanjidic (app);
          print_kanjinfo(g_utf8_get_char(gjitenApp->conf->kanji_to_lookup));
        }
      }
      else if (!gjitenApp->conf->clip_kanji_lookup && !gjitenApp->conf->clip_word_lookup)
        gjiten_start_worddic (app);
  }

  // if nothing was started so far (= no cmd params given)
  if (!gjitenApp->worddic && !gjitenApp->kanjidic)
  {
    gjiten_start_worddic (app);
  }

  gjiten_flush_errors();

  // enable quick lookup mode if set
  if(gjitenApp->conf->quick_lookup_mode && gjitenApp->worddic)
    enable_quick_lookup_mode();

}

GtkApplication *
gjiten_new()
{
  gjitenApp = g_new0(GjitenApp, 1);
  conf_init_handler();
  gjitenApp->conf = conf_load();

  if (gjitenApp->conf->envvar_override == TRUE) {
    if (gjitenApp->conf->gdk_use_xft == TRUE) putenv("GDK_USE_XFT=1");
    else putenv("GDK_USE_XFT=0");
    // if (gjitenApp->conf->force_ja_JP == TRUE) putenv("LC_CTYPE=ja_JP");
    if (gjitenApp->conf->force_ja_JP == TRUE) putenv("LC_ALL=ja_JP");
    if (gjitenApp->conf->force_language_c == TRUE) putenv("LANGUAGE=C");
  }



  #ifdef ENABLE_NLS
    bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);
  #endif


  GtkApplication * app = gtk_application_new(NULL,G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK (gjiten_activate), NULL);


  gjiten_init_cmd_params (G_APPLICATION (app), gjitenApp->conf);

  return app;
}