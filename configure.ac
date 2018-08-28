AC_INIT([libhdate],[1.8.0])
AC_SUBST(VERSION_INFO)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_ISC_POSIX
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_STDC_HEADERS
AM_PROG_LIBTOOL

AM_MAINTAINER_MODE

AC_C_CONST
AC_CONFIG_SRCDIR(src/hdate.h)
AC_CONFIG_MACRO_DIR([m4])
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

CFLAGS=
LIBS=-lm

dnl =======================================================================================

AC_CHECK_PROG(have_fpc, "fpc", do, dont)

build_fpc=$have_fpc

AC_ARG_ENABLE([fpc],
  [AS_HELP_STRING([--disable-fpc],
  [do not build free pascal binding])],
  [build_fpc=dont], [build_fpc=$have_fpc])

if test "$have_fpc" = "do"; then
  PA_VERSION=`fpc -iV`
  PA_TPR=`fpc -iTP`
  PA_TOS=`fpc -iTO`
  PA_MODULES_PATH="/usr/lib/fpc/$PA_VERSION/units/$PA_TPR-$PA_TOS"
  AC_SUBST([PA_VERSION])
  AC_SUBST([PA_MODULES_PATH])
fi

dnl =======================================================================================

AC_CHECK_PROG(have_gpc, "gpc", do, dont)

build_gpc=$have_gpc

AC_ARG_ENABLE([gpc],
  [AS_HELP_STRING([--disable-gpc],
  [do not build gnu pascal binding])],
  [build_gpc=dont], [build_gpc=$have_gpc])

if test "$have_gpc" = "do"; then
  GP_MODULES_PATH=`gpc -print-search-dirs | grep install | cut -d' ' -f 2`
  AC_SUBST([GP_MODULES_PATH])
fi

dnl =======================================================================================

AM_CONDITIONAL(WITH_GPC_FPC, [test "$build_fpc" = "yes" -a "$build_gpc" = "yes" ])
AM_CONDITIONAL(WITH_GPC, [test "$build_gpc" = "yes" -a "$build_fpc" != "yes" ])
AM_CONDITIONAL(WITH_FPC, [test "$build_gpc" != "yes" -a "$build_fpc" = "yes" ])

dnl =======================================================================================

AC_CHECK_PROG(have_swig, "swig", do, dont)

if test "$have_swig" = "do"; then
  AX_PKG_SWIG([], [], [])
  AX_SWIG_ENABLE_CXX
  AX_SWIG_MULTI_MODULE_SUPPORT
  AX_SWIG_PYTHON
fi

dnl =======================================================================================

AC_CHECK_PROG(have_python, "python", do, dont)

build_python=$have_python

AC_ARG_ENABLE([python],
  [AS_HELP_STRING([--disable-python],
  [do not build python binding])],
  [build_python=dont], [build_python=$have_python])

AC_ARG_WITH(python-sitelib-dir,
  [AS_HELP_STRING([--with-python-sitelib-dir=PATH],
  [path to python site lib])])

if test "$have_python" = "do"; then
    AM_PATH_PYTHON([2.7],, [:])
    PY_PREFIX=`python -c 'import sys ; print sys.prefix'`
    PY_VERSION=`python -c 'import sys ; print sys.version[[0:3]]'`
    AC_MSG_CHECKING(for $PY_PREFIX/include/python$PY_VERSION/Python.h)
    if test -f $PY_PREFIX/include/python$PY_VERSION/Python.h; then
      AC_MSG_RESULT(yes)
      PY_CFLAGS="-I$PY_PREFIX/include/python$PY_VERSION"

      if test "$with_python_sitelib_dir" = "" ; then
        PY_MODULES_PATH="$PY_PREFIX/lib/python$PY_VERSION/dist-packages"
      else
        PY_MODULES_PATH=$with_python_sitelib_dir
      fi

      AC_SUBST([PY_VERSION])
      AC_SUBST([PY_CFLAGS])
      AC_SUBST([PY_MODULES_PATH])
      if test "$have_swig" = "dont"; then
      build_python=dont
      fi
    else
      AC_MSG_RESULT(no)
      build_python=dont
    fi
