#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>

// TODO:remove after _()-function works properly!
const char * _(const char* a);

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

#endif