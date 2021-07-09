/**
 *
 * This file contains migration code from Version 2.6 to version 3.0
 *
 **/


#include <gtk/gtk.h>
#include "gconf_reader.h"

#include "utils.h"
#include "error.h"
#include "data_store.h"

#define STORE_ROOT_DCONF "net.sf.gjiten"
#define STORE_DCONF_GENERAL STORE_ROOT_DCONF
#define STORE_KANJIDIC_DCONF STORE_ROOT_DCONF ".kanjidic"
#define STORE_HISTORY_DCONF STORE_ROOT_DCONF

#define STORE_ROOT_GCONF ".gconf/apps/gjiten/"
#define STORE_GCONF_FILENAME "%gconf.xml"
#define STORE_GCONF_GENERAL STORE_ROOT_GCONF "general/" STORE_GCONF_FILENAME
#define STORE_KANJIDIC_GCONF STORE_ROOT_GCONF "kanjidic/" STORE_GCONF_FILENAME
#define STORE_HISTORY_GCONF STORE_ROOT_GCONF "history/" STORE_GCONF_FILENAME



struct _MigrationData {
  gchar * gconfig_path_general;
  gchar * gconfig_path_kanjidic;
  gchar * gconfig_path_history;
};
typedef struct _MigrationData MigrationData;

static gchar *
get_gconf_path_general()
{
  const gchar * home_dir = g_get_home_dir();
  return g_build_path ("/", home_dir, STORE_GCONF_GENERAL, NULL);
}



void
migration_data_init(MigrationData *self)
{
  const gchar * home_dir = g_get_home_dir();
  self->gconfig_path_general  = get_gconf_path_general();
  self->gconfig_path_kanjidic = g_build_path ("/", home_dir, STORE_KANJIDIC_GCONF, NULL);
  self->gconfig_path_history  = g_build_path ("/", home_dir, STORE_HISTORY_GCONF, NULL);
}



void
migration_data_finalize(MigrationData *self)
{
  g_free (self->gconfig_path_general);
  g_free (self->gconfig_path_kanjidic);
  g_free (self->gconfig_path_history);
}



static gboolean
_keyfile_save_new(GKeyFile    *storage,
                 const gchar *target_dir,
                 const gchar *target_file)
{
  // update version info
  g_key_file_set_string (storage, SECTION_GENERAL, "version", "3.0");

  // Save keyfile
  return keyfile_save (storage, target_dir, target_file);
}



static gboolean
_store_available_gconf()
{
  gboolean ret = FALSE;
  gchar * gconf_path = get_gconf_path_general();
  ret = g_file_test (gconf_path, G_FILE_TEST_EXISTS);

  g_free (gconf_path);

  return ret;
}


/**
 *
 * Return TRUE, if everything went fine
 *        FALSE, if an error occured
 **/
static gboolean
_move_gconf_hashtable_to_keyfile (GHashTable * hashtable,
                                  GKeyFile * keyfile,
                                  const gchar * section)
{
  GHashTableIter iter;
  gpointer pKey, pValue;

  if (NULL == hashtable) return FALSE;

  // iterate over hashtable and insert every item into keyfile
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

  return TRUE;
}



/**
 * Merge all int/bool/string values from
 *  gsettings into keyfile.
 * This will not merge lists!
 **/
static void
_load_gsettings(const gchar *schema_id,
                GKeyFile    *keyfile,
                const gchar *section)
{
  // get list of all keys
  gchar ** keys = NULL;
  {
    GSettingsSchema * schema = g_settings_schema_source_lookup (
                                      g_settings_schema_source_get_default(),
                                      schema_id, FALSE);
    keys = g_settings_schema_list_keys (schema);
    g_settings_schema_unref (schema);
  }


  {
    GSettings * gsettings = g_settings_new (schema_id);
    for (int i = 0; keys[i] != NULL; ++i)
    {
      gchar * key = keys[i];
      GVariant * variant = g_settings_get_value (gsettings, keys[i]);
      const GVariantType * type = g_variant_get_type (variant);

      // prepare naming for keyfile
      chr_replace (key, '-', '_');

      if (g_variant_type_equal (G_VARIANT_TYPE_BOOLEAN, type))
        g_key_file_set_boolean (keyfile, section, key, g_variant_get_boolean (variant));
      else if (g_variant_type_equal (G_VARIANT_TYPE_STRING, type))
        g_key_file_set_string (keyfile, section, key, g_variant_get_string (variant, NULL));
      else if (g_variant_type_equal (G_VARIANT_TYPE_INT32, type))
        g_key_file_set_integer (keyfile, section, key, g_variant_get_int32 (variant));
    }
    g_object_unref (gsettings);
  }

  g_strfreev (keys);
}



static gboolean
_store_available_gsettings()
{
  return g_settings_has_schema (STORE_ROOT_DCONF);
}