fi

AM_CONDITIONAL(WITH_PYTHON, test "$build_python" = "do")

dnl =======================================================================================
dnl :
dnl : AC_CHECK_PROG(have_ruby, "ruby", do, dont)
dnl :
dnl : build_ruby=$have_ruby
dnl :
dnl : AC_ARG_ENABLE([ruby],
dnl :   [AS_HELP_STRING([--disable-ruby],
dnl :   [do not build ruby binding])],
dnl :   [build_ruby=dont], [build_ruby=$have_ruby])
dnl :
dnl : AC_ARG_WITH(ruby-sitelib-dir,
dnl :   [AS_HELP_STRING([--with-ruby-sitelib-dir=PATH],
dnl :   [path to ruby site lib])])
dnl :
dnl : if test "$have_ruby" = "do"; then
dnl :     RU_PREFIX=`ruby -e 'require "mkmf"; puts $topdir'`
dnl :     RU_VERSION=`ruby -e "require 'mkmf'; puts config_string('MAJOR')"`
dnl :     RU_VERSION+=.`ruby -e "require 'mkmf'; puts config_string('MINOR')"`
dnl :     AC_MSG_CHECKING(for $RU_PREFIX/ruby.h)
dnl :     if test -f $RU_PREFIX/ruby.h; then
dnl :       AC_MSG_RESULT(yes)
dnl :       RU_CFLAGS="-I$RU_PREFIX"
dnl :
dnl :       if test "$with_ruby_sitelib_dir" = "" ; then
dnl :         RU_MODULES_PATH="$RU_PREFIX"
dnl :       else
dnl :         RU_MODULES_PATH=$with_ruby_sitelib_dir
dnl :       fi
dnl :
dnl :       AC_SUBST([RU_VERSION])
dnl :       AC_SUBST([RU_CFLAGS])
dnl :       AC_SUBST([RU_MODULES_PATH])
dnl :       if test "$have_swig" = "dont"; then
dnl :       build_ruby=dont
dnl :       fi
dnl :     else
dnl :       AC_MSG_RESULT(no)
dnl :       build_ruby=dont
dnl :     fi
dnl : fi
dnl :
dnl : AM_CONDITIONAL(WITH_RUBY, test "$build_ruby" = "do")
AM_CONDITIONAL(WITH_RUBY, false)
dnl :
dnl =======================================================================================

AC_CHECK_PROG(have_php, "php", do, dont)

build_php=$have_php

AC_ARG_ENABLE([php],
  [AS_HELP_STRING([--disable-php],
  [do not build php binding])],
  [build_php=dont], [build_php=$have_php])

AC_ARG_WITH(php-sitelib-dir,
  [AS_HELP_STRING([--with-php-sitelib-dir=PATH],
  [path to php site lib])])

if test "$have_php" = "do"; then
    AC_CHECK_PROG(have_php_config, "php-config5", do, dont)
    if test "$have_php_config" = "do"; then

        PH_CONFIG_PATH=`php5 --ini |  grep "Scan" | cut -d: -f2 | sed 's/\s//g'`
        PH_INCLUDE_PATH=`php5 -r "echo get_include_path();" | cut -f2 -d:`
      if test "$with_php_sitelib_dir" = "" ; then
        PH_MODULES_PATH=`php-config5 --extension-dir`
      else
        PH_MODULES_PATH=$with_php_sitelib_dir
      fi

      AC_SUBST([PH_CONFIG_PATH])
      AC_SUBST([PH_INCLUDE_PATH])
      AC_SUBST([PH_MODULES_PATH])
      if test "$have_swig" = "dont"; then
      build_php=dont
      fi
    else
      AC_MSG_RESULT(no)
      build_php=dont
    fi
fi

AM_CONDITIONAL(WITH_PHP, test "$build_php" = "do")

dnl =======================================================================================

AC_CHECK_PROG(have_perl, "perl", do, dont)

build_perl=$have_perl

