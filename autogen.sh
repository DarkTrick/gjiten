#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.


DIE=0

(test -f $srcdir/configure.ac) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level package directory"
    exit 1
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`autoconf' installed."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}


(grep "^IT_PROG_INTLTOOL" $srcdir/configure.ac >/dev/null) && {
  (intltoolize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`intltool' installed."
    echo "On Ubuntu you can install it with:"
    echo '  sudo add-apt-repository "deb http://archive.ubuntu.com/ubuntu $(lsb_release -sc) universe"'
    echo '  sudo apt install intltool intltool-debian'
    DIE=1
  }
}

(grep "^AM_PROG_XML_I18N_TOOLS" $srcdir/configure.ac >/dev/null) && {
  (xml-i18n-toolize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`xml-i18n-toolize' installed."
    echo "You can get it from:"
    echo "  ftp://ftp.gnome.org/pub/GNOME/"
    DIE=1
  }
}

(grep "^AM_PROG_LIBTOOL" $srcdir/configure.ac >/dev/null) && {
  (libtool --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`libtool' installed."
    echo "On Ubuntu you can install it with:"
    echo '  sudo add-apt-repository "deb http://archive.ubuntu.com/ubuntu $(lsb_release -sc) universe"'
    echo '  sudo apt install libtool'
    DIE=1
  }
}

(grep "^AM_GLIB_GNU_GETTEXT" $srcdir/configure.ac >/dev/null) && {
  (grep "sed.*POTFILES" $srcdir/configure.ac) > /dev/null || \
  (glib-gettextize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`glib' 2.0-dev installed."
    echo "On Ubuntu you can install it with:"
    echo '  sudo add-apt-repository "deb http://archive.ubuntu.com/ubuntu $(lsb_release -sc) universe"'
    echo '  sudo apt install libglib2.0-dev'
    DIE=1
  }
}


(glib-compile-resources --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Warning**: You should have \`glib-compile-resources' installed."
  echo "Otherwise you cannot build resource files."
  echo "If you want to continue anyway, manually work around the \`all-local' target \`in Makefile.am'"
}



(automake --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`automake' installed."
  echo "You can get it from: ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
  NO_AUTOMAKE=yes
}


# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
  echo "installed doesn't appear recent enough."
  echo "You can get automake from ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

if test "$DIE" -eq 1; then
  exit 1
fi

if test -z "$*"; then
  echo "**Warning**: I am going to run \`configure' with no arguments."
  echo "If you wish to pass any to it, please specify them on the"
  echo \`$0\'" command line."
  echo
fi

case $CC in
xlc )
  am_opt=--include-deps;;
esac

for coin in `find $srcdir -path $srcdir/CVS -prune -o -name configure.ac -print`
do
  dr=`dirname $coin`
  if test -f $dr/NO-AUTO-GEN; then
    echo skipping $dr -- flagged as no auto-gen
  else
    echo processing $dr
    ( cd $dr

      if grep "^AM_GLIB_GNU_GETTEXT" configure.ac >/dev/null; then
	echo "Creating $dr/aclocal.m4 ..."
	test -r $dr/aclocal.m4 || touch $dr/aclocal.m4
	echo "Running glib-gettextize...  Ignore non-fatal messages."
	echo "no" | glib-gettextize --force --copy
	echo "Making $dr/aclocal.m4 writable ..."
	test -r $dr/aclocal.m4 && chmod u+w $dr/aclocal.m4
      fi
      if grep "^IT_PROG_INTLTOOL" configure.ac >/dev/null; then
        echo "Running intltoolize..."
	intltoolize --copy --force --automake
      fi
      if grep "^AM_PROG_XML_I18N_TOOLS" configure.ac >/dev/null; then
        echo "Running xml-i18n-toolize..."
	xml-i18n-toolize --copy --force --automake
      fi
      if grep "^AM_PROG_LIBTOOL" configure.ac >/dev/null; then
	if test -z "$NO_LIBTOOLIZE" ; then
	  echo "Running libtoolize..."
	  libtoolize --force --copy
	fi
      fi
      echo "Running aclocal ..."
      aclocal
      if grep "^AM_CONFIG_HEADER" configure.ac >/dev/null; then
	echo "Running autoheader..."
	autoheader
      fi
      echo "Running automake --gnu $am_opt ..."
      automake --copy --add-missing --gnu $am_opt
      echo "Running autoconf ..."
      autoconf
    )
  fi
done

#conf_flags="--enable-maintainer-mode"


if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile. || exit 1
else
  echo Skipping configure process.
fi

#copy missing mkinstalldirs
if test ! -x ./mkinstalldirs && test -x /usr/share/automake-1.8/mkinstalldirs; then
    cp /usr/share/automake-1.8/mkinstalldirs ./mkinstalldirs
fi
if test ! -x ./mkinstalldirs && test -x /usr/share/automake-1.7/mkinstalldirs; then
    cp /usr/share/automake-1.7/mkinstalldirs ./mkinstalldirs
fi
if test ! -x ./mkinstalldirs && test -x /usr/share/automake/mkinstalldirs; then
    cp /usr/share/automake/mkinstalldirs ./mkinstalldirs
fi
