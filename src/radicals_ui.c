
#include "radicals_ui.h"

#include "utils.h"


/**
 * Disconnect signals to foreign objects,
 * so they don't run with this (already destroyed)
 * widget as parameter.
 **/
static void
_disconnect_signals (GtkWidget *self,
          GjKanjidicWindow* kanjidic_window)
{
  g_signal_handlers_disconnect_by_func (kanjidic_window, gtk_widget_destroy, self);
}



GtkWidget *
radicals_window_new(GjKanjidicWindow* kanjidic_window,
                    Radicals * radicals,
                    GHashTable **rad_button_hash)
{
  GtkWidget *self = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect_swapped (kanjidic_window, "destroy", G_CALLBACK (gtk_widget_destroy), self);
  g_signal_connect (self, "destroy", G_CALLBACK (_disconnect_signals), kanjidic_window);


  int i = 0, j = 0;
  int curr_strokecount = 0;
  GtkWidget *radtable;
  GtkWidget *tmpwidget = NULL;
  GtkWidget *radical_label;
  gchar *strokenum_label;
  gchar radical[6];
  RadInfo *rad_info = NULL;
  GList *rad_info_list;


  if ((*rad_button_hash) != NULL)  {
    g_hash_table_destroy ((*rad_button_hash));
    (*rad_button_hash) = NULL;
  }

  (*rad_button_hash) = g_hash_table_new_full (NULL, NULL, NULL, NULL);

  gtk_window_set_title (GTK_WINDOW (self), _("Radicals"));

  radtable = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (self), radtable);
  gtk_widget_show (radtable);

  for (rad_info_list = radicals->rad_info_list; rad_info_list != NULL; rad_info_list = g_list_next (rad_info_list)) {
    if (i == RADLISTLEN) {
      i = 0;
      j++;
    }
    rad_info = (RadInfo *) rad_info_list->data;
    if (curr_strokecount != rad_info->strokes) {
      if (i == RADLISTLEN - 1) {
        i = 0;
        j++;
      }
      curr_strokecount = rad_info->strokes;
      strokenum_label = g_strdup_printf ("<b>%d</b>", curr_strokecount); //Make a label with the strokenumber
      tmpwidget = gtk_label_new (""); //radical stroke number label
      gtk_label_set_markup (GTK_LABEL (tmpwidget), strokenum_label);
      g_free (strokenum_label);

      gtk_grid_attach (GTK_GRID (radtable), tmpwidget, i, j, 1, 1);
      gtk_widget_show (tmpwidget);
      i++;
    }
    memset (radical, 0, sizeof (radical));
    g_unichar_to_utf8(rad_info->radical, radical);
    radical_label = gtk_label_new (radical);
    gtk_widget_style_add_class (radical_label, "normalfont");
    gtk_widget_show (radical_label);
    tmpwidget = gtk_button_new ();
    gtk_container_add (GTK_CONTAINER (tmpwidget), radical_label);
    g_signal_connect_swapped (G_OBJECT (tmpwidget), "clicked",
                              G_CALLBACK (kanjidic_radical_selected),
                              TO_POINTER (rad_info->radical));

    gtk_grid_attach (GTK_GRID (radtable), tmpwidget, i, j, 1, 1);
    gtk_widget_show (tmpwidget);
    g_hash_table_insert ((*rad_button_hash), TO_POINTER (rad_info->radical), tmpwidget);
    i++;
  }
  gtk_widget_show (self);
  return self;
}
