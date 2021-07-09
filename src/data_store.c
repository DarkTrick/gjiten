/**
 * Abstraction layer for persistent-data access.
 *  In this file, access to gconf / gsettings /
 *  keyfile / hardcoded-value is done. So conf.c
 *  does not need to care where the data actually came from

 *  Structure
 * ============
 *
 *  Domain code:                    gjiten / worddic / ....
 *
 *  Settings domain layer:              conf.c
 *
 *  Settings technical layer:       data_store.c  (this)
 *
 *  Settings machine layer:            gkeyfile
 *
 *
 *  Why do we use keyfiles?
 * ========================
 *  - gconf is deprecated
 *  - gsettings work, but
 *      - their documentation is too bad.
 *      - their setup is too tedious.
 *      - you don't know when they become deprecated
 *         (as happened with gconf).
 *      - dconf is very difficult to use (manually change,
 *         navigate, copy information), because tools are
 *         insufficient.
 *      - we don't need their functionalities.
 *
 **/

#include "data_store.h"
#include "gconf_reader.h"
#include <sys/stat.h> // for mkdir modes
#include "error.h"
#include "utils.h"
#include "migration2-6_3-0.h"

#define return_if(expression, value) if (expression){ return value; }


void
_init_config_paths(DataStore *self)
{
  const gchar * user_conf = g_get_user_config_dir();
  const gchar * subpath = "/gjiten/";
  const gchar * config_filename = "gjiten.conf";

  self->config_dir  = g_build_path ("/", user_conf, subpath, NULL);
  self->config_file = g_build_path ("/", user_conf, subpath, config_filename, NULL);
}



void
data_store_init(DataStore *self)
{
  self->read_from_real_storage = FALSE;
  self->read_from_g_settings = FALSE;
  self->read_from_g_conf = FALSE;

  _init_config_paths (self);

  self->storage = g_key_file_new();
}

void
data_store_finalize(DataStore *self)
{
  g_key_file_free (self->storage);

  g_free (self->config_dir);
  g_free (self->config_file);
}


gboolean
data_store_get_boolean(DataStore   *self,
                       const gchar * section,
                       const gchar *key)
{
  GError * error = NULL;
  gboolean ret = g_key_file_get_boolean (self->storage, section, key, &error);
  if (NULL == error)
    return ret;

  // return defaults

  // gjiten
  MATCH ("autoadjust_enabled", TRUE);
  MATCH ("bigkanji", FALSE);
  MATCH ("bigwords", FALSE);
  MATCH ("deinflection_enabled", FALSE);
  MATCH ("envvar_override", FALSE);
  MATCH ("force_ja_JP", FALSE);
  MATCH ("force_language_c", FALSE);
  MATCH ("gdk_use_xft", FALSE);
  MATCH ("search_hira_on_kata", FALSE);
  MATCH ("search_kata_on_hira", TRUE);
  MATCH ("searchlimit_enabled", FALSE);

  MATCH ("toolbar", TRUE);
  MATCH ("menubar", TRUE);

  // kanjidict
  MATCH ("bushu", FALSE);
  MATCH ("classic", FALSE);
  MATCH ("cref", FALSE);
  MATCH ("deroo", FALSE);
  MATCH ("eindex", FALSE);
  MATCH ("english", TRUE);
  MATCH ("fourc", FALSE);
  MATCH ("freq", TRUE);
  MATCH ("hindex", FALSE);
  MATCH ("iindex", FALSE);
  MATCH ("jisascii", FALSE);
  MATCH ("jouyou", TRUE);
  MATCH ("kanji", TRUE);
  MATCH ("kindex", FALSE);
  MATCH ("korean", TRUE);
  MATCH ("lindex", FALSE);
  MATCH ("missc", FALSE);
  MATCH ("mnindex", FALSE);
  MATCH ("mpindex", FALSE);
  MATCH ("nindex", FALSE);
  MATCH ("oindex", FALSE);
  MATCH ("pinyin", TRUE);
  MATCH ("radical", TRUE);
  MATCH ("reading", TRUE);
  MATCH ("skip", FALSE);
  MATCH ("strokes", TRUE);
  MATCH ("unicode", FALSE);
  MATCH ("unicode_radicals", TRUE);
  MATCH ("vindex", FALSE);

  // return default for unknown option
  g_printerr ("Programming Error: Settings key \"%s\" does not"
              " even have a default value. Please report a bug\n", key);

  return FALSE;
}



gboolean
keyfile_save(GKeyFile    *storage,
             const gchar *target_dir,
             const gchar *target_file)
{
  // Save keyfile
  g_mkdir_with_parents (target_dir, S_IRWXU);
  gboolean succ = g_key_file_save_to_file (storage, target_file, NULL);
  return succ;
}


