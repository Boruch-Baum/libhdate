#!/bin/sh
this="libhdate autogen: "
failure=""
for pgm in gcc libtoolize autoreconf make ; do
  if ! which $pgm ; then
    printf "$this dependency not found: $pgm\n"
    failure=true
  fi
done
if [ -n "$failure" ] ; then
  printf "  Hint: packages build-essential, libtool, autoconf, automake.\n"
  exit 1
fi
for pgm in "libtoolize" "autoreconf -i" "./configure" "make" ; do
  if ! $pgm ; then
    printf "$this $pgm failure.\n"
    exit 2
  fi
done
printf "$this completed successfully.\n"
