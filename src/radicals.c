#include "radicals.h"

#include "constants.h"
#include "kanjidic.h"
#include "utils.h"
#include "dicutil.h"
#include "dicfile.h"
#include "radical-convtable.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>




static gunichar
_jis_radical_to_unicode(const gchar *radical, gboolean unicode_radicals)
{
  gint i;

  gchar jisutf8[6];

  if (unicode_radicals == TRUE) {
    g_unichar_to_utf8(g_utf8_get_char (radical), jisutf8);

    for (i = 0; i < sizeof (radicaltable) / sizeof (radpair); i++) {
      if (strcmp (radicaltable[i].jis, jisutf8) == 0) {
        return g_utf8_get_char (radicaltable[i].uni);
      }
    }
  }

  return g_utf8_get_char (radical);
}


static void
_radical_hashtables_init(Radicals *self, gboolean unicode_radicals)
{
  int error = FALSE;
  struct stat radk_stat;
  gint rad_cnt = 0;
  const gchar *radkfile_ptr;
  const gchar *radkfile_end;
  int fd = 0;
  RadInfo *rad_info = NULL;
  KanjiInfo *kanji_info;
  gunichar kanji;

  radkfile_end = self->radkfile + self->radkfile_size;
  radkfile_ptr = self->radkfile;

  if (self->kanji_info_hash != NULL ||
      self->rad_info_hash   != NULL)
    return;

  self->kanji_info_hash = g_hash_table_new (NULL, NULL);
  self->rad_info_hash = g_hash_table_new (NULL, NULL);


  while ((radkfile_ptr < radkfile_end) && (radkfile_ptr != NULL))
  {
    if (*radkfile_ptr == '#') //find $ as first char on the line
    {
      radkfile_ptr = get_eof_line (radkfile_ptr, radkfile_end); //Goto next line
      continue;
    }

    //Radical info line ?
    if (*radkfile_ptr == '$') {
      rad_cnt++;          //Increase number of radicals found
      radkfile_ptr = g_utf8_next_char (radkfile_ptr);
      while (g_unichar_iswide (g_utf8_get_char (radkfile_ptr)) == FALSE) { //Find radical
        radkfile_ptr = g_utf8_next_char (radkfile_ptr);
      }

      rad_info = g_new0(RadInfo, 1);
      rad_info->radical = _jis_radical_to_unicode (radkfile_ptr, unicode_radicals); //store radical

      while (g_ascii_isdigit (*radkfile_ptr) == FALSE) { //Find stroke number
        radkfile_ptr = g_utf8_next_char (radkfile_ptr);
      }

      rad_info->strokes = atoi (radkfile_ptr);  //Store the stroke number
      self->rad_info_list = g_list_append (self->rad_info_list, rad_info);
      g_hash_table_insert (self->rad_info_hash, TO_POINTER(rad_info->radical), rad_info);
      radkfile_ptr = get_eof_line (radkfile_ptr, radkfile_end); //Goto next line
    }
    //Kanji info line ?
    else
    {
      while ((*radkfile_ptr != '$') && (radkfile_ptr < radkfile_end))
      {
        if (*radkfile_ptr == '\n')
        {
          radkfile_ptr++;
          continue;
        }
        kanji = g_utf8_get_char (radkfile_ptr);
        kanji_info = g_hash_table_lookup (self->kanji_info_hash, TO_CONST_POINTER (kanji));
        if (kanji_info == NULL)
        {
          kanji_info = g_new0(KanjiInfo, 1);
          kanji_info->kanji = kanji;
          g_hash_table_insert (self->kanji_info_hash, TO_POINTER (kanji), TO_POINTER (kanji_info));
        }
        kanji_info->rad_info_list = g_list_prepend (kanji_info->rad_info_list, rad_info);
        rad_info->kanji_info_list = g_list_prepend (rad_info->kanji_info_list, kanji_info);
        radkfile_ptr = g_utf8_next_char (radkfile_ptr);
      }
    }
  }
}


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


void
radicals_free (Radicals *self)
{
  if (self->kanji_info_hash)
    g_hash_table_destroy (self->kanji_info_hash);
  if (self->rad_info_hash)
    g_hash_table_destroy (self->rad_info_hash);

  g_list_free_full (self->rad_info_list, free);

  g_free (self);
}



Radicals *
radicals_new(gboolean unicode_radicals)
{
  Radicals * self = g_new0(Radicals, 1);

  _load_radk_file (self);
  _radical_hashtables_init (self, unicode_radicals);

  return self;
}

