#include "utils.h"
#include <string.h>
#include <gtk/gtk.h>



/**
 *  Reimplementation of original function from gtk 2
 **/
GtkToolButton*
gj_container_append_stock (GtkContainer *toolbar,
                          const char *icon_name,
                          const char *tooltip_text,
                          const char *tooltip_private_text,
                          GCallback callback_nullable,
                          gpointer user_data_nullable)
{
  GtkWidget     *icon;
  GtkToolButton *button;

  icon = gtk_image_new_from_icon_name (icon_name, 20);
  button = GTK_TOOL_BUTTON (gtk_tool_button_new (icon, tooltip_private_text));
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (button), tooltip_text);

  gtk_container_add (toolbar, GTK_WIDGET (button));

  if (callback_nullable){
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (callback_nullable), user_data_nullable);
  }

  gtk_widget_set_size_request (GTK_WIDGET (button), 32, 32);

  return button;
}



/**
 *  Reimplementation of original function from gtk 2
 **/
GtkToolButton*
gj_container_append_item(GtkContainer *toolbar,
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

  gtk_container_add (toolbar, GTK_WIDGET (button));

  if (callback_nullable){
    g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (callback_nullable), user_data_nullable);
  }
  gtk_widget_set_size_request (GTK_WIDGET (button), 32, 32);
  return button;
}



/**
 *  allow clearing an entry as callback
 **/
void
gtk_entry_clear_callback(gpointer entrybox)
{
  g_return_if_fail (GTK_IS_ENTRY (entrybox));
  gtk_entry_set_text (GTK_ENTRY (entrybox), "");
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
    gtk_window_set_icon_from_file (window, icon_path, NULL);
  }
}



/**
 * Set css content, that will be valid for the whole application
 **/
