#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>


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

void setWindowIcon (GtkWindow       *window,
                    char            *icon_path);

void gtk_widget_register_action_entries(GtkWidget    *self,
                                        const gchar  *group_name,
                                        GActionEntry  actions[]);

void set_global_css(gchar *css_class,
                   gchar *css);

void gtk_widget_style_add_class(GtkWidget   *widget,
                                const gchar *css_class);

void gtk_list_store_prepend_string(GtkListStore * store,
                                   gchar        * theString);
void gtk_list_store_append_string(GtkListStore  * store,
                                  gchar         * theString);
gchar *gtk_list_store_get_string(GtkListStore   * self,
                                 GtkTreeIter    * iter);

gint gtk_tree_model_length(GtkTreeModel* self);

gboolean gtk_combo_box_next(GtkComboBox * self);
gboolean gtk_combo_box_previous(GtkComboBox * self);


#endif