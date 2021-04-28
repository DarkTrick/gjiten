/**
*  License: CC-0
*  Creator: DarkTrick - 69f925915ed0193a3b841aeec09451df2326f104
**/

#ifndef __GJ_WORDDIC_WINDOW_H__
#define __GJ_WORDDIC_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TYPE_GJ_WORDDIC_WINDOW gj_worddic_window_get_type ()



// note: member vars of derivable classes go into GjWorddicWindowPrivate (see c file)

G_DECLARE_DERIVABLE_TYPE (GjWorddicWindow, gj_worddic_window, GJ, WORDDIC_WINDOW, GtkApplicationWindow)
struct _GjWorddicWindowClass
{
  GtkApplicationWindowClass parent_class;
};



/* verb deinflection */
struct vinfl_struct {
  gchar *conj;
  gchar *infl;
  gchar *type;
};



GjWorddicWindow* worddic_create();
void             worddic_close();
GtkWidget*       gj_worddic_window_new(GtkApplication * app);
void worddic_paste();
void on_search_clicked();
void worddic_update_dic_menu();
void worddic_apply_fonts();
void enable_quick_lookup_mode();

void worddic_lookup_word(gchar * word_to_lookup);

G_END_DECLS

#endif /* include-protector */