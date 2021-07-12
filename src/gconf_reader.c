/**
 *  This file provides parsing functionality for gconf files.
 *  Why not use the gconf reader from gnome directly?
 *    - static dependency:
 *        Sources and documentation are difficult to find;
 *        compilation might be difficult due to further dependencies
 *    - dynamic dependency:
 *        Going to be unresolvable (on ubuntu)
 *
 **/

#include <glib.h>
#include <glib-object.h>
#include <glib/gprintf.h> // g_snprintf
#include "gconf_reader.h"

#define return_if(expression, value) if (expression){ return value; }

/**
 *  Remove first item in list, free it, return its data
 *
 *  Returns
 *    `data` of first element in list
 *    `NULL`, if list is empty
 **/
static gpointer
g_slist_pop (GSList ** io_self)
{
  GSList * self = *io_self;
  if (NULL == self) return NULL;

  gpointer ret = self->data;
  GSList * newStart = g_slist_remove_link (self,self);
  *io_self = newStart;

  g_slist_free_1 (self);

  return ret;
}


/**
 * Free's owned data and itself.
 * Use for GValues on heap.
 **/
static void
g_value_finalize (GValue *self)
{
  g_value_unset (self);
  g_free (self);
}



/** Return:
 *     data of first element
 *     NULL, if list is empty
 **/
static gpointer
g_slist_head (GSList *self){
  if (self == NULL)
    return NULL;
  return self->data;
}



/** key/value are owned by pair **/
struct _Pair {
  gchar * key;
  GValue * value;
};
typedef struct _Pair Pair;



static Pair *
pair_new(gchar * key, GValue *value)
{
  Pair * self = g_new (Pair, 1);
  self->key = key;
  self->value = value;
  return self;
}


/**
 * Free all resources of pair
 *      and then itself
 **/
static void
pair_free(gpointer _self)
{
  Pair * self = (Pair*)_self;
  g_free (self->key);
  g_value_finalize (self->value);
  g_free (self);
}



struct _ParsingData {
  GSList * pipeline; // pipeline of `Pair`s
  GHashTable * map;
  gboolean processing_entry;
  gboolean processing_li;
  gboolean processing_string;
  gboolean processing_list;
  gboolean unrecoverable_error;
  GSList * strErrors;
};
typedef struct _ParsingData ParsingData;

static void
parsing_data_switches_off(ParsingData *self)
{
  self->processing_entry = FALSE;
  self->processing_li = FALSE;
  self->processing_string = FALSE;
  self->processing_list = FALSE;
  self->unrecoverable_error = FALSE;
}

static void
parsing_data_on_unrecoverable_error (ParsingData *self,
                                     GMarkupParseContext *context)
{
  //g_print("!!!!!!!! unrecoverable error; stop parsing\n");

  self->unrecoverable_error = TRUE;
}

/**
 * Returns
 * Error string containing line and char position.
 * Must be freed with `g_free`.
 **/
static gchar *
g_markup_parse_context_create_error (GMarkupParseContext *self,
                                     const gchar * strError)
{
  gchar buffer[256] = {0};

  gint errorLine = -1;
  gint errorChar = -1;
  g_markup_parse_context_get_position (self, &errorLine, &errorChar);

  g_snprintf (buffer, 255, "Error on line %d char %d: %s",
              errorLine, errorChar, strError);

  return g_strdup (buffer);
}


/**
 * Append the given error
 **/
static void
parsing_data_append_error(ParsingData * self,
                          GMarkupParseContext *context,
                          const gchar * error)
{
  gchar * error_full = g_markup_parse_context_create_error (context, error);
  //g_print ("ERROR: %s\n", error_full);
  self->strErrors = g_slist_append (self->strErrors, error_full);
}


static void g_value_finalize_pointer(gpointer p)
{
  g_value_finalize ((GValue*)p);
}



static ParsingData *
parsing_data_new ()
{
  ParsingData * self = g_new0(ParsingData,1);

  self->map = g_hash_table_new_full (g_str_hash, g_str_equal,
                                    g_free, g_value_finalize_pointer);
  self->strErrors = NULL;
  parsing_data_switches_off (self);
  return self;
}



/** Cleans up owned members of structure **/
static void
parsing_data_free(ParsingData *self)
{
  if (NULL != self->map){
    g_hash_table_destroy (self->map);
    g_free (self->map);
  }
  g_slist_free_full (self->pipeline, pair_free);
  g_slist_free_full (self->strErrors, g_free);
  g_free (self);
}



