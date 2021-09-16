#ifndef __RADICALS_H__
#define __RADICALS_H__

#include <gtk/gtk.h>

typedef struct {
  const gchar *radkfile;
  gsize radkfile_size;
} Radicals;

Radicals *
radicals_new();

#endif