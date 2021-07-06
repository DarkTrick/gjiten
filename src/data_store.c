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
 *  Settings machine layer:      gconf / gsettings / gkeyfile
 *
 **/

#include "data_store.h"
#include "gconf_reader.h"
#include <sys/stat.h> // for mkdir modes
#include "error.h"

#define return_if(expression, value) if (expression){ return value; }

#define STORE_ROOT_DCONF "net.sf.gjiten"
#define STORE_DCONF_GENERAL STORE_ROOT_DCONF
#define STORE_KANJIDIC_DCONF STORE_ROOT_DCONF ".kanjidic"
#define STORE_HISTORY_DCONF STORE_ROOT_DCONF

#define STORE_ROOT_GCONF ".gconf/apps/gjiten/"
#define STORE_GCONF_FILENAME "%gconf.xml"
#define STORE_GCONF_GENERAL STORE_ROOT_GCONF "general/" STORE_GCONF_FILENAME
#define STORE_KANJIDIC_GCONF STORE_ROOT_GCONF "kanjidic/" STORE_GCONF_FILENAME
#define STORE_HISTORY_GCONF STORE_ROOT_GCONF "history/" STORE_GCONF_FILENAME


void
_init_config_paths(DataStore *self)
{
  const gchar * user_conf = g_get_user_config_dir();
  const gchar * subpath = "/gjiten/";
  const gchar * config_filename = "gjiten.conf";

  self->config_dir  = g_build_path ("/", user_conf, subpath, NULL);
  self->config_file = g_build_path ("/", user_conf, subpath, config_filename, NULL);
}

/**
 * Initializes the paths for gconf
 *  (old configuration directory)
 **/
void
_init_gconf_paths(DataStore *self)
{
  const gchar * home_dir = g_get_home_dir();

  self->depr_config_gconf_general  = g_build_path ("/", home_dir, STORE_GCONF_GENERAL, NULL);
  self->depr_config_gconf_kanjidic = g_build_path ("/", home_dir, STORE_KANJIDIC_GCONF, NULL);
  self->depr_config_gconf_history  = g_build_path ("/", home_dir, STORE_HISTORY_GCONF, NULL);
}



void
data_store_init(DataStore *self)
{
  self->read_from_real_storage = FALSE;
  self->read_from_g_settings = FALSE;
  self->read_from_g_conf = FALSE;

  _init_config_paths (self);
  _init_gconf_paths (self);

  self->storage = g_key_file_new();
}

void
data_store_finalize(DataStore *self)
{
  g_key_file_free (self->storage);

  g_free (self->config_dir);
  g_free (self->config_file);

  g_free (self->depr_config_gconf_general);
  g_free (self->depr_config_gconf_kanjidic);
  g_free (self->depr_config_gconf_history);
}


