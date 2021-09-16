#ifndef __RADICALS_UI_H__
#define __RADICALS_UI_H__

#include <gtk/gtk.h>
#include "radicals.h"
#include "kanjidic.h"

GtkWidget *
radicals_window_new(GjKanjidicWindow* kanjidic_window,
                    Radicals * radicals,
                    GHashTable **rad_button_hash);


#endif

