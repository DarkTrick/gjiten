#include <glib.h>
#include <glib-object.h>

#define TYPE_BOXED_SLIST_OF_GVALUE _TYPE_BOXED_SLIST_OF_GVALUE()
GType _TYPE_BOXED_SLIST_OF_GVALUE();

GHashTable *
gconf_reader_parse_file(const gchar * filename,
                        GSList     ** o_gerrors_nullable);

GHashTable *
gconf_reader_parse_string(const gchar * gconf_content,
                          gsize         length,
                          GSList     ** o_gerrors_nullable);

GValue *
gconf_reader_get (GHashTable * hashtable,
                   const gchar * key);