GSList *
data_store_get_list   (DataStore   *self,
                       const gchar * section,
                       const gchar *key)
{
  g_print ("store_get_list: default value was triggered:");
  g_print ("  Key: %s\n", key);

  return NULL;
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

  GJITEN_DEBUG ("I could not find key \"%s::%s\". Use default.\n",section, key);
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


static gboolean
_store_available_gconf(DataStore *self)
{
  return g_file_test (self->depr_config_gconf_general, G_FILE_TEST_EXISTS);
}


void
_merge_gconf_to_key_file (GHashTable * hashtable,
                          GKeyFile * keyfile,
                          const gchar * section)
{
  GHashTableIter iter;
  gpointer pKey, pValue;

  g_hash_table_iter_init (&iter, hashtable);
  while (g_hash_table_iter_next (&iter, &pKey, &pValue))
  {
    const gchar * key = (const gchar *) pKey;
    GValue * gvalue = (GValue*) pValue;

    if (G_VALUE_TYPE (gvalue) == G_TYPE_BOOLEAN)
    {
      g_key_file_set_boolean (keyfile, section, key, g_value_get_boolean (gvalue));
    } else

    if (G_VALUE_TYPE (gvalue) == G_TYPE_INT)
    {
      g_key_file_set_integer (keyfile, section, key, g_value_get_int (gvalue));
    } else

    if (G_VALUE_TYPE (gvalue) == G_TYPE_FLOAT)
    {
      g_key_file_set_double (keyfile, section, key, (double)g_value_get_float (gvalue));
    } else

    if (G_VALUE_TYPE (gvalue) == G_TYPE_STRING)
    {
      const gchar * strValue = g_value_get_string (gvalue);
      g_key_file_set_string (keyfile, section, key, strValue);
    } else

    if (G_VALUE_TYPE (gvalue) == TYPE_BOXED_SLIST_OF_GVALUE)
    {
      GSList * stringlist = (GSList*)g_value_get_boxed (gvalue);
      guint list_size = g_slist_length (stringlist);

      const gchar ** stringarray = g_malloc(sizeof(gchar*) * list_size);

      guint i = 0;
      while (stringlist)
      {
        GValue * gValueValue = (GValue*)(stringlist->data);
        const gchar * strVal = g_value_get_string (gValueValue);
        stringarray[i] = strVal;
        stringlist = stringlist->next;
        i++;
      }
      g_key_file_set_string_list (keyfile, section, key, stringarray, list_size);
      g_free (stringarray);
    }
  }
}





/**
 *  This function should only be called once
 *    per instance.
 *  In case data storage migration is needed, it will
 *   do it.
 *
 *  Returns
 *    TRUE, if data could be loaded;
 *    FALSE, otherwise
 **/
gboolean
data_store_load_from_disk(DataStore *self)
{
  /**
   *  Algorithm:
   *  1. Check, if the real storage is available
   *     If no:
   *      2.1 Figure out current storage
   *            (gconf / gsettings)
   *      2.2 Read all values from there and store it to
   *          real storage
   *      2.3 Remove current storage
   *  3. Load data from real storage
   **/

  // Check if main data storage is missing
  if (FALSE == g_file_test (self->config_file, G_FILE_TEST_EXISTS))
  {
    gboolean succ = FALSE;

    // find former storages and convert
    GKeyFile * storage = g_key_file_new ();

    // 2.x load older values and store in keyfile,section,key,
    {
      //TODO:impl data migration from gsetting to keyfile
      //if (_store_available_gsettings())
      //{
      //
      //} else
      if (_store_available_gconf (self))
      {
        GJITEN_DEBUG ("Migrate gconf to gjiten.conf");

        // iterate over hashtable and insert every item into keyfile
        GHashTable * general = gconf_reader_parse_file (self->depr_config_gconf_general, NULL);
        _merge_gconf_to_key_file (general, storage, SECTION_GENERAL);
        g_hash_table_destroy (general);

        // don't care to migrate history
        g_key_file_set_string (storage, SECTION_GENERAL, "word_search_history", "");

        GHashTable * kanjidic = gconf_reader_parse_file (self->depr_config_gconf_kanjidic, NULL);
        _merge_gconf_to_key_file (kanjidic, storage, SECTION_KANJIDIC);
        g_hash_table_destroy (kanjidic);

        // TODO: remove gconf files
      }
    }

    // Save keyfile
    g_mkdir_with_parents (self->config_dir, S_IRWXU);
    succ = g_key_file_save_to_file (storage, self->config_file, NULL);
    g_key_file_free (storage);
    return_if (succ == FALSE, FALSE);
  }

  // 3. read from main data storage
  return g_key_file_load_from_file (self->storage,
                             self->config_file, G_KEY_FILE_NONE, NULL);
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
  gchar ** ret = g_key_file_get_string_list (self->storage, section, key, o_length, NULL);
  return ret;
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


  GJITEN_DEBUG ("I could not find key \"%s::%s\". Use default.\n",section, key);
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

  GJITEN_DEBUG ("I could not find key \"%s::%s\". Use default.\n",section, key);

  // return defaults
  MATCH ("maxwordmatches", 100);
  MATCH ("numofdics", 0); // prevent old style dictionary loading

  // default
  g_printerr ("Programming Error: Settings key \"%s\" does not"
              " even have a default value. Please report a bug\n", key);

  return -1;
}