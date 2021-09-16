AC_PREREQ(2.52)

GJITEN_VERSION=3.1

AC_INIT(gjiten, 3.1)
AC_CONFIG_SRCDIR(src/main.c)
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER([config.h])

AC_ISC_POSIX
AC_PROG_CC
#AM_PROG_CC_STDC
AC_PROG_INSTALL
AM_PROG_LIBTOOL
AC_PROG_MAKE_SET

PKG_CHECK_MODULES(GJITEN, gtk+-3.0 >= 3.22.0)
AC_CHECK_PROG(RESOURCES, glib-compile-resources)

IT_PROG_INTLTOOL([0.35.0])
AC_CONFIG_MACRO_DIRS([m4])



# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h fcntl.h locale.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_STAT
AC_CHECK_FUNCS([getcwd putenv setlocale strerror strrchr strstr munmap])

GNOME_COMPILE_WARNINGS(maximum)

# Gettext stuff
GETTEXT_PACKAGE=gjiten
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", ["$GETTEXT_PACKAGE"])
#AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,[GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"])
ALL_LINGUAS="ja de de_CH"
#AM_GLIB_GNU_GETTEXT
AM_GLIB_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])
PACKAGE_LOCALE_DIR="${datadir}/locale"
AC_SUBST(PACKAGE_LOCALE_DIR)


AC_ARG_ENABLE(debug,
		[  --enable-debug  Print debugging information. This is a feature which should not be usually enabled.],
    [debugging=yes; DEBUG_FLAGS="-ggdb -DDEBUG_GJITEN -O0"],
    [debugging=no]
	      )
AC_SUBST(DEBUG_FLAGS)
#DEBUG_FLAGS="-DGNOME_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED -ggdb -std=c89 -pedantic"],


AC_SUBST(GJITEN_CFLAGS)
AC_SUBST(GJITEN_LIBS)
AC_SUBST(PACKAGE_VERSION)


dnl Set PACKAGE_LOCALE_DIR in config.h.
gjiten_prefix="$prefix"
test "x$gjiten_prefix" = xNONE && gjiten_prefix=$ac_default_prefix
AS_IF([test "x${enable_debug}" == "xyes"], gjiten_prefix="..", )

SHARE_DIR="$gjiten_prefix/share"
AS_IF([test "x${enable_debug}" == "xyes"], gjiten_prefix="$gjiten_prefix/.", )
AC_DEFINE_UNQUOTED(SHARE_DIR, "$SHARE_DIR", ["$SHARE_DIR"])

PACKAGE_LOCALE_DIR="$SHARE_DIR/locale"
AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "$PACKAGE_LOCALE_DIR", ["$PACKAGE_LOCALE_DIR"])

GJITEN_BINDIR="$gjiten_prefix/bin"
AS_IF([test "x${enable_debug}" == "xyes"], GJITEN_BINDIR="./", )
AC_DEFINE_UNQUOTED(GJITEN_BINDIR, "$GJITEN_BINDIR", ["$GJITEN_BINDIR"])

GJITEN_DATADIR="$SHARE_DIR/gjiten"
AS_IF([test "x${enable_debug}" == "xyes"], GJITEN_DATADIR="../data", )
AC_DEFINE_UNQUOTED(GJITEN_DATADIR, "$GJITEN_DATADIR", ["$GJITEN_DATADIR"])

GNOMEDATADIR="$SHARE_DIR"
AS_IF([test "x${enable_debug}" == "xyes"], GNOMEDATADIR="../data", )
AC_DEFINE_UNQUOTED(GNOMEDATADIR, "$GNOMEDATADIR", ["$GNOMEDATADIR"])

PIXMAPDIR="$SHARE_DIR/pixmaps"
AS_IF([test "x${enable_debug}" == "xyes"], PIXMAPDIR="../pixmaps", )
AC_DEFINE_UNQUOTED(PIXMAPDIR, "$PIXMAPDIR", ["$PIXMAPDIR"])

PIXMAPDIR_LOGO="$PIXMAPDIR/gjiten"
AS_IF([test "x${enable_debug}" == "xyes"], PIXMAPDIR_LOGO="../pixmaps", )
AC_DEFINE_UNQUOTED(PIXMAPDIR_LOGO, "$PIXMAPDIR_LOGO", ["$PIXMAPDIR_LOGO"])


MAINTAINERCLEANFILES=Makefile.in
AC_SUBST(MAINTAINERCLEANFILES)
CLEANFILES='*~'
AC_SUBST(CLEANFILES)


AC_OUTPUT([ Makefile
	    pixmaps/Makefile
	    packaging/redhat/rpm/gjiten.spec
	    src/Makefile
	    data/Makefile
	    po/Makefile.in
	    po/Makefile
	    gnome/Makefile
	    dicfiles/Makefile
	  ])

# removed from AC_OUTPUT (above):
#	    help/Makefile
#			help/c/Makefile
#			help/ja/Makefile
# removed from Makefile.ac SUBDIRS=[...]
#			help

AC_MSG_NOTICE()
AS_IF([test "x${enable_debug}" == "xyes"], AC_MSG_NOTICE(IMPORTANT: Run gjiten from `src` directory. Otherwise static paths won't work), AC_MSG_NOTICE(IMPORTANT: You have to `make install` before running GJiten. Otherwise static paths will not be valid.))
AC_MSG_NOTICE()
