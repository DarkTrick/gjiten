
INTLTOOL_FILES = intltool-extract.in \
                 intltool-merge.in \
                 intltool-update.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src data po gnome pixmaps dicfiles

EXTRA_DIST = README TODO ChangeLog COPYING rpm/gjiten.spec scripts/make_debs scripts/make_release\
	     intltool-extract.in intltool-merge.in intltool-update.in

DISTCLEANFILES = ${INTLTOOL_FILES} \
                 po/.intltool-merge-cache

MAINTAINERCLEANFILES += configure config.sub config.guess aclocal.m4 compile \
      depcomp install-sh \
      ${DISTCLEANFILES} intltool-extract intltool-merge intltool-update.in \
		ltmain.sh missing mkinstalldirs config.h.in po/*stamp* *stamp*


