#include <gtk/gtk.h>

#include "gjiten.h"
#include "utils.h"
#include "config.h"


/**
 * @brief: This function does the basic calls to setup a window.
**/
int
main (int argc, char *argv[])
{
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (PACKAGE_TARNAME, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  GtkApplication * app = gjiten_new();

  int status = g_application_run(G_APPLICATION (app), argc, argv);
  g_object_unref(app);

  return status;
}
