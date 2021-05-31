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
	radical-convtable.h	  \
	kanjidicconsts.h      \
	constants.h

radical_convtable: radical-convtable.txt
	./convtable.pl

EXTRA_DIST = convtable.pl radical-convtable.txt

gjiten_CFLAGS = $(GJITEN_CFLAGS) -D DEBUG_GJITEN -Wall -Wunused \
                -Wno-unused-function -Wno-unused-variable @DEBUG_FLAGS@ \
                -DPACKAGE_LOCALE_DIR=\"${PACKAGE_LOCALE_DIR}\"

gjiten_LDADD = $(GJITEN_LIBS)