void set_global_css(gchar     *css_class,
                    gchar     *css)
{
  // Note: It can't be good to put more and more providers
  //       onto the screen. But the user won't
  //       run this code too often, so I guess it's ok

  GtkCssProvider * cssProvider = gtk_css_provider_new ();
  {
    GString * cssContent = g_string_new("");
    g_string_printf (cssContent, ".%s{%s}", css_class, css);
    gtk_css_provider_load_from_data (cssProvider, cssContent->str, cssContent->len, NULL);
    g_string_free (cssContent, TRUE);
  }

  GdkScreen * screen = gdk_screen_get_default ();
  gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (cssProvider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
}



/**
 *  Adds a css class `css_class` to `widget`.
 **/
void gtk_widget_style_add_class(GtkWidget   *widget,
                                const gchar *css_class)
{
  GtkStyleContext * context = gtk_widget_get_style_context (widget);
  gtk_style_context_add_class (context, css_class);
}



GtkListStore *
gtk_list_store_string_new()
{
  return gtk_list_store_new (1, G_TYPE_STRING);
}



/**
 * Prepends `string` onto `store`.
 * `store` is a GtkListStore storing only strings and has only one column.
 **/
void
gtk_list_store_string_prepend(GtkListStore * store,
                              gchar * theString)
{
  GtkTreeIter iter;
  gtk_list_store_prepend (store, &iter);
  gtk_list_store_set (store, &iter, 0, theString, -1);
}




/**
 * Appends `string` onto `store`.
 * `self` is a GtkListStore storing only strings and has only one column.
 **/
void
gtk_list_store_string_append(GtkListStore * self,
                              gchar * theString)
{
  GtkTreeIter iter;
  gtk_list_store_append (self, &iter);
  gtk_list_store_set (self, &iter, 0, theString, -1);
}



/**
 *  Returns stored string value
 *
 * Params:
 *  `self` is a GtkListStore storing only strings and has only one column.
 *
 * Returns
 *    string has to be freed
 **/
gchar *
gtk_list_store_string_get(GtkListStore *self,
                          GtkTreeIter  *iter)
{
  gchar *tmp;
  gtk_tree_model_get (GTK_TREE_MODEL (self), iter, 0, &tmp, -1);
  return tmp;
}



gint
gtk_list_store_length(GtkListStore *self)
{
  return gtk_tree_model_length (GTK_TREE_MODEL (self));
}



gint
gtk_tree_model_length(GtkTreeModel* self)
{
  return gtk_tree_model_iter_n_children (self, NULL);
}



gint
gtk_combo_box_length(GtkComboBox * self)
{
  return gtk_tree_model_length (gtk_combo_box_get_model (self));
}



gboolean
gtk_combo_box_next(GtkComboBox * self)
{
  gint index = gtk_combo_box_get_active (self);
  if (gtk_tree_model_length (gtk_combo_box_get_model (self))){
    gtk_combo_box_set_active (self, index + 1);
    return TRUE;
  }
  return FALSE;
}



gboolean
gtk_combo_box_previous(GtkComboBox * self)
{
  gint index = gtk_combo_box_get_active (self);
  if (index > -1)
  {
    gtk_combo_box_set_active (self, index - 1);
    return TRUE;
  }
  return FALSE;
}



/**
 *   Converts a pango font string (E.g. "Sans 14")
 *   to a CSS font string (E.g. "14px Sans").
 *
 *   Return value has to be freed with g_free.
 **/
gchar *
g_pango_font_convert_to_css(const gchar * pango_font)
{
  gchar * font_size   = NULL;
  gchar * font_family = NULL;
  gchar * css_font    = NULL;

  //we need a modifyable string
  gchar * pango_font_m = g_strdup (pango_font);

  char * last_space = (char*)str_find_last_of (pango_font_m, ' ');
  last_space[0] = '\0';

  font_family = pango_font_m;
  font_size   = last_space+1;

  GString * css = g_string_new ("");
  g_string_printf (css, "%spx %s", font_size, font_family);

  css_font = css->str;
  g_free (pango_font_m);
  g_string_free (css, FALSE);

  return css_font;
}



/**
 * Check if a schema exist.
 * This is important, because g_settings_new will core dump,
 * if the id does not exist
 **/
gboolean
g_settings_has_schema (const char * id)
{
  GSettingsSchema * res = g_settings_schema_source_lookup (
                                 g_settings_schema_source_get_default (),
                                 id, FALSE);
  gboolean ret = FALSE;
  if (res != NULL)
  {
    ret = TRUE;
    g_settings_schema_unref (res);
  }

  return ret;
}



gboolean
g_settings_has_key (const gchar * schema_id,
                    const char * key)
{
  GSettingsSchema * schema = g_settings_schema_source_lookup (
                              g_settings_schema_source_get_default (),
                              schema_id, FALSE);
  if (schema)
  {
    gboolean has_key = g_settings_schema_has_key (schema, key);
    return has_key;
  }

  return FALSE;
}



/**
 * Changes the string in place (only even-byte-sized chars!)
 * `str` must be null-terminated.
 *
 * Return
 *  `str`
 **/
char *
chr_replace (char *str,
              const char search_for,
              const char replace_with)
{
  for (int i=0; str[i] != '\0'; ++i)
  {
    if (search_for == str[i])
      str[i] = replace_with;
  }
  return str;
}


void
gtk_application_set_accel_for_action (GtkApplication *self,
                                      const gchar * detailed_action_name,
                                      const gchar * accelerator)
{
  const gchar * const shortcuts[] = {accelerator, NULL};
  gtk_application_set_accels_for_action (self, detailed_action_name, shortcuts);
}



/**
 * If this function is connected to GtkWidget::`delete-event`,
 * the widget will not be `gtk_widget_destroy`ed
 **/
gboolean
delete_event_prevent_destruction(GtkWidget *widget,
                                GdkEvent  *unused1,
                                gpointer   unused2)
{
  return TRUE;
}



void
gtk_combo_box_set_text (GtkComboBox *self,
                        const gchar  *text)
{
  GtkEntry * entry = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (self)));
  gtk_entry_set_text (entry, text);
}



const gchar  *
gtk_combo_box_get_text (GtkComboBox *self)
{
  GtkEntry * entry = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (self)));
  return gtk_entry_get_text (entry);
}



/**
 * Find `needle` in `haystack`. This function does not depend
 * on integer sizes.
 *
 * Params:
 *        \0-terminated strings.
 *        `needle` cannot be '\0'.
 *
 * Return: pointer to last occurance of `needle` in haystack.
 *         0, if nothing was found
 **/
