#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <locale.h>

// Why these macros?
// Casts without `uintptr_t` raise a compiler warning.
// These warning should not throw, as the consequences of
//  the cast are known.
#define TO_POINTER(X) (gpointer)(uintptr_t)(X)
#define TO_CONST_POINTER(X) (gconstpointer)(uintptr_t)(X)
#define POINTER_TO_UNICHAR(X) (gunichar)(uintptr_t)(X)
#define POINTER_TO_UCHAR32(X) (guint32)(uintptr_t)(X)

#define GJITEN_WINDOW_ICON PIXMAPDIR"/jiten.png"

GtkToolButton* gj_container_append_stock(GtkContainer *toolbar,
                                        const char *icon_name,
                                        const char *tooltip_text,
                                        const char *tooltip_private_text,
                                        GCallback   callback_nullable,
                                        gpointer    user_data_nullable);

GtkToolButton* gj_container_append_item(GtkContainer *toolbar,
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


gint
gtk_combo_box_length(GtkComboBox * self);

gboolean
gj_gtk_window_close_on_focus_out(GtkWidget *window,
                                 GdkEvent  *event,
                                 gpointer   unused);

gboolean
gj_gtk_window_close_on_escape(GtkWidget   *window,
                              GdkEventKey *event,
                              gpointer     unused);

void
gj_enable_quick_lookup_mode(GtkWindow * window);

void
g_menu_item_paste_clicked(GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       gtk_application);

void
g_menu_item_copy_clicked(GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       gtk_application);
void
g_menu_setup_default_actions_copy_paste(GtkWindow *window);

gboolean
gx_utf8_validate(const gchar *str_nullable,
                 gssize max_len,
                 const gchar **end);


GtkWidget *
gj_toolbutton_image_new_from_icon_name (const char *name);

void
gj_paned_handle_hide (GtkWidget *handle);

#endif
