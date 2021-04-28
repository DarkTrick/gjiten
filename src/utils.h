#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>

//TODO:correct As long as internationalization is not activated
#ifndef GETTEXT_PACKAGE
  #define GETTEXT_PACKAGE "gtk30"
#endif

#include <glib/gi18n.h>
#include <locale.h>

#define GJITEN_WINDOW_ICON PIXMAPDIR"/jiten.png"

GtkToolButton* gtk_toolbar_insert_stock(GtkToolbar *toolbar,
                                        const char *icon_name,
                                        const char *tooltip_text,
                                        const char *tooltip_private_text,
                                        GCallback   callback_nullable,
                                        gpointer    user_data_nullable,
                                        gint        position);

GtkToolButton* gtk_toolbar_append_item(GtkToolbar *toolbar,
                                       const char *text,
                                       const char *tooltip_text,
                                       const char *tooltip_private_text,
                                       GtkWidget  *icon,
                                       GCallback   callback_nullable,
                                       gpointer    user_data_nullable);

void gtk_entry_clear_callback(gpointer entrybox);

void gtk_combo_box_text_add_entries(GtkComboBoxText *self,
                                    GList           *list_of_strings);

void setWindowIcon                 (GtkWindow       *window,
                                    char            *icon_path);

void gtk_widget_register_action_entries(GtkWidget    *self,
                                        const gchar  *group_name,
                                        GActionEntry  actions[]);


#endif