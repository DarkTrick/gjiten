## Process this file with automake to produce Makefile.in

SYSCONFDIR = @sysconfdir@
bin_PROGRAMS = gjiten

gjiten_SOURCES = 	      \
	main.c                \
	utils.c utils.h       \
	gjiten.c gjiten.h     \
	kanjidic.c kanjidic.h \
	pref.c pref.h         \
	conf.c conf.h         \
	error.c error.h	      \
	dicfile.c dicfile.h	  \
	worddic.c worddic.h	  \
	dicutil.c dicutil.h	  \
	radicals.c radicals.h	  \
	radicals_ui.c radicals_ui.h	  \
	radical-convtable.h	  \
	kanjidicconsts.h      \
	gconf_reader.c gconf_reader.h \
	data_store.c data_store.h \
	resources.c resources.h	  \
	migration2-6_3-0.c migration2-6_3-0.h \
	constants.h

radical_convtable: radical-convtable.txt
	./convtable.pl

EXTRA_DIST = convtable.pl radical-convtable.txt

gjiten_CFLAGS = $(GJITEN_CFLAGS) -Wall -Wunused \
                -Wno-unused-function -Wno-unused-variable @DEBUG_FLAGS@ \
                -DPACKAGE_LOCALE_DIR=\"${PACKAGE_LOCALE_DIR}\"

gjiten_LDADD = $(GJITEN_LIBS)