static gboolean
_data_store_initialize_storage_file(DataStore *self)
{
  // setup
  gboolean succ = FALSE;
  GKeyFile * storage = g_key_file_new ();

  // run
  g_key_file_set_string (storage, SECTION_GENERAL, "version", "3.0");
  succ = keyfile_save (storage, self->config_dir, self->config_file);

  // cleanup
  g_key_file_free (storage);

  return succ;
}



/**
 * Make sure there's a config file, we can open
 *
 *  Returns
 *    TRUE, if data could be loaded;
 *    FALSE, otherwise
 **/
gboolean
create_config_file_if_necessary(DataStore *self)
{
  // Check if main data storage is missing
  if (FALSE == g_file_test (self->config_file, G_FILE_TEST_EXISTS))
  {
    gboolean succ = FALSE;

    if (migration_possible_2_6_to_3_0())
      return migrate_2_6_to_3_0 (self->config_dir, self->config_file);

    else
      _data_store_initialize_storage_file (self);
  }

  return TRUE;
}




/**
 *  This function should only be called once
 *    per instance.
 *  In case data storage migration is needed, it will
 *   do it.
 *
 **/
void
data_store_load_from_disk(DataStore *self)
{
  create_config_file_if_necessary (self);
  g_key_file_load_from_file (self->storage,
                             self->config_file,
                             G_KEY_FILE_NONE, NULL);
}



gboolean
data_store_save_to_disk(DataStore * self)
{
  return FALSE;
}

/**
 *  Return
 *    value must be freed
 **/
gchar **
data_store_get_string_array (DataStore   * self,
                            const gchar * section,
                            const gchar * key,
                            gsize       * o_length)
{
  GError * error = NULL;
  gchar ** ret = g_key_file_get_string_list (self->storage, section, key, o_length, &error);
  if (NULL == error)
    return ret;

  // return default values
  gchar * dict_list_default[] = {"/usr/share/gjiten/dics/edict\nEnglish-main",NULL};
  MATCH ("dictionary_list", g_strdupv (dict_list_default));

  // TODO: do we need to add default edict path?

  return NULL;
}


/**
 *  Persistent store interface functions
 *  (currently mocked)
 *
 *  Return
 *    value must be freed
 **/
gchar *
data_store_get_string (DataStore   *self,
                       const gchar * section,
                       const gchar * key)
{
  GError * error = NULL;
  gchar * ret = g_key_file_get_string (self->storage, section, key, &error);
  if (NULL == error)
    return ret;


  // return defaults

  MATCH("dictpath", g_strdup ("/usr/share/gjiten/dics/"))
  MATCH("kanjipad", g_strdup ("/usr/bin/kanjipad"))
  MATCH("largefont", g_strdup ("Sans 14"))
  MATCH("normalfont", g_strdup ("Sans 22"))
  MATCH("version", g_strdup ("3.0"))

  MATCH ("kanjidicfile", g_strdup ("/usr/share/gjiten/dics/kanjidic"));

  #define HISTORY(N) MATCH ("history" N, g_strdup("testhistory" N));
  HISTORY ("0"); HISTORY ("1"); HISTORY ("2"); HISTORY ("3"); HISTORY ("4"); HISTORY ("5"); HISTORY ("6");
  HISTORY ("7"); HISTORY ("8"); HISTORY ("9"); HISTORY ("10"); HISTORY ("11");
  HISTORY ("12"); HISTORY ("13"); HISTORY ("14"); HISTORY ("15");
  HISTORY ("16"); HISTORY ("17"); HISTORY ("18"); HISTORY ("19");
  HISTORY ("20"); HISTORY ("21"); HISTORY ("22"); HISTORY ("23");
  HISTORY ("24"); HISTORY ("25"); HISTORY ("26"); HISTORY ("27");
  HISTORY ("28"); HISTORY ("29"); HISTORY ("30"); HISTORY ("31");
  HISTORY ("32"); HISTORY ("33"); HISTORY ("34"); HISTORY ("35");
  HISTORY ("36"); HISTORY ("37"); HISTORY ("38"); HISTORY ("39");
  HISTORY ("40"); HISTORY ("41"); HISTORY ("42"); HISTORY ("43");
  HISTORY ("44"); HISTORY ("45"); HISTORY ("46"); HISTORY ("47");
  HISTORY ("48"); HISTORY ("49"); HISTORY ("50")
  #undef HISTORY

  // default
  g_printerr ("Programming Error: Settings key \"%s\" does not"
              " even have a default value. Please report a bug\n", key);

  return g_strdup("");
}



gint
data_store_get_int (DataStore   * self,
                    const gchar * section,
                    const gchar * key)
{
  GError * error = NULL;
  gint ret = g_key_file_get_integer (self->storage, section, key, &error);
  if (NULL == error)
    return ret;


  // return defaults
  MATCH ("maxwordmatches", 100);
  MATCH ("numofdics", 0); // prevent old style dictionary loading

  // default
  g_printerr ("Programming Error: Settings key \"%s\" does not"
              " even have a default value. Please report a bug\n", key);

  return -1;
}