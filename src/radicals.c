#include "radicals.h"

#include "constants.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


static gboolean
_load_radkfile_from_file(Radicals * self)
{
  int error = FALSE;
  struct stat radk_stat;
  gchar *radkfile_name = RADKFILE_NAME;
  int fd = 0;

  if (stat (radkfile_name, &radk_stat) != 0) {
    return FALSE;
  }
  self->radkfile_size = radk_stat.st_size;
  fd = open (radkfile_name, O_RDONLY);
  if (fd == -1) {
    return FALSE;
  }
  self->radkfile = (gchar *) mmap (NULL, self->radkfile_size, PROT_READ, MAP_SHARED, fd, 0);
  if (self->radkfile == NULL){
    g_print ("Could not load radical file from %s\n", RADKFILE_NAME);
    return FALSE;
  }

  return TRUE;
}




static void
_load_radkfile_from_resource(Radicals * self)
{
  GBytes * bytes = g_resources_lookup_data (RADKFILE_RESOURCE, 0, NULL);
  self->radkfile = g_bytes_get_data (bytes, &(self->radkfile_size));
}



static void
_load_radk_file(Radicals * self)
{
  gboolean initialized = FALSE;
  initialized = _load_radkfile_from_file (self);

  if (!initialized)
  {
    _load_radkfile_from_resource (self);
  }
}



Radicals *
radicals_new()
{
  Radicals * self = g_new0(Radicals, 1);

  _load_radk_file (self);

  return self;
}