/** Return
 *      char* value at `index` position
 *      NULL, if index is < 0
 **/
static const gchar *
getCStringAt (const gchar **char_array,
           int           index,
           const gchar  *defaultValue)
{
  if (index >= 0)
    return char_array[index];
  return defaultValue;
}



static void
attributes_find_indices (const gchar **attribute_names,
                         int          *o_idxName,
                         int          *o_idxValue,
                         int          *o_idxType)
{
  for (int i=0; attribute_names[i]; ++i)
  {
    if (g_str_equal ("name", attribute_names[i]))
      *o_idxName = i;
    else if (g_str_equal ("value", attribute_names[i]))
      *o_idxValue = i;
    else if (g_str_equal ("type", attribute_names[i]))
      *o_idxType = i;
  }
}



static void
attributes_find_indices2 (const gchar **attribute_names,
                          const gchar **attribute_values,
                          const gchar **o_strName,
                          const gchar **o_strValue,
                          const gchar **o_strType)
{
  int nameIndex  = -1;
  int valueIndex = -1;
  int typeIndex  = -1;

  {
    attributes_find_indices (attribute_names, &nameIndex,
                              &valueIndex, &typeIndex);

  }

  *o_strType = getCStringAt (attribute_values, typeIndex, NULL);
  *o_strName  = getCStringAt (attribute_values, nameIndex, "");
  *o_strValue = getCStringAt (attribute_values, valueIndex, NULL);
}



void
boxed_slist_gvalue_finalize(gpointer _self)
{
  GSList * self = (GSList*)_self;
  if (NULL == self) return;

  g_slist_free_full (self, g_value_finalize_pointer);
}



gpointer
boxed_copy_default(gpointer _self)
{
  g_error ("❌ CRITICAL ERROR: Boxed type does not support copying, but copy was called.\n");
  exit(1);
}



GType
_TYPE_BOXED_SLIST_OF_GVALUE()
{
  static GType type;
  if (!type)
    type = g_boxed_type_register_static (
                                "type_boxed_slist_of_gvalue",
                                boxed_copy_default,
                                boxed_slist_gvalue_finalize);
  return type;
}



/**
 * creates a gvalue for the given type
 *   changes parser settings, if necessary
 * Parameters:
 *  value_nullable: NULL is only allowed for
 *                  strings and lists.
 *
 * Return:
 *    NULL, if type is unnknown
 **/
static const gchar *
parsing_data_create_value(ParsingData  * self,
                          const gchar  * type,
                          const gchar  * value_nullable,
                          GValue      ** o_gvalue)
{
  return_if (NULL == type, "Element has no type");

  GValue * gvalue = NULL;

  if (g_str_equal ("bool", type))
  {
    // value for bool not optional
    return_if (NULL == value_nullable, "No value for bool entry given.");

    gboolean val = g_str_equal ("true", value_nullable);

    gvalue = g_new0(GValue, 1);
    g_value_init (gvalue, G_TYPE_BOOLEAN);
    g_value_set_boolean (gvalue, val);
  }

  else if (g_str_equal ("int", type))
  {
    // value for int not optional
    return_if (NULL == value_nullable, "No value for int entry given.");

    gint value = (gint) (g_ascii_strtoll (value_nullable, NULL, 10));

    gvalue = g_new0(GValue, 1);
    g_value_init (gvalue, G_TYPE_INT);
    g_value_set_int (gvalue, value);
  }

  else if (g_str_equal ("float", type))
  {
    // value for bool not optional
    return_if (NULL == value_nullable, "No value for float entry given.");

    gfloat value = (gfloat) (g_ascii_strtod (value_nullable, NULL));

    gvalue = g_new0(GValue, 1);
    g_value_init (gvalue, G_TYPE_FLOAT);
    g_value_set_float (gvalue, value);
  }

  else if (g_str_equal ("string", type))
  {
    gvalue = g_new0(GValue, 1);
    g_value_init (gvalue, G_TYPE_STRING);

    // handle strings in `text` function
    self->processing_string = TRUE;
  }

  else if (g_str_equal ("list", type))
  {
    gvalue = g_new0(GValue, 1);
    GType listType = TYPE_BOXED_SLIST_OF_GVALUE;
    g_value_init (gvalue, listType);
    g_value_take_boxed (gvalue, NULL);
    self->processing_list = TRUE;
  }

  else
  {
    // don't process unknown entries
  }
  *o_gvalue = gvalue;

  return NULL;
}


