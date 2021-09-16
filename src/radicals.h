#ifndef __RADICALS_H__
#define __RADICALS_H__

#include <gtk/gtk.h>

typedef struct {
  const gchar *radkfile;
  gsize radkfile_size;

  GHashTable *kanji_info_hash;
  GHashTable *rad_info_hash;
  GList      *rad_info_list;
} Radicals;

Radicals *
radicals_new(gboolean unicode_radicals);
void
radicals_free (Radicals *self);

#endif