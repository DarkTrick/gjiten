
#include <gtk/gtk.h>
#include "utils.h"


gchar *strginfo[] = { //FIXME: change this to EnumPair
  N_("Kanji"),
  N_("Radicals"),
  N_("Stroke count"),
  N_("Readings"),
  N_("Romanized Korean reading"),
  N_("Romanized Pinyin reading"),
  N_("English meaning"),
  N_("Bushu radical number"),
  N_("Classical radical number"),
  N_("Frequency number"),
  N_("Jouyou grade level"),
  N_("De Roo code"),
  N_("Skip pattern code"),
  N_("Four Corner code"),
  N_("\"New Jp-En Char Dictionary\" index"),
  N_("Index in Nelson's \"Modern Reader's Char Dict\""),
  N_("\"The New Nelson Char Dict\" index"),
  N_("Spahn&Hadamitzky index"),
  N_("Morohashi \"Daikanwajiten\" index"),
  N_("Morohashi \"Daikanwajiten\" volume.page number"),
  N_("\"A Guide To Remembering Jap. Chars\" index"),
  N_("Gakken Kanji Dictionary index"),
  N_("Index in \"Remembering The Kanji\""),
  N_("Index in \"Japanese Names\""),
  N_("Cross-reference code"),
  N_("Misclassification code"),
  N_("Unicode hex number of the kanji"),
  N_("ASCII JIS Code of Kanji")
};


void create_dialog_preferences() {
  g_print ("Open Preferences: preferences dialog is not yet implemented\n");
}