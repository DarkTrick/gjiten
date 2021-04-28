#include "utils.h"
#include <gtk/gtk.h>

/**
 *  Reimplementation of original function from gtk 2
 **/
GtkToolButton*
gtk_toolbar_insert_stock (GtkToolbar *toolbar,
                          const char *icon_name,
                          const char *tooltip_text,
                          const char *tooltip_private_text,
                          GCallback callback_nullable,
                          gpointer user_data_nullable,
                          gint position)
{
  GtkWidget     *icon;
  GtkToolButton *button;

  icon = gtk_image_new_from_icon_name (icon_name, 20);
  button = GTK_TOOL_BUTTON (gtk_tool_button_new (icon, tooltip_private_text));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), tooltip_text);

  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (button), position);

  if (callback_nullable){
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (callback_nullable), user_data_nullable);
  }

  return button;
}


/**
 *  Reimplementation of original function from gtk 2
 **/
GtkToolButton*
gtk_toolbar_append_item(GtkToolbar *toolbar,
                        const char *text,
                        const char *tooltip_text,
                        const char *tooltip_private_text,
                        GtkWidget *icon,
                        GCallback callback_nullable,
                        gpointer  user_data_nullable)
{
  GtkToolButton *button;

  button = GTK_TOOL_BUTTON (gtk_tool_button_new (icon, tooltip_private_text));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), tooltip_text);

  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (button), -1);

  if (callback_nullable){
    g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (callback_nullable), user_data_nullable);
  }

  return button;
}

/**
 *  allow clearing an entry as callback
 **/
void
gtk_entry_clear_callback(gpointer entrybox)
{
  g_return_if_fail (GTK_IS_ENTRY (entrybox));
  gtk_entry_set_text(GTK_ENTRY(entrybox), "");
}

void
gtk_combo_box_text_add_entries(GtkComboBoxText *self,
                               GList           *list_of_strings)
{
  GList * iter;
  iter = list_of_strings;
  while (NULL != iter){
    gtk_combo_box_text_append_text (self, iter->data);
    iter = iter->next;
  }
}


void
setWindowIcon(GtkWindow *window,
              char      *icon_path)
{
  if (! g_file_test (icon_path, G_FILE_TEST_EXISTS)) {
    g_warning ("Could not find %s", icon_path);
  }
  else {
    gtk_window_set_icon_from_file(window, icon_path, NULL);
  }
}


void
gtk_widget_register_action_entries(GtkWidget    *self,
                                   const gchar  *group_name,
                                   GActionEntry  actions[])
{
  GSimpleActionGroup * group = g_simple_action_group_new ();
  g_action_map_add_action_entries (G_ACTION_MAP (group), actions, G_N_ELEMENTS (actions), self);
  gtk_widget_insert_action_group (GTK_WIDGET (self), group_name, G_ACTION_GROUP (group));
}
