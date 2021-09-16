#ifndef __RADICALS_UI_H__
#define __RADICALS_UI_H__

#include <gtk/gtk.h>
#include "radicals.h"

GtkWidget *
radicals_window_new(Radicals * radicals,
                    GHashTable **rad_button_hash);


#endif