static void
_load_gsettings_dictionary_list (MigrationData *self,
                                               GSettings     *gsettings,
                                               GKeyFile      *storage)
{
  GVariant * variant = g_settings_get_value (gsettings, "dictionary-list");
  gsize num_dicts = g_variant_n_children (variant);

  gchar ** dictionary_array = g_malloc (sizeof(gchar*) *(num_dicts+1));

  // read each dictionary information
  //    and store in array
  for (int i=0; i<num_dicts; ++i)
  {
    GVariant * dictEntry = g_variant_get_child_value (variant, i);
    GVariant * varPath = g_variant_get_child_value (dictEntry, 0);
    GVariant * varName = g_variant_get_child_value (dictEntry, 1);

    const gchar * strPath = g_variant_get_string (varPath, NULL);
    const gchar * strName = g_variant_get_string (varName, NULL);

    dictionary_array[i] = g_strconcat (strPath, "\n", strName, NULL);
  }

  // NULL-terminate list
  dictionary_array[num_dicts] = NULL;

  // store data in keyfile
  g_key_file_set_string_list (storage,
                              SECTION_GENERAL,
                              "dictionary_list",
                              (const char * const *)dictionary_array,
                              num_dicts);

  // free list of dictionaries
  g_strfreev (dictionary_array);
}



static void
_load_gsettings_history(MigrationData *self,
                                      GSettings     *gsettings,
                                      GKeyFile      *storage)
{
  GVariant * variant = g_settings_get_value (gsettings, "history");
  gsize count = g_variant_n_children (variant);

  // create string array from values
  gchar ** string_array = NULL;
  {
    string_array = g_malloc (sizeof(gchar*) *(count+1));
    for (int i=0; i<count; ++i)
    {
      GVariant * entry = g_variant_get_child_value (variant, i);
      string_array[i] = g_strdup (g_variant_get_string (entry, NULL));
    }
    string_array[count] = NULL;
  }
  // store string array in keyfile
  g_key_file_set_string_list (storage,
                              SECTION_GENERAL,
                              "word_search_history",
                              (const char * const *)string_array,
                              count);
  // free array
  g_strfreev (string_array);
}



static gboolean
_migrate_gsettings(MigrationData *self,
                   const gchar   *new_storage_dir,
                   const gchar   *new_storage_file)
{
  GJITEN_DEBUG ("DEBUG: Migrate gsettings to gjiten.conf\n");

  GKeyFile *storage = g_key_file_new();

  {
    // load int/bool/string values
    _load_gsettings (STORE_ROOT_DCONF, storage, SECTION_GENERAL);
    _load_gsettings (STORE_KANJIDIC_DCONF, storage, SECTION_KANJIDIC);

    // load dict list and history
    GSettings * gsettings = g_settings_new (STORE_ROOT_DCONF);
    _load_gsettings_dictionary_list (self, gsettings, storage);
    _load_gsettings_history (self, gsettings, storage);
    g_object_unref (gsettings);
  }

  gboolean succ = _keyfile_save_new (storage, new_storage_dir, new_storage_file);
  g_key_file_free (storage);

  // TODO: remove gsettings entries from system

  return succ;
}


static void
_migrate_gconf_section(const gchar *gconf_file_path,
                       const gchar *keyfile_section,
                       GKeyFile    *storage)
{
  GHashTable * section = gconf_reader_parse_file (gconf_file_path, NULL);
  if (NULL != section)
  {
    _move_gconf_hashtable_to_keyfile (section, storage, keyfile_section);
    g_hash_table_destroy (section);
  }
  else
  {
    g_printerr ("Tried to migrate GJiten GConf data "
                "'%s' to section '%s', but the necessary"
                " file did not exist.\n", gconf_file_path, keyfile_section);
  }
}



static gboolean
_migrate_gconf(MigrationData *self,
               const gchar   *new_storage_dir,
               const gchar   *new_storage_file)
{
  GJITEN_DEBUG ("DEBUG: Migrate gconf to gjiten.conf\n");

  GKeyFile *storage = g_key_file_new();

  {
    // migrate "general" section
    _migrate_gconf_section (self->gconfig_path_general, SECTION_GENERAL, storage);

    // migrate history: don't, because tedious (Initialize it empty)
    g_key_file_set_string (storage, SECTION_GENERAL, "word_search_history", "");

    // migrate "kanjidic" section
    _migrate_gconf_section (self->gconfig_path_kanjidic, SECTION_KANJIDIC, storage);
  }

  gboolean succ = _keyfile_save_new (storage, new_storage_dir, new_storage_file);
  g_key_file_free (storage);

  // TODO: remove gconf files from system

  return succ;
}



gboolean
migrate_2_6_to_3_0(const gchar *new_storage_dir,
                   const gchar *new_storage_file)
{
  // setup
  gboolean succ = FALSE;
  MigrationData self;
  migration_data_init (&self);

  // run
  {
    if (_store_available_gsettings())
      succ = _migrate_gsettings (&self, new_storage_dir, new_storage_file);
    else
    if (_store_available_gconf (&self))
      succ = _migrate_gconf (&self, new_storage_dir, new_storage_file);
  }

  // cleanp
  migration_data_finalize (&self);

  return succ;
}



gboolean
migration_possible_2_6_to_3_0()
{
  return _store_available_gconf() || _store_available_gsettings();
}