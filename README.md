Libhdate, hcal, hdate
=====================

CONTENTS
--------
1] Description
2] Brief highlights of this update
3] Installation from source (Instructions)


1] Description
--------------
LibHdate is a small library for the Hebrew calendar, dates, holidays,
and reading sequence. It is written in C and includes bindings for
pascal, perl, python, php, ruby.

hcal and hdate are small example command line programs written in C.

Refer to the following files:
INSTALL      instructions how to install this package
USE          basic example of how to use one of the library functions
             in a C program, how to compile it, and how to run it.
NEWS         press-release style information of the current release
Changelog    more detailed list of project activity over many releases

Refer to the man pages and examples directory for further documentation
and examples.


2] Brief highlights of this update
----------------------------------
This release brings many new options, features, and bug fixes to the two example
programs hcal and hdate. The changes to the underlying function library include
a few minor bug fixes, deprecation of a series of string functions in favor
of a single new one with better memory allocation, and hard-coding of core
elements of the Hebrew localization so that Hebrew can be displayed in all
locales. The documentation in the man pages and in the new configuration
file should be sufficient.

2.1] selected highlights
------------------------

hcal
- introduced config file $XDG_CONFIG_HOME/hcal/hcalrc or ~/.config/hcal/hcalrc
- option --menu -m to prompt user for user-defined menu selections from config file
- option --candles and --havdalah take optional numeric arguments to modify the default minhag.
- remove dependency of Hebrew on locale environment variables
- new feature: sunset-aware to highlight correct hebrew date
- new options: shabbat times and readings
- new feature: location-awareness (options -l -L -z)
- basic location-guessing based upon time zone
- coordinates may now also be in form degrees, minutes, seconds and be delimited by :, ', or "
- coordinates may now include compass directions N, S, E, W.
- time zone may now include fractional hours, or minutes (delimited by :, ', or ")
- highlight current day in reverse video
- compress width of display on screen
- print Gregorian and Hebrew months on one line
- option three-month to display previous and next months with current
- option three-month with year displays four rows, three columns
- option colorize to display data in, um, color
- option footnote to explain the month's holidays
- option force-hebrew for non-hebrew locales
- basic diaspora-awareness based on system time-zone

hdate
- introduced config file $XDG_CONFIG_HOME/hdate/hdaterc or ~/.config/hdate/hdaterc
- option --menu -m to prompt user for user-defined menu selections from config file
- option --candles and --havdalah take optional numeric arguments to modify the default minhag.
- remove dependency of Hebrew on locale environment variables
- new feature: sunset-aware to highlight correct hebrew date
- coordinates may now also be in form degrees, minutes, seconds and be delimited by :, ', or "
- coordinates may now include compass directions N, S, E, W.
- time zone may now include fractional hours, or minutes (delimited by :, ', or ")
- Hebrew information can optionally display in Hebrew characters in all locales
- option --sunset-aware, prints tomorrow's Hebrew date and info (for today only) if after sunset
- option --table, prints data in csv format, suitable for spreadsheets, awk, etc.

libhdate
- new function hdate_string
- DEPRECATION: hdate_get_day_string is now only a wrapper for hdate_string
- DEPRECATION: hdate_get_hebrew_month_string is now only a wrapper for hdate_string
- DEPRECATION: hdate_get_holyday_string is now only a wrapper for hdate_string
- DEPRECATION: hdate_get_month_string is now only a wrapper for hdate_string
- DEPRECATION: hdate_get_parasha_string is now only a wrapper for hdate_string
- DEPRECATION: hdate_get_omer_string is now only a wrapper for hdate_string
- DEPRECATION: hdate_get_int_string_ is now only a wrapper for hdate_string
- BUGFIX hdate_get_format_date, hdate-get_day_string, hdate_get_int_string no longer
  write to static local buffers. Callers to those functions must free().
- BUGFIX made all functions const-correct and fixed resulting conflicts

bindings - ruby
- BROKEN: We depend on 'swig', which is no longer supporting ruby 1.x.


3] Installation from source (Instructions)
------------------------------------------

  This package is available for most major operating systems, so in
  most cases if you just want to use the command line programs, the
  following instructions are unnecessary; just install your
  distribution's pre-built package, or a packaged binary from a
  reliable repository compatable with your distribution.


3.1 Pre-Requisites
------------------

  GNU toolchain, ie. gcc, libtoolize, autoreconf, and make. Your
  distribution may package these as `build-essential`, `libtool`,
  `autoconf`, and `automake`.

  This package was tested using the following toolchain versions:

    libtool (GNU libtool) 2.4.6
    autoconf (GNU Autoconf) 2.69
    automake (GNU automake) 1.16.1
    gcc (Debian 8.2.0-4) 8.2.0
    ldd (Debian GLIBC 2.27-5) 2.27

  Additonally, in order to build the bindings to other languages:
    swig 3.0.10
    g++ (Debian 8.2.0-4) 8.2.0
    g++-multilib 8.2.0  (for php)
    php 5.6.36
    python 2.7
    perl 5.26.2

3.2 Procedure
-------------
  Run the provided shell script `autogen.sh`.

  The script will invoke `libtoolize`, `autoreconf`, `configure`, and
  `make`. Upon success, you can either use the built files at their
  current paths, or you can perform `sudo make install` to install
  them system-wide on your machine.

3.3 Extra credit
----------------

  The extensive manuals and documentation for the elements of the GNU
  toolchain are a reliable source of interminable travail,
  frustration, torture, and enlightenment for all those who would dare
  turn to them. The search engine of your choice also works.
