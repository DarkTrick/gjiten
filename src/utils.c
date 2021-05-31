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


/**
 * Set css content, that will be valid for the whole application
 **/
void set_global_css(gchar     *css_class,
                    gchar     *css)
{
  // Note: It can't be good to put more and more providers
  //       onto the screen. But the user won't
  //       run this code too often, so I guess it's ok

  GtkCssProvider * cssProvider = gtk_css_provider_new();
  {
    GString * cssContent = g_string_new("");
    g_string_printf (cssContent, ".%s{%s}", css_class, css);
    gtk_css_provider_load_from_data(cssProvider, cssContent->str, cssContent->len, NULL);
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
  gtk_list_store_new (1, G_TYPE_STRING);
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
 *  Returs the stored string value
 *  Returned string has to be freed
 * `self` is a GtkListStore storing only strings and has only one column.
 **/
gchar *
gtk_list_store_string_get(GtkListStore *self,
                          GtkTreeIter  *iter)
{
  gchar *tmp;
  gtk_tree_model_get (self, iter, 0, &tmp, -1);
  return tmp;
}

gint
gtk_tree_model_length(GtkTreeModel* self)
{
  return gtk_tree_model_iter_n_children(self, NULL);
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