const char *
str_find_last_of(const char *haystack,
                 const char  needle)
{
  if (haystack[0] == '\0'){
    return (const char*)(0);
  }

  const char * cur = haystack;
  const char * last_location = haystack;
  int found = 0;

  // Check each single character...
  while (cur[0] != '\0')
  {
    // ... if it matches the `needle`, store the location...
    if (cur[0] == needle)
    {
      last_location = cur;
      found = 1;
    }
    ++cur;
  }

  // ... and return the last stored location.
  if (1 == found)
    return last_location;

  // Otherwise return 0.
  return (const char*)(0);
}



gboolean
gj_gtk_window_close_on_escape(GtkWidget   *window,
                              GdkEventKey *event,
                              gpointer     unused)
{
  if (event->keyval == GDK_KEY_Escape)
  {
    gtk_widget_destroy (window);
    return TRUE;
  }
  return FALSE;
}



gboolean
gj_gtk_window_close_on_focus_out(GtkWidget *window,
                                 GdkEvent  *event,
                                 gpointer   unused)
{
  // Problems:
  //  - window will close, if comboboxes are opened
  //  - window will close, if user moves it
  //  Fix these problems, if there is an answer here:
  //  https://discourse.gnome.org/t/how-to-detect-if-the-user-left-the-application/7492
  //   ( How to detect, if the user left the application? )
  gtk_widget_destroy (window);
  return TRUE;
}



/**
 * Enables quick lookup mode for `window`.
 *
 * Quick lookup mode is currently defined as:
 *  - Terminate application on ESC
 *  - Terminate application on unfocus window
 **/
void
gj_enable_quick_lookup_mode(GtkWindow *window)
{
  g_signal_connect (G_OBJECT (window), "focus-out-event",
                    G_CALLBACK (gj_gtk_window_close_on_focus_out), NULL);
  g_signal_connect (G_OBJECT (window), "key-press-event",
                  G_CALLBACK (gj_gtk_window_close_on_escape), NULL);
}



void
g_menu_item_copy_clicked(GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       gtk_window)
{
  GtkWindow * window = GTK_WINDOW (gtk_window);
  if (NULL == window)
    return;

  GtkWidget * focus = gtk_window_get_focus (window);
  if (NULL == focus)
    return;

  if (GTK_IS_EDITABLE (focus) || GTK_IS_TEXT_VIEW (focus))
  {
    g_signal_emit_by_name (focus, "copy-clipboard", NULL);
  }
}



void
g_menu_item_paste_clicked(GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       gtk_window)
{
  GtkWindow * window = GTK_WINDOW (gtk_window);
  if (NULL == window)
    return;

  GtkWidget * focus = gtk_window_get_focus (window);
  if (NULL == focus)
    return;

  if (GTK_IS_EDITABLE (focus) || GTK_IS_TEXT_VIEW (focus))
  {
    g_signal_emit_by_name (focus, "paste-clipboard", NULL);
  }
}


void
g_menu_setup_default_actions_copy_paste(GtkWindow *window)
{
  GActionEntry actions[] = {
    {.name="copy",  .activate=g_menu_item_copy_clicked, NULL, NULL, NULL },
    {.name="paste",  .activate=g_menu_item_paste_clicked, NULL, NULL, NULL },
  };

  GSimpleActionGroup *action_group = g_simple_action_group_new ();
  g_action_map_add_action_entries (G_ACTION_MAP (action_group), actions, G_N_ELEMENTS (actions), window);
  gtk_widget_insert_action_group (GTK_WIDGET (window), "window", G_ACTION_GROUP (action_group));
}


/**
 * like g_utf8_validate, but it
 * also checks, if str is NULL
 **/
gboolean
gx_utf8_validate(const gchar *str_nullable,
                 gssize max_len,
                 const gchar **end)
{
  if (NULL == str_nullable) return FALSE;

  return g_utf8_validate (str_nullable, max_len, end);
}


GtkWidget *
gj_toolbutton_image_new_from_icon_name (const char *name)
{
  GtkWidget * tmp = gtk_image_new_from_icon_name (name, GTK_ICON_SIZE_INVALID /*ignored*/);
  gtk_image_set_pixel_size (GTK_IMAGE (tmp), 28);
  return tmp;
}



void
gtkx_widget_css_class_add (GtkWidget  *widget,
                         const gchar *css_class_name)
{
  GtkStyleContext * context = gtk_widget_get_style_context (widget);
  gtk_style_context_add_class (context, css_class_name);
}