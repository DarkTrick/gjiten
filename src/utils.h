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

void setWindowIcon(GtkWindow       *window,
                   char            *icon_path);

void gtk_widget_register_action_entries(GtkWidget    *self,
                                        const gchar  *group_name,
                                        GActionEntry  actions[]);

void set_global_css(gchar *css_class,
                   gchar *css);

void gtk_widget_style_add_class(GtkWidget   *widget,
                                const gchar *css_class);

GtkListStore * gtk_list_store_string_new();
void gtk_list_store_string_prepend(GtkListStore * store,
                                   gchar        * theString);
void gtk_list_store_string_append(GtkListStore  * store,
                                  gchar         * theString);
gchar *gtk_list_store_string_get(GtkListStore   * self,
                                 GtkTreeIter    * iter);

gint gtk_tree_model_length(GtkTreeModel* self);

gboolean gtk_combo_box_next(GtkComboBox * self);
gboolean gtk_combo_box_previous(GtkComboBox * self);

gchar * g_pango_font_convert_to_css(const gchar * pango_font);

gboolean
g_settings_has_schema(const char * id);

gboolean
g_settings_has_key(const gchar * schema_id,
                    const char * key);

GValue *
g_value_new_int(int value);
GValue *
g_value_new_string(const gchar * value);
GValue *
g_value_new_boolean(gboolean value);

char *
chr_replace(char *str,
            const char search_for,
            const char replace_with);

gint
gtk_list_store_length(GtkListStore *self);

void
gtk_application_set_accel_for_action(GtkApplication *self,
                                     const gchar * detailed_action_name,
                                     const gchar * accelerator);

gboolean
delete_event_prevent_destruction(GtkWidget *widget,
                                GdkEvent  *unused1,
                                gpointer   unused2);

const gchar  *
gtk_combo_box_get_text (GtkComboBox *self);

void
gtk_combo_box_set_text (GtkComboBox *self,
                        const gchar  *text);

const char *
str_find_last_of(const char *haystack,
                 const char  needle);

#endif