AC_ARG_ENABLE([perl],
  [AS_HELP_STRING([--disable-perl],
  [do not build perl binding])],
  [build_perl=dont], [build_perl=$have_perl])

AC_ARG_WITH(perl-sitelib-dir,
  [AS_HELP_STRING([--with-perl-sitelib-dir=PATH],
  [path to perl site lib])])

if test "$have_perl" = "do"; then
    PE_PREFIX=`(perl -e 'use Config; print $Config{archlib};')`
    AC_MSG_CHECKING(for $PE_PREFIX/CORE/perl.h)
    if test -f $PE_PREFIX/CORE/perl.h; then
      AC_MSG_RESULT(yes)
      PE_CFLAGS="-I$PE_PREFIX/CORE "
      PE_CFLAGS=$PE_CFLAGS`(perl -e 'use Config; print $Config{cppflags};')`

      if test "$with_perl_sitelib_dir" = "" ; then
        PE_MODULES_PATH=`(perl -e 'use Config; print $Config{sitelib};')`
      else
        PE_MODULES_PATH=$with_perl_sitelib_dir
      fi

      AC_SUBST([PE_CFLAGS])
      AC_SUBST([PE_MODULES_PATH])
      if test "$have_swig" = "dont"; then
      build_perl=dont
      fi
    else
      AC_MSG_RESULT(no)
      build_perl=dont
    fi
fi

AM_CONDITIONAL(WITH_PERL, test "$build_perl" = "do")

dnl =======================================================================================

dnl gettext

dnl Languages which your application supports
ALL_LINGUAS="he es_MX ru fr sv"
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.3])

if test "$DATADIRNAME" = ""; then
   DATADIRNAME="share"
   AC_SUBST(DATADIRNAME)
fi

localepath=`eval echo ${localedir}`

AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${localepath}")

dnl =======================================================================================

build_hcal=do

AC_ARG_ENABLE([hcal],
  [AS_HELP_STRING([--disable-hcal],
  [do not build hcal/hdate utility programs])],
  [build_hcal=dont], [build_hcal=do])

AM_CONDITIONAL(WITH_HCAL, test "$build_hcal" = "do")
AM_PROG_CC_C_O

dnl =======================================================================================

AC_SUBST(CFLAGS)
AC_SUBST(LIBS)

dnl =======================================================================================

AC_OUTPUT([
Makefile
libhdate.pc
po/Makefile.in
docs/Makefile
docs/man/Makefile
docs/man/man1/Makefile
docs/man/man3/Makefile
src/Makefile
examples/Makefile
examples/hcal/Makefile
examples/bindings/Makefile
examples/bindings/pascal/Makefile
bindings/Makefile
bindings/python/Makefile
bindings/perl/Makefile
bindings/pascal/Makefile
bindings/ruby/Makefile
bindings/php/Makefile
])

dnl ===========================================================================================

echo "
${PACKAGE}-${VERSION}

  compiler:                       ${CC}

  prefix:                         ${prefix}
  locale-dir:                     ${localepath}

  ${build_hcal} build cli progs (hcal, hdate)

  ${have_swig} have swig

    ${build_fpc} build free pascal binding
      ${have_fpc} have free pascal
      free pascal site lib path:        $PA_MODULES_PATH

    build gnu pascal binding        tbd
      ${have_gpc} have gnu pascal
      gnu pascal site lib path:         $GP_MODULES_PATH

    ${build_perl} build perl binding
      ${have_perl} have perl
      perl site lib path:               $PE_MODULES_PATH

    ${build_php} build php binding
      ${have_php} have php-dev (v5.x)
      ${have_php_config} php-cli (php-config5)
      php site lib path:                $PH_MODULES_PATH
      php config path:                  $PH_CONFIG_PATH
      php include path:                 $PH_INCLUDE_PATH

    ${build_python} build python binding
      ${have_python} have python
      python site lib path:             $PY_MODULES_PATH

dnl :    ${build_ruby} build ruby binding
dnl :      ${have_ruby} have ruby
dnl :      ruby site lib path:               $RU_MODULES_PATH
"
