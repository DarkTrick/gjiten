#include <gtk/gtk.h>

#ifndef __DATA_STORE_H
#define __DATA_STORE_H

#define STORE_ROOT       "net.sf.gjiten."
//#define STORE_ROOT_PATH  "/net/sf/gjiten/"
#define STORE_ROOT_PATH  STORE_ROOT
#define DICTPREFIX       STORE_ROOT "dic"
#define GNOMECFG         STORE_ROOT "kanjidic/"

#define SECTION_GENERAL "general"
#define SECTION_KANJIDIC "kanjidic"


// define paths to settings.
// dconf is the new one (binary based (like win registry))
// gconf is the old one (file/xml-based)

//#define REAL_STORE_BASE_DIR g_get_user_config_dir() "/gjiten"
//#define REAL_STORE_PATH g_get_user_config_dir() "/gjiten/gjiten.conf"

// This stuff is for legacy configurations




#define MATCH(KEY, VALUE) if (g_strcmp0 (key, KEY) == 0){ return VALUE; } \
                          if (g_strcmp0 (key, STORE_ROOT_PATH KEY) == 0){ return VALUE; } \
                          if (g_strcmp0 (key, GNOMECFG KEY) == 0){ return VALUE; } \





struct _DataStore{
  gboolean read_from_real_storage;
  gboolean read_from_g_settings;
  gboolean read_from_g_conf;

  gchar * depr_config_gconf_general;
  gchar * depr_config_gconf_kanjidic;
  gchar * depr_config_gconf_history;

  gchar * config_dir;
  gchar * config_file;

  GKeyFile * storage;
};
typedef struct _DataStore DataStore;

void
data_store_init(DataStore *self);

void
data_store_finalize(DataStore *self);
GSList *
data_store_get_list   (DataStore   *self,
                    const gchar * section,
                       const gchar *key);

                       gboolean
data_store_get_boolean(DataStore   *self,
                    const gchar * section,
                       const gchar *key);

gboolean
data_store_load_from_disk(DataStore * self);

gboolean
data_store_save_to_disk(DataStore * self);

gchar *
data_store_get_string (DataStore   *self,
                    const gchar * section,
                       const gchar * key);
gchar **
data_store_get_string_array (DataStore   * self,
                            const gchar * section,
                            const gchar * key,
                            gsize       * o_length);

gint
data_store_get_int (DataStore   * self,
                    const gchar * section,
                    const gchar * key);



#endif