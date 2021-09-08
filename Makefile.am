
INTLTOOL_FILES = intltool-extract.in \
                 intltool-merge.in \
                 intltool-update.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = . src data po gnome pixmaps dicfiles

EXTRA_DIST = COPYING rpm/gjiten.spec scripts/make_debs scripts/make_release\
	     intltool-extract.in intltool-merge.in intltool-update.in


DISTCLEANFILES =                         \
	          intltool-extract         \
	          intltool-merge           \
	          intltool-update          \
	          po/.intltool-merge-cache

MAINTAINERCLEANFILES +=   \
      	configure           \
      	config.sub          \
      	config.guess        \
      	aclocal.m4          \
      	compile             \
      	depcomp install-sh  \
      	${DISTCLEANFILES}   \
		ltmain.sh           \
      	missing             \
      	mkinstalldirs       \
      	config.h.in         \
      	po/*stamp*          \
      	*stamp*

all-local:
	     glib-compile-resources --generate-header --target=./src/resources.h ./resources.xml
	     glib-compile-resources --generate-source --target=./src/resources.c ./resources.xml