static void
parsing_data_on_error(GMarkupParseContext *context,
                      GError              *error,
                      gpointer             _self)
{
  ParsingData * self = ((ParsingData*)_self);

  self->strErrors = g_slist_append (self->strErrors, g_strdup(error->message));

  // DO NOT FREE `error` (or the like). Otherwise we get
  //  a segfault at `g_markup_parse_context_unref`

  //g_clear_error (&error);
}




/**
 * Function, that's called, if a tag opens.
 *
 * Here: just print tag name and attributes
 **/
static void
parsing_data_start_element(GMarkupParseContext *context,
              const gchar         *element_name,
              const gchar        **attribute_names,
              const gchar        **attribute_values,
              gpointer             _self,
              GError             **error)
{
  ParsingData * self = ((ParsingData*)_self);
  return_if (self->unrecoverable_error == TRUE, );

  gboolean doProcess = FALSE;
  if (g_str_equal ("entry", element_name))
  {
    // check if an `entry` was already opened
    if (self->processing_entry == TRUE ||
        self->processing_list == TRUE ||
        self->processing_li == TRUE){
      parsing_data_append_error (self, context,
          "Previous <entry> was not closed.");
    } else {
      doProcess = TRUE;
      self->processing_entry = TRUE;
    }
  }

  if (g_str_equal ("li", element_name))
  {
    if (self->processing_li == TRUE)
    {
      parsing_data_append_error (self, context,
          "<li> should not be inside <li>.");
      parsing_data_on_unrecoverable_error (self, context);
      return;
    }

    if (self->processing_list == FALSE)
    {
      parsing_data_append_error (self, context,
          "<li> is not inside a <entry type='list'>.");
      parsing_data_on_unrecoverable_error (self, context);
      return;
    }

    self->processing_li = TRUE;
    doProcess = TRUE;

  }

  if (doProcess)
  {
    const gchar * strType;
    const gchar * strKey;
    const gchar * strValue;
    attributes_find_indices2 (attribute_names, attribute_values,
                               &strKey, &strValue, &strType);


    GValue * gvalue = NULL;
    const gchar * error = parsing_data_create_value (self, strType, strValue, &gvalue);

    if (NULL != error)
      parsing_data_append_error (self, context, error);


    if (gvalue)
    {
      Pair * pair = pair_new (g_strdup (strKey), gvalue);
      self->pipeline = g_slist_prepend (self->pipeline, pair);
    }
    else
    {
      // No item is placed in the pipeline; `_end_element()` needs
      //    to know that.
      self->processing_li = FALSE;
      self->processing_entry = FALSE;
    }

  }

}



/**
 * Function, that's called, if a tag closes
 * Here: just print the closing tag
 **/
static void
parsing_data_end_element(GMarkupParseContext *context,
            const gchar         *element_name,
            gpointer             _self,
            GError             **error)
{
  ParsingData * self = ((ParsingData*)_self);

  return_if (self->unrecoverable_error == TRUE, );

  Pair * currentPair = (Pair*)g_slist_head (self->pipeline);

  if (currentPair == NULL) return;

  else if (g_str_equal ("li", element_name))
  {
    if (! self->processing_list || !self->processing_li)
    {
      parsing_data_append_error (self, context, "Parsing error: '</li> without list");
      parsing_data_switches_off (self);
      return;
    }
    if (self->processing_string)
    {
      parsing_data_append_error (self, context, "Parsing error: '</li>, but no value given");
      return;
    }

    currentPair = (Pair*)g_slist_pop (&(self->pipeline));
    GValue * listData = currentPair->value; // e.g. GValue<string> (NOT list)
    currentPair->value = NULL;

    // free all data we don't need
    g_free (currentPair->key);
    g_free (currentPair);

    // get GList from pipeline
    Pair * listPair = (Pair*)g_slist_head (self->pipeline);
    GValue * _slist = listPair->value;
    GSList * slist = (GSList*)(g_value_get_boxed (_slist));

    // update GList in pipeline
    // IMPORTANT: WE CANNOT UPDATE THE VALUE IN
    slist = g_slist_append (slist, listData);
    if (g_value_get_boxed (_slist) == NULL)
      g_value_take_boxed (_slist, slist);

    self->processing_li = FALSE;

  }

  else if (g_str_equal ("entry", element_name))
  {
    if (FALSE == self->processing_entry)
      // no error because:
      //    if error is semantic,  there's already one from opening tag.
      //    if error is syntactic, there's already one from the parser.
      return;



    currentPair = (Pair*)g_slist_pop (&(self->pipeline));
    g_hash_table_insert (self->map, currentPair->key, currentPair->value);
    g_free (currentPair);


    // clear all switches
    self->processing_entry  = FALSE;
    self->processing_list   = FALSE;
    self->processing_li     = FALSE;
    self->processing_string = FALSE;
  }
}



