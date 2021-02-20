#include "utils.h"
#include <gtk/gtk.h>

// TODO: remove method; just here, because
//       compiling currently doesn't find "_" function
const char * _(const char* a){
  return a;
}


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
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (callback_nullable), user_data_nullable);
  }

  return button;
}