/**
 * Function, that's called, if text is encountered
 * E.g. <div>my text</div>
 *
 * Note: there is text before every opening tag.
 * E.g. <div>
 *        <div></div>
 *      </div>
 *     Has "\n  " after the first "<div>"
 **/
static void
parsing_data_text(GMarkupParseContext *context,
             const gchar         *text,
             gsize                text_len,
             gpointer             _self,
             GError             **error)
{
  ParsingData * self = ((ParsingData*)_self);
  return_if (self->unrecoverable_error == TRUE, );

  // add the current text value to the element on processing stack
  if (self->processing_string)
  {
    const gchar * element_name = g_markup_parse_context_get_element (context);
    if (g_str_equal ("stringvalue", element_name))
    {
      Pair * current = (Pair*)g_slist_head (self->pipeline);
      g_value_take_string (current->value, g_strdup (text));
      //g_value_set_string (current->value, text);
      self->processing_string = FALSE;
    }
  }
}

/**
 * Convenience function, that does the same as
 * g_hash_table_lookup, but parses the results.
 **/
GValue *
gconf_reader_get (GHashTable * hashtable,
                   const gchar * key)
{
  return (GValue*)g_hash_table_lookup (hashtable, key);
}


/**
 * Converts `gconf_content` to a GHashTable.
 * If any errors occured, they will be stored in
 * `o_str_errorlist_nullable`, if it's not NULL.
 *
 * `gconf_content` follows the spec for a gconf file.
 *
 * If `o_str_errorlist_nullable` is not NULL, it
 * must be freed with g_slist_free_full ([x], g_free)
 *
 * Returns
 *    A hash table representing the content of `gconf_content`
 *    It has to be freed with `g_hash_table_destroy()`.
 *    Hash table structure is as follows:
 *       key:   string
 *       value: GValue
 *    For lists in gconf files, GValue contains a GSList
 **/
GHashTable *
gconf_reader_parse_string(const gchar * gconf_content,
                          gsize         length,
                          GSList     ** o_str_errorlist_nullable)
{
  ParsingData * parsing_data = parsing_data_new();
  {
    GMarkupParser parser = {parsing_data_start_element, parsing_data_end_element,
                            parsing_data_text, NULL, parsing_data_on_error};
    {
      GMarkupParseContext * parseContext = g_markup_parse_context_new (
                                            &parser,(GMarkupParseFlags)0,
                                            parsing_data, NULL);
      g_markup_parse_context_parse (parseContext, gconf_content, length, NULL);

      // don't use free for parseContext!
      g_markup_parse_context_unref (parseContext);
    }
  }

  // return management
  {
    GHashTable * ret = parsing_data->map;

    // error management
    if (o_str_errorlist_nullable != NULL){
      *o_str_errorlist_nullable = parsing_data->strErrors;
      // prevent freeing of error list
      parsing_data->strErrors = NULL;
    }

    // prevent map memory from being freed
    parsing_data->map = NULL;

    // free anything, that's still left
    parsing_data_free (parsing_data);


    return ret;
  }
}



/**
 * Convenience function to call `gconf_reader_parse_string`
 * for file content.
 *
 * Returns:
 *    NULL, if there was an error opening the file.
 *    A GHashTable otherwise (see `gconf_reader_parse_string`)
 **/
GHashTable *
gconf_reader_parse_file(const gchar * filename,
                        GSList     ** o_str_errorlist_nullable)
{
  gsize length;
  gchar *fileContents;
  if (g_file_get_contents (filename, &fileContents, &length, NULL))
  {
    GHashTable * ret = gconf_reader_parse_string (fileContents, length, o_str_errorlist_nullable);
    g_free ((gpointer)fileContents);
    return ret;
  }

  return NULL;
}

