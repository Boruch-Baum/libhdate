/** hcal.c            http://libhdate.sourceforge.net
 * Hebrew calendar (part of package libhdate)
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hcal.c -o hcal
 *
 *  Copyright (C) 2011-2018,2022 Boruch Baum  <boruch_baum@gmx.com>
 *                2004-2010 Yaacov Zamir <kzamir@walla.co.il>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//gcc -Wall -c -g -I "../../src" "%f"
//gcc -Wall -c -g -fmudflap -funwind-tables -I "../../src" "%f"
//gcc -Wall -g -I "../../src" -L"../../src/.libs" -lhdate -efence -o "%e" "%f"
//gcc -Wall -g -I "../../src" -L"../../src/.libs" -lhdate -lmudflap -rdynamic -o "%e" "%f"

#include <hdate.h>		/// For hebrew date (gcc -I ../../src)
#include <support.h>	/// libhdate general macros, including for gettext
#include <stdio.h>		/// For printf
#include <stdlib.h>		/// For atoi, malloc
#include <stdbool.h>  /// For boolean type
#include <locale.h>		/// For setlocale
#include <getopt.h>		/// For getopt_long
#include <string.h>		/// For strlen, mempcpy
#include <fnmatch.h>	/// For fnmatch
#include <time.h>		/// For time, tzset
#include <error.h>		/// For error
#include <errno.h>		/// For errno
//#include <wchar.h>	/// for unicode character operations
//#include "memwatch.h"	//  REMOVE - for debugging only
#include "local_functions.h" /// hcal,hdate common_functions
#include "custom_days.h" /// hcal,hdate common_functions
#include "timezone_functions.h"		/// for get_tz_adjustment


// Here temporarily for hdate_parse_date
// Should be moved to a header file
#define HDATE_PREFER_YM 1
#define HDATE_PREFER_MD 0


#define SHABBAT 7

#define HDATE_STRING_SHORT 1

#define CALENDAR_WIDTH_WIDE   41
#define CALENDAR_WIDTH_NARROW 20

// for opt.menu[MAX_MENU_ITEMS]
#define MAX_MENU_ITEMS 10

// for custom days
#define BAD_CUSTOM_DAY_CNT -1

// for colorization
#define CODE_BOLD_VIDEO    "%c[1m", 27
#define CODE_REVERSE_VIDEO "%c[7m", 27
#define CODE_RESTORE_VIDEO "%c[m", 27
#define CODE_BLACK         "%c[30m", 27
#define CODE_LIGHT_RED     "%c[31m", 27
#define CODE_LIGHT_GREEN   "%c[32m", 27
#define CODE_LIGHT_BROWN   "%c[33m", 27
#define CODE_DARK_BLUE     "%c[34m", 27
#define CODE_LIGHT_PURPLE  "%c[35m", 27
#define CODE_LIGHT_AQUA    "%c[36m", 27
#define CODE_LIGHT_GREY    "%c[37m", 27
#define CODE_BOLD_GREY     "%c[1;30m", 27
#define CODE_BOLD_RED      "%c[1;31m", 27
#define CODE_BOLD_GREEN    "%c[1;32m", 27
#define CODE_BOLD_YELLOW   "%c[1;33m", 27
#define CODE_BOLD_BLUE     "%c[1;34m", 27
#define CODE_BOLD_PURPLE   "%c[1;35m", 27
#define CODE_BOLD_AQUA     "%c[1;36m", 27
#define CODE_BOLD_WHITE    "%c[1;37m", 27
#define CODE_BACK_BLUE     "%c[46m", 27
#define ELEMENT_WEEKDAY_G      1
#define ELEMENT_WEEKDAY_H      2
#define ELEMENT_SHABBAT_DAY    3
#define ELEMENT_HOLIDAY_DAY    4
#define ELEMENT_HOLIDAY_FLAG   5
#define ELEMENT_SHABBAT_NAME   6
#define ELEMENT_WEEKDAY_NAMES  7
#define ELEMENT_MONTH_G        8
#define ELEMENT_MONTH_H        9
#define ELEMENT_SHABBAT_TIMES 10
#define ELEMENT_PARASHA       11
#define ELEMENT_THIS_SHABBAT_TIMES 12
#define ELEMENT_THIS_PARASHA  13
#define ELEMENT_HOLIDAY_NAME  14
#define ELEMENT_TODAY_HOLIDAY_DAY  15
#define ELEMENT_TODAY_HOLIDAY_NAME 16

static const char* month_text = N_("month");
static const char* year_text = N_("year");

#define BAD_HOLIDAY_TYPE 999
static char holiday_flag[] = { '/', '+', '*', '~', '!', '@', '$' };
/**********************************************************
    Holiday types: (reference hdate_holyday.c)
	/ - 0 - Regular day
	+ - 1 - Yom tov (plus yom kippor)
	* - 2 - Erev yom kippur, Erev Pesach, Erev Shavuot
	~ - 3 - Hol hamoed
	! - 4 - Hanuka and purim
	@ - 5 - Tzomot
	$ - 6 - Lag baomer ,Tu beav, Tu beshvat
**********************************************************/

// option_list structure definition
typedef struct {
  int prefer_hebrew;  // TODO - Make this user-selectable
  int base_year_h;    // TODO - Make this user-selectable
  int base_year_g;    // TODO - Make this user-selectable
  int gregorian;      // 0=Hebrew only, 1=Hebrew prime, else gregorian prime
  int html;
  int external_css;
  int diaspora;
  int parasha;
  int shabbat;
  int candles;
  int havdalah;
  int no_reverse;
  int three_month;
  int one_year;       // get tzif data only once
  char* border_spacing;
  char* separator;    // BB 2022-01-16: deprecating this
  int colorize;
  int bold;
  int footnote;
  int jd_today_g;
  int jd_today_h;
  int force_hebrew;
  int force_israel;
  int not_sunset_aware;
  int quiet_alerts;
  int bidi;
  bool mlterm;
  double lat;
  double lon;
  double tz_lon;      // for sanity checking against user-entered longitude
  int tz;
  char* tz_name_str;
  int custom_days_cnt;
  int custom_days_index;
  int* jdn_list_ptr;      // for custom_days
  char* string_list_ptr;  // for custom_days
  int tzif_entries;
  int tzif_index;         // counter into tzif_entries
  void* tzif_data;
  time_t epoch_start;     // for dst transition calc
  time_t epoch_end;       // for dst transition calc
  int menu;
  char* menu_item[MAX_MENU_ITEMS];
} option_list;
// END: option_list structure definition

// for option --borders
static char* default_borders_spacing = " | ";
static char* default_borders_separator = "=";

// for option --three-month
static char* default_spacing = "  ";

// for first line of all calendars
typedef struct {
  int g_month_1;
  int g_year_1;
  int g_month_2;
  int g_year_2;
  int h_month_1;
  int h_year_1;
  int h_month_2;
  int h_year_2;
} header_info;

static const char* hcal_config_file_text = N_("\
# configuration file for hcal - Hebrew calendar program\n\
# part of package libhdate\n\
#\n# Should you mangle this file and wish to restore its default content,\n\
# rename or delete this file and run hcal; hcal will automatically\n\
# regenerate the default content.\n#\n\
# Your system administrator can set system-wide defaults for hcal by\n\
# modifying file <not yet implemented>.\n\
# You may override all defaults by changing the contents of this file.\n\
#\n\
# Version information\n\
# This may be used by updates to hcal to determine how to parse the file\n\
# and whether additional information and options should be appended to\n\
# the end of this file.\n\
VERSION=2.00\n\
# Location awareness\n\
# hcal wants to accurately highlight the current Hebrew day, including\n\
# during the hours between sunset and secular midnight. If you don't\n\
# provide it with latitude, longitude, and time zone information, hcal\n\
# will try to guess the information, based upon your system timezone,\n\
# and its (limited, and maybe biased) of the dominant Jewish community\n\
# in that timezone. When hcal is forced to guess, it alerts the user\n\
# with a message that includes the guessed location.\n\
# hcal's guesses will also affect its default behaviour for output of\n\
# Shabbat times, parshiot, and choice of Israel/diaspora hoidays\n\
#SUNSET_AWARE=TRUE\n\n\
# Base Years\n\
# hcal, by default, interprets two-digit year parameters as Hebrew\n\
# years, and adds the value BASE_YEAR_HEBREW *100 to them. Set\n\
# variable PREFER_HEBREW=FALSE to have them interpreted as gregorian,\n\
# and have BASE_YEAR_GREGORIAN * 100 added to them.\n\
#PREFER_HEBREW=TRUE\n\
# valid values for BASE_YEAR_HEBREW are 30 - 69\n\
#BASE_YEAR_HEBREW=57\n\
# valid values for BASE_YEAR_GREGORIAN are 10 - 29\n\
#BASE_YEAR_GREGORIAN=20\n\n\
# LATITUDE and LONGITUDE may be in decimal format or in the form\n\
# degrees[:minutes[:seconds]] with the characters :'\" as possible\n\
# delimiters. Use negative values to indicate South and West, or\n\
# use the abbreviated compass directions N, S, E, W.\n\
#LATITUDE=\n\
#LONGITUDE=\n\
# TIMEZONE may may be in decimal format or in the form degrees[:minutes]\n\
# with the characters :'\" as possible delimiters.\n\
#TIMEZONE=\n\n\
# Israel and the diaspora\n\
# If hcal guesses that you're not in Israel, the DIASPORA option will be\n\
# set true. This will affect holiday and parasha output.  FORCE_ISRAEL\n\
# forces hcal to display calendar information for Israel, using hcal's\n\
# default coordinates foe Israel, or coordinates that you provide that\n\
# seem legitmately within Israel.\n\
# Thus, if you are living in Sao Paolo, and will be visiting Israel for\n\
# Sukkot, set BOTH flags true in order to see holiday information for\n\
# a non-resident vistor to Israel. The command line options for these\n\
# features are --israel, -I, --diaspora, -d.\n\
#FORCE_DIASPORA=FALSE;\n\
#FORCE_ISRAEL=FALSE;\n\n\
# Shabbat related\n\
# Setting SHABBAT_INFO true will output parshiot and Shabbat times.\n\
# The command line options for these features are -p (--parasha), and\n\
# -s (--shabbat). The CANDLE_LIGHTING field can accept a value of 18 - 90 (minutes\n\
# before sunset). The HAVDALAH field can accept a value of 20 - 90\n\
# (minutes after sunset).\n\
#PARASHA_NAMES=FALSE\n\
#SHABBAT_INFO=FALSE\n\
#CANDLE_LIGHTING=FALSE\n\
#HAVDALAH=FALSE\n\n\
# Holiday identification\n\
# hcal flags holidays by replacing the gregorian/Hebrew date separator\n\
# with assorted unhelpful cryptic symbols. Setting FOOTNOTES to true\n\
# will have hcal output after the month's calendar, a list of the month's\n\
# holidays along with the days on which they occur.\n\
#FOOTNOTE=FALSE\n\n\
# Output in hebrew characters\n\
# hcal defaults to output all information in your default language, so\n\
# if your default language is Hebrew, you're all set. Otherwise, you can\n\
# set FORCE_HEBREW to true to output Hebrew information in Hebrew, while\n\
# still outputting gregorian information in your default language. To\n\
# output ALL information in Hebrew, run something like this:\n\
#    LC_TEMP=LC_ALL; LC_ALL=\"he_IL.UTF-8\"; hcal; LC_ALL=$LC_TEMP\n\
# If setting FORCE_HEBREW to true results in 'garbage' or non-Hebrew\n\
# output, you need to install a terminal font that includes the Hebrew\n\
# character set (hint: unicode).\n\
# The command line option for FORCE_HEBREW is either --hebrew or -H\n\
#FORCE_HEBREW=FALSE\n\n\
# The FORCE_HEBREW option outputs data that is 'correct' and 'logical'.\n\
# Unfortunately, the world can not be depended upon to be either. Most\n\
# Xwindow applications will display the data fine with FORCE_HEBREW; most\n\
# xterm implementations will not. (in fact, the only xterm clone I know\n\
# of that comes close is mlterm). If using FORCE_HEBREW results in\n\
# Hebrew characters being displayed in reverse, set OUTPUT_BIDI to true.\n\
# This will reverse the order of the Hebrew characters, so they will\n\
# display 'visual'ly correct; however, such output will not be suitable\n\
# for piping or pasting to many other applications. Setting OUTPUT_BIDI\n\
# automatically forces hebrew.\n\
# The command line option for OUTPUT_BIDI is either --bidi, --visual, or -b\n\
#OUTPUT_BIDI=FALSE\n\n\
# Display enhancements\n\
# hcal defaults to display the current day in reverse video\n\
# The command line option for this feature is --no-reverse\n\
#SUPPRESS_REVERSE_VIDEO=FALSE;\n\
# hcal can display its output \"calming, muted tones\". Note that piping\n\
# colorized output may yield unexpected results.\n\
#COLORIZE=FALSE\n\n\
# HTML OUTPUT\n\
#OUTPUT_HTML=FALSE\n\
#USE_EXTERNAL_CSS_FILE=\"pathto/foo/bar\"\n\n\
# Suppress alerts and warnings\n\
# hcal alerts the user via STDERR when it guesses the user's location.\n\
#QUIET_ALERTS=FALSE\n\n\
# Three month display\n\
# hcal can display a previous, current and next month side-by-side. hcal\n\
# can also display a calendar for an entire year in four rows of three\n\
# months each. Note that this will display properly only if your console\n\
# is set for at least 127 columns. Note also that setting this option to\n\
# will cause options FOOTNOTES, SHABBAT, and PARASHA_NAMES to false.\n\
#THREE_MONTH=FALSE\n\n\
# User-defined menus\n\
# You may specify here command-line strings to optionally be parsed\n\
# by hcal at execution time. To do so, use the command line option -m\n\
# (--menu). hcal will process first the settings of this config file,\n\
# then the other settings of your command line, and then will prompt\n\
# you for which menu item you would like to select. hcal will process\n\
# your menu selection as if it were a new command line, further modifying\n\
# all the prior settings.\n\
# Only the first ten \"MENU=\" entries will be read. Each line will be\n\
# truncated at one hundred characters\n\
#MENU= -l -23.55 -L -46.61 -z -3      # parents in Sao Paolo\n\
#MENU= -l 32 -L 34 -z 2               # son in bnei brak\n\
#MENU= -fbc -l 43.71 -L -79.43 -z -5  # me in Toronto\n\
#MENU= -l 22.26 -L 114.15 -z 8        # supplier in Hong Kong\n\
");

/**************************************************
*  print version
*************************************************/
int print_version ()
{
  printf ("hcal (libhdate) 1.8\n\
Copyright (C) 2011-2013, 2022 Boruch Baum; 2004-2010 Yaacov Zamir\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n");
  return 0;
}

/**************************************************
*  print usage
*************************************************/
void print_usage_hcal ()
{
  printf ("%s\n",
N_("Usage: hcal [options] [coordinates] [timezone] [date_spec]\n\
       hdate [options] [coordinates] [timezone] [julian_day|time_t]\n\n\
       coordinates: -l [NS]yy[.xxx]     -L [EW]xx[.xxx]\n\
                    -l [NS]yy[:mm[:ss]] -L [EW]xx[:mm[:ss]]\n\n\
       timezone:    -z [ [nn[.nn|:mm]] | tz_name ]\n\n\
       date_spec:   [ [[month] year] | flexible_entry ]"));
}

/**************************************************
*  print "try --help" message
*************************************************/
void print_try_help_hcal ()
{
  printf ("%s\n", N_("Try \'hcal --help\' for more information"));
}

/**************************************************
*  print help
*************************************************/
void print_help ()
{
  print_usage_hcal();
  printf ("%s\n", N_("Hebrew calendar\nOPTIONS:\n\
   -0 --no-gregorian  default, print only Hebrew month information.\n\
                      exists to override config file setting\n\
   -1 --one-month     over-ride config file setting if you had set\n\
                      option --three-month as a default there\n\
   -3 --three-month   displays previous/next months\n\
                      side by side. requires 127 columns\n\
      --borders       display a separator  between  months\n\
   -b --bidi          prints hebrew in reverse (visual)\n\
      --visual\n\
      --no-bidi       over-ride config file setting if you had set\n\
      --no-visual     option -bidi as a default there\n\
   -B --bold          display special days in boldface, no color\n\
   -c --colorize      display in calming, muted tones. -cc will display\n\
                      in bolder, and mildy irritating, colors\n\
      --no-color      over-ride a --colorize or --bold setting\n\
   -d --diaspora      use diaspora reading and holidays.\n\
   -f --footnote      print descriptive notes of holidays\n\
      --no-footnote   over-ride a footnote setting in your config file\n\
   -g --gregorian     include gregorian corresponding days. -gg displays\n\
                      a gregorian month with corresponding Hebrew days.\n\
   -h --html          output in html format to stdout\n\
   -H --hebrew        print hebrew information in Hebrew\n\
   -i                 use external css file \"./hcal.css\".\n\
   -I --israel        override a diaspora default\n\
      --no-reverse    do not highlight today's date\n\
   -m --menu          prompt user-defined menu from config file\n\
   -p --parasha       print week's parasha on each calendar row\n\
   -q --quiet-alerts  suppress warning messages\n\
   -s --shabbat       print Shabbat times and parshiot\n\
      --candles       modify default minhag of 20 minutes. (17<n<91)\n\
      --havdalah      modify default minhag of 3 stars. (19<n<91 minutes)\n\
   -z --timezone nn   timezone, +/-UTC\n\
   -l --latitude yy   latitude yy degrees. Negative values are South\n\
   -L --longitude xx  longitude xx degrees. Negative values are West\n\n\
   --prefer-hebrew    interpret ambiguous mm yy as Hebrew date\n\
   --prefer-gregorian interpret ambiguous mm yy as gregorian date\n\n\
All options can be made default in the config file, or menu-ized for\n\
easy selection.\n\
Report bugs to: <http://sourceforge.net/tracker/?group_id=63109&atid=502872>\n\
libhdate home page: <http://libhdate.sourceforge.net>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>\n"));
}


/**************************************************
*  HTML - print header
*************************************************/
int html_print_header ( const int external_css, const int force_hebrew)
{
  printf ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\"http://www.w3.org/TR/html4/strict.dtd\">\n\
<html>\n\
<head>\n\
<meta name=\"generator\" content=\"hcal (libhdate)\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n\
<style title=\"Normal\" type=\"text/css\" media=\"all\">");

  if (external_css) printf ("\n\t@import \"hcal.css\";\n");
  else
  {
    printf ("\n\
img { margin:0; padding: 0;  vertical-align: middle;  border: 0;}\n\
p {}\n\
body { background-color: #ffffff; color: #000000; }\n\
# body.hebrew { direction: rtl;}\n\
table.main { table-layout: fixed; font-size: 14pt; border: solid #aaaaaa; }\n\
table.day {  width: 100%%; height: 100%%; table-layout: fixed; font-size: 14pt; }\n\
th.gmonth { font-size: 30pt; font-weight: bold }\n\
th.gyear { font-size: 30pt; font-weight: bold}\n\
th.hmonth { font-size: 30pt; font-weight: bold}\n\
th.hyear {font-size: 30pt; font-weight: bold }\n\
th.week { background-color: #aaaaaa;  text-align: center; }\n\
# td.gday { font-size: 30pt; vertical-align: top; }\n\
# td.hday { font-size: 12pt; vertical-align: top; }\n\
td.primary_day   { width: 25%%; font-size: 26pt; vertical-align: top; }\n\
td.secondary_day { width: 70%%; font-size: 12pt; vertical-align: top; }\n\
td.sat { color: #000099; border: solid #000099; }\n\
td.regular { border: solid #aaaaaa; }\n\
td.holiday { color: #000099; border: solid #000099; }\n\
td.holiday_name { font-size: 12pt; color: #000099; }\n\
td.out_of_month { background-color: #dddddd; color: #aaaaaa; border: solid #aaaaa; }\n\
tr.line1 { height: 30%%; }\n\
tr.line2 { height: 50%%; font-size: 30pt; }\n\
tr.line3 { height: 20%%; font-size: 80%%; vertical-align: bottom; }\n\
span.today { }\n\
span.gmonth { font-size: 24pt; }\n\
span.gyear { font-size: 24pt; }\n\
span.hmonth { font-size: 24pt; }\n\
span.hyear { font-size: 24pt; }\n");
  }
  printf ("</style>\n");

  /// some alternative css styles
  printf ("\n\
<link rel=\"alternate stylesheet\" title=\"High contrast\" type=\"text/css\" media=\"screen\" href=\"high_contrast.css\">\n\
<link rel=\"alternate stylesheet\" title=\"Colorful\" type=\"text/css\" media=\"screen\" href=\"colorful.css\">\n\
<link rel=\"alternate stylesheet\" title=\"Print\" type=\"text/css\" media=\"all\" href=\"print.css\">\n");
  if ( (force_hebrew) || (hdate_is_hebrew_locale()) )
    printf ("<title>Hebrew Calendar</title>\n</head>\n<body class=\"hebrew\"><center>\n");
  else
    printf ("<title>Hebrew Calendar</title>\n</head>\n<body><center>\n");
  return 0;
}



/**************************************************
 *  HTML - print footer
 *************************************************/
int html_print_footer ()
{
  printf ("<!-- <p>\n\
<a href=\"http://validator.w3.org/check/referer\">\n<img \
src=\"http://www.w3.org/Icons/valid-html401\"\n\
alt=\"Valid HTML 4.01!\" height=\"31\" width=\"88\" />\n</a>\n\
<a href=\"http://jigsaw.w3.org/css-validator\">\n\
<img src=\"http://www.w3.org/Icons/valid-css\" alt=\"Valid CSS!\">\n\
</a>\n</p> -->\n</body></html>\n");
  return 0;
}


/**************************************************
*  Print HTML header - month line
*************************************************/
void html_print_calendar_header( const header_info header, const int calendar_type, int calendar_lang)
{
  char *h_year_1 = NULL;
  char *h_year_2 = NULL;
  char *align1 = NULL;
  char *align2 = NULL;
  char *bidi_dir = NULL;

  switch (calendar_type)
  {
  case 0: // Hebrew only
    if (calendar_lang) { align1="right"; bidi_dir="rtl"; }
    else               { align1="left";  bidi_dir="ltr"; }
    h_year_1 = hdate_string(HDATE_STRING_INT, header.h_year_1, HDATE_STRING_LONG, calendar_lang);
    // Hebrew month and year
    printf ("<table width=90%% dir=%s><tr><th align=%s><span class=\"hmonth\">%s </span><span class=\"hyear\">%s</th></tr></table>\n",
        bidi_dir, align1,
        hdate_string( HDATE_STRING_HMONTH , header.h_month_2, HDATE_STRING_LONG, calendar_lang),
        h_year_1);
    break;
  case 1: // Hebrew primary, with gregorian correspondence
    if (calendar_lang) { align1="right"; align2="left";   bidi_dir="rtl"; }
    else               { align1="left";  align2="right";  bidi_dir="ltr"; }
    h_year_1 = hdate_string(HDATE_STRING_INT, header.h_year_1, HDATE_STRING_LONG, calendar_lang);
    // Hebrew month and year
    printf ("<table width=90%% dir=%s><tr><th align=%s><span class=\"hmonth\">%s </span><span class=\"hyear\">%s</span></th>",
        bidi_dir, align1,
        hdate_string( HDATE_STRING_HMONTH , header.h_month_2, HDATE_STRING_LONG, calendar_lang),
        h_year_1);
    // gregorian month and year
    printf ("<th align=%s><span class=\"gmonth\">", align2);
    if (header.g_month_1 == header.g_month_2)
    {
      printf ("%s </span><span class=\"gyear\">%d</span></th></tr></table>\n",
          hdate_string( HDATE_STRING_GMONTH , header.g_month_1, HDATE_STRING_LONG, calendar_lang),
          header.g_year_1);
    }
    else
    {
      if (header.g_year_1 != header.g_year_2)
      {
        printf ("%s </span><span class=\"gyear\">%d - </span><span class=\"gmonth\">%s </span><span class=\"gyear\">%d</span></th></tr></table>\n",
          hdate_string( HDATE_STRING_GMONTH , header.g_month_1, HDATE_STRING_LONG, calendar_lang),
          header.g_year_1,
          hdate_string( HDATE_STRING_GMONTH , header.g_month_2, HDATE_STRING_LONG, calendar_lang),
          header.g_year_2);
      }
      else
      {
        printf ("%s - %s </span><span class=\"gyear\">%d</span></th></tr></table>\n",
          hdate_string( HDATE_STRING_GMONTH , header.g_month_1, HDATE_STRING_LONG, calendar_lang),
          hdate_string( HDATE_STRING_GMONTH , header.g_month_2, HDATE_STRING_LONG, calendar_lang),
          header.g_year_1);
      }
    }
    break;
  default:// gregorian primary, with Hebrew correspondence
    // gregorian month and year
    printf ("<table width=90%% dir=ltr><tr><th align=left><span class=\"gmonth\">%s </span><span class=\"gyear\">%d</span></th>",
        hdate_string( HDATE_STRING_GMONTH, header.g_month_1, HDATE_STRING_LONG, calendar_lang),
        header.g_year_1);
    h_year_1 = hdate_string(HDATE_STRING_INT, header.h_year_1, HDATE_STRING_LONG, calendar_lang);
    // Hebrew month and year
    printf ("<th align=right><span class=\"hmonth\">");
    if (header.h_month_1 == header.h_month_2)
    {
      printf ("%s </span><span class=\"hyear\">%s</span></th></tr></table>\n",
          hdate_string( HDATE_STRING_HMONTH , header.h_month_1, HDATE_STRING_LONG, calendar_lang),
          h_year_1);
    }
    else
    {
      if (header.h_year_1 != header.h_year_2)
      {
        h_year_2 = hdate_string(HDATE_STRING_INT, header.h_year_2, HDATE_STRING_LONG, calendar_lang);
        printf ("%s </span><span class=\"hyear\">%s - </span><span class=\"hmonth\">%s </span><span class=\"hyear\">%s</span></th></tr></table>\n",
          hdate_string( HDATE_STRING_HMONTH , header.h_month_1, HDATE_STRING_LONG, calendar_lang),
          h_year_1,
          hdate_string( HDATE_STRING_HMONTH , header.h_month_2, HDATE_STRING_LONG, calendar_lang),
          h_year_2);
        free(h_year_2);
      }
      else
      {
        printf ("%s - %s </span><span class=\"hyear\">%s</span></th></tr></table>\n",
          hdate_string( HDATE_STRING_HMONTH , header.h_month_1, HDATE_STRING_LONG, calendar_lang),
          hdate_string( HDATE_STRING_HMONTH , header.h_month_2, HDATE_STRING_LONG, calendar_lang),
          h_year_1);
      }
      bidi_dir="ltr";    // force days of week to appear in ltr
                // BUG ? - what if locale is Israel or other rtl locale?
      calendar_lang = 0;  // force days of week to appear in local language
    }
    break;
  } // end switch (gregorian
  if (h_year_1 != NULL) free(h_year_1);
  printf ("<table class=main dir=%s>\n<tr>", bidi_dir);
  int j;
  for (j = 1; j < 8; j++)
    printf ("<th class=\"week\">%s</th>",
        hdate_string( HDATE_STRING_DOW, j, HDATE_STRING_LONG, calendar_lang));
  printf ("</tr>\n");
}
/**************************************************
 *  end HTML functions
 *************************************************/


/*************************************************
*  Display data in a more pleasing visual manner
*************************************************/
void colorize_element ( const int color_scheme, const int element )
{
  if ( color_scheme > 1 )  printf(CODE_BOLD_VIDEO);
  switch (element) {
  case ELEMENT_WEEKDAY_G: printf(CODE_LIGHT_GREY); break;
  case ELEMENT_WEEKDAY_H: printf(CODE_LIGHT_BROWN); break;
  case ELEMENT_MONTH_G: printf(CODE_LIGHT_GREY); break;
  case ELEMENT_MONTH_H: printf(CODE_LIGHT_BROWN); break;
  case ELEMENT_WEEKDAY_NAMES: printf(CODE_LIGHT_GREEN); break;
  case ELEMENT_SHABBAT_NAME: printf(CODE_LIGHT_AQUA); break;
  case ELEMENT_SHABBAT_DAY: printf(CODE_LIGHT_AQUA); break;
  case ELEMENT_HOLIDAY_DAY: printf(CODE_LIGHT_AQUA); break;
  case ELEMENT_SHABBAT_TIMES: printf(CODE_LIGHT_PURPLE); break;
  case ELEMENT_PARASHA: printf(CODE_LIGHT_GREEN); break;
  case ELEMENT_THIS_SHABBAT_TIMES: printf(CODE_LIGHT_GREEN); break;
  case ELEMENT_THIS_PARASHA: printf(CODE_LIGHT_GREEN); break;
  case ELEMENT_HOLIDAY_NAME: printf(CODE_LIGHT_GREY); break;
  case ELEMENT_TODAY_HOLIDAY_DAY: printf(CODE_LIGHT_GREEN); break;
  case ELEMENT_TODAY_HOLIDAY_NAME: printf(CODE_LIGHT_GREEN); break;
  }
}


/**************************************************
 *  print header - month line to stdout
 *************************************************/
void print_month_line( const header_info header, const option_list* opt )
{
  char *g_month, *h1_month, *h2_month;
  char *h1_year = NULL;
  char *h2_year = NULL;

  int padding = 0;
  size_t g_month_len, g_year_len, h1_month_len, h2_month_len, h1_year_len, h2_year_len;
  int calendar_width = CALENDAR_WIDTH_WIDE;
  #define DASH_WIDTH      3 // includes surrounding spaces
  #define YEAR_WIDTH      5 // includes surrounding spaces
  #define SPACE_WIDTH    1

  char *hebrew_buffer, *hebrew_buffer_next;
  size_t hebrew_buffer_len, literals_len;

  /**************************************************
  *  Print Gregorian month and year
  *************************************************/
  if (opt->gregorian == 0)
  {
    g_month_len = 0;
    g_year_len = 0;
    calendar_width = CALENDAR_WIDTH_NARROW;
  }
  else
  {
    g_month = hdate_string( HDATE_STRING_GMONTH, header.g_month_1, HDATE_STRING_LONG, HDATE_STRING_LOCAL);
    if (opt->colorize) colorize_element(opt->colorize, ELEMENT_MONTH_G);
    if ( (opt->gregorian != 1) ||
       ( (opt->gregorian == 1) && (header.g_month_1 == header.g_month_2) )||
       (header.g_year_1 != header.g_year_2) )
      printf ("%s %d", g_month, header.g_year_1);
    else printf ("%s", g_month);

    /*************************************************
    *  padding info for gregorian date
    *************************************************/
    g_month_len = strlen(g_month);
    g_year_len = YEAR_WIDTH;
    switch (header.g_year_1)
    {
    case 1-9:    g_year_len  = 2; break;
    case 10-99:    g_year_len  = 3; break;
    case 100-999:  g_year_len  = 4; break;
    }

    if ( (opt->gregorian == 1) && (header.g_month_1 != header.g_month_2) )
    {
      g_month = hdate_string( HDATE_STRING_GMONTH, header.g_month_2, HDATE_STRING_LONG, HDATE_STRING_LOCAL);
      printf (" - %s %d", g_month, header.g_year_2);

      /*************************************************
      *  padding info for gregorian date
      *************************************************/
      g_month_len = g_month_len + strlen(g_month) + DASH_WIDTH;
      if (header.g_year_1 != header.g_year_2)
      {
        g_year_len = g_year_len + YEAR_WIDTH;
        switch (header.g_year_1)
        {
        case 1-9:    g_year_len  = g_year_len - 3; break;
        case 10-99:    g_year_len  = g_year_len - 2; break;
        case 100-999:  g_year_len  = g_year_len - 1; break;
        }
      }

    }
    if (opt->colorize) printf(CODE_RESTORE_VIDEO);
  }


  /**************************************************
  *  Print Hebrew month and year
  *
  *  because we want this printed right-justified on
  *  the same line, we need to calculate how many
  *  spaces to pad.
  *
  *  because of the possible user option --bidi, we
  *  need to buffer the entire Hebrew text in order
  *  reverse its sequence for 'visual' display.
  *
  *  This all requires collecting the data first
  *  and determining each's length.
  *************************************************/

  /**************************************************
  *  collect padding info for Hebrew date
  *************************************************/
  h1_month = hdate_string( HDATE_STRING_HMONTH , header.h_month_1, HDATE_STRING_LONG, opt->force_hebrew);
  h1_year  = hdate_string( HDATE_STRING_INT, header.h_year_1, HDATE_STRING_SHORT, opt->force_hebrew);

  h1_month_len = strlen(h1_month);
  h1_year_len = strlen(h1_year);

  if (header.h_year_1 != header.h_year_2)
  {

    h2_month = hdate_string( HDATE_STRING_HMONTH , header.h_month_2, HDATE_STRING_LONG, opt->force_hebrew);
    h2_year = hdate_string(HDATE_STRING_INT, header.h_year_2, HDATE_STRING_SHORT,opt->force_hebrew);

    h2_month_len = strlen(h2_month);
    h2_year_len = strlen(h2_year);
    literals_len = SPACE_WIDTH + DASH_WIDTH + SPACE_WIDTH;

    hebrew_buffer_len = h1_month_len + h1_year_len + h2_month_len
              + h2_year_len + literals_len;

    hebrew_buffer = malloc(hebrew_buffer_len+1);
    hebrew_buffer_next = mempcpy(hebrew_buffer, h1_month, h1_month_len);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, " ", SPACE_WIDTH);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, h1_year, h1_year_len);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, " - ", DASH_WIDTH);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, h2_month, h2_month_len);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, " ", SPACE_WIDTH);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, h2_year, h2_year_len);
    hebrew_buffer[hebrew_buffer_len]='\0';
  }
  else if (header.h_month_1 != header.h_month_2)
  {
    h2_month = hdate_string( HDATE_STRING_HMONTH , header.h_month_2, HDATE_STRING_LONG, opt->force_hebrew);
    h2_month_len = strlen(h2_month);
    literals_len = DASH_WIDTH + SPACE_WIDTH;

    hebrew_buffer_len = h1_month_len + h2_month_len + h1_year_len + literals_len;
    hebrew_buffer = malloc(hebrew_buffer_len+1);
    hebrew_buffer_next = mempcpy(hebrew_buffer, h1_month, h1_month_len);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, " - ", DASH_WIDTH);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, h2_month, h2_month_len);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, " ", SPACE_WIDTH);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, h1_year, h1_year_len);
    hebrew_buffer[hebrew_buffer_len]='\0';
  }
  else
  {
    h2_month_len = 0;
    h2_year_len = 0;
    literals_len = SPACE_WIDTH;

    hebrew_buffer_len = h1_month_len + h1_year_len + literals_len;

    hebrew_buffer = malloc(hebrew_buffer_len+1);
    hebrew_buffer_next = mempcpy(hebrew_buffer, h1_month, h1_month_len);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, " ", SPACE_WIDTH);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, h1_year, h1_year_len);
    hebrew_buffer[hebrew_buffer_len]='\0';
  }

  if (!opt->force_hebrew)
    padding = calendar_width - g_month_len - g_year_len - hebrew_buffer_len ;
  else
  {
    padding = calendar_width - g_month_len - g_year_len - ((hebrew_buffer_len+1)/2) - literals_len + 1;
    if (opt->gregorian > 1)
    {
      if (header.h_month_1 != header.h_month_2)
      {
        padding = padding + 1;
        if ((header.h_month_1 > 12) ||
          (header.h_month_2 > 12)  ) padding = padding - 1;
        if (header.h_year_1 != header.h_year_2)  padding = padding +1;
      }
      else if (header.h_month_1 > 12) padding = padding - 1;
    }
    if (hdate_is_hebrew_locale()) padding = padding + (g_month_len+1)/2;
  }

  /****************************************************
  *  print padding
  ****************************************************/
  if (opt->gregorian != 0) printf("%*s",padding," ");
  else printf("%*s",((padding+2)/2)," ");


  /**************************************************
  *  Print Hebrew month and year
  *************************************************/
  if (opt->colorize) colorize_element(opt->colorize, ELEMENT_MONTH_H);
  if (opt->bidi) revstr(hebrew_buffer, hebrew_buffer_len);
  printf ("%s", hebrew_buffer);
  if (opt->colorize) printf(CODE_RESTORE_VIDEO);


  /**************************************************
  *  free allocated memory
  *************************************************/
  if (h1_year != NULL) free(h1_year);
//  if (header.h_year_1 != header.h_year_2)  free(h2_year);
  if (h2_year != NULL) free(h2_year);
  free(hebrew_buffer);

  /****************************************************
  *  print padding
  ****************************************************/
  if ( (opt->gregorian == 0) && (opt->three_month) )
    if (header.h_month_1 == 14) // Adar II
			printf("%*s",((padding-1)/2)-1," ");
    else
			printf("%*s",((padding-1)/2)," ");
}


/**************************************************
*  print column headings for days of weeks
**************************************************/
void print_dow_line( option_list* opt )
{
  int column;

  /**************************************************
  *  sub-function
  **************************************************/
  void print_dow_column( const int column )
  {
    int padding = 2;

    if (hdate_is_hebrew_locale() || opt->force_hebrew)
    {  // Hebrew heading is a single character per day
      if (!opt->gregorian) printf("%s%s"," ", hdate_string( HDATE_STRING_DOW, column, HDATE_STRING_SHORT, HDATE_STRING_HEBREW));
      else printf ("%s%s%s", "  ",
        hdate_string( HDATE_STRING_DOW, column, HDATE_STRING_SHORT, HDATE_STRING_HEBREW),
        " ");
    }
    else
    {
      if (opt->gregorian == 0)
      {
        putchar(' ');
        putchar( *(hdate_string( HDATE_STRING_DOW, column, HDATE_STRING_SHORT, HDATE_STRING_LOCAL)));
      }
      else
        printf ("%s%3s", " ",
            hdate_string( HDATE_STRING_DOW, column, HDATE_STRING_SHORT, HDATE_STRING_LOCAL));
    }

    if ( (column == 7) || (!opt->gregorian) ) padding = 1;
    if ( (column != 7) || ( ( column == 7) && (opt->gregorian) ) ) printf("%*s",padding," ");
  }

  /**************************************************
  *  begin function - print_dow_line
  **************************************************/
  if (opt->bidi)
  {
    if (opt->colorize) colorize_element(opt->colorize, ELEMENT_SHABBAT_NAME);
    print_dow_column(7);
    printf(" ");
    if (opt->colorize) colorize_element(opt->colorize, ELEMENT_WEEKDAY_NAMES);
    for (column = 6; column > 0; column--) print_dow_column(column);
  }
  else
  {
    if (opt->colorize) colorize_element(opt->colorize, ELEMENT_WEEKDAY_NAMES);
    for (column = 1; column < 7; column++) print_dow_column(column);
    if (opt->colorize) colorize_element(opt->colorize, ELEMENT_SHABBAT_NAME);
    print_dow_column(7);
  }
  if (opt->colorize) printf(CODE_RESTORE_VIDEO);
}


/********************************************************
*  calculate the number of the previous Hebrew month
*********************************************************/
int hmonth_previous( const int month, const hdate_struct * h )
{
  if (month == 13)
    return 5;
  else if ( (month == 7) && (h->hd_size_of_year > 365) )
    return 14;
  else
  return (month==1 ? 12 : month-1);
}
/********************************************************
*  calculate the number of the next Hebrew month
*********************************************************/
int hmonth_next( const int month, const hdate_struct * h )
{
  if (month == 14)
    return 7;
  else if ( (month == 5) && (h->hd_size_of_year > 365) )
    return 13;
  else
    return (month==12 ? 1 : month+1);

}


/****************************************************
 *  print a horizontal separator line in 3-month mode
 ***************************************************/
void print_separator( const option_list opt )
{
// BB 2022-01-16: I'm deciding to remove this feature as not visually
//                appealing and just more weird bidi work to do.
  printf("\n");
//  int calendar_width = CALENDAR_WIDTH_WIDE;
//  int i;
//
//  if (!opt.gregorian) calendar_width = CALENDAR_WIDTH_NARROW;
//  if (opt.three_month == 1)
//  {
//    for (i=0 ; i < calendar_width; i++) printf("%s",opt.separator);
//    printf("%s", opt.border_spacing);
//    for (i=0 ; i < calendar_width; i++) printf("%s",opt.separator);
//    printf("%s", opt.border_spacing);
//    for (i=0 ; i < calendar_width; i++) printf("%s",opt.separator);
//    printf("\n");
//  }
//  else
//  {
//    if (opt.three_month == 12)
//			printf("%*s%s", calendar_width," ", opt.border_spacing);
//    else
//    {
//      for (i=0 ; i < calendar_width; i++) printf("%s",opt.separator);
//      printf("%s", opt.border_spacing);
//    }
//    for (i=0 ; i < calendar_width; i++) printf("%s",opt.separator);
//    printf("%s\n", opt.border_spacing);
//  }
  return;
}

/**************************************************
 *  print header - year and month
 *************************************************/
int print_header ( const int month, const int year, option_list* opt)
{
  header_info previous_month, current_month, next_month;
  hdate_struct h;
  int calendar_width;
  int fourteenth_month = FALSE;

    /********************************************************
    *  embedded sub-function:
    *********************************************************/
    header_info gcal_start_end( const int month, const int year )
    {
      header_info header;

      hdate_set_gdate (&h, 1, month, year);
      header.g_month_1 = h.gd_mon;
      header.g_year_1 = h.gd_year;
      header.h_month_1 = h.hd_mon;
      header.h_year_1 = h.hd_year;

      hdate_set_jd ( &h, hdate_gdate_to_jd ( 1, month % 12 + 1, ((month==12) ? year+1 : year) ) - 1  );
      header.g_month_2 = h.gd_mon;
      header.g_year_2 = h.gd_year;
      header.h_month_2 = h.hd_mon;
      header.h_year_2 = h.hd_year;

      return header;
    }

    /********************************************************
    *  embedded sub-function:
    *********************************************************/
    header_info hcal_start_end( const int month, const int year )
    {
      header_info header;

      hdate_set_hdate (&h, 1, month, year);
      header.g_month_1 = h.gd_mon;
      header.g_year_1 = h.gd_year;
      header.h_month_1 = h.hd_mon;
      header.h_year_1 = h.hd_year;

      hdate_set_jd ( &h, h.hd_jd + hdate_get_size_of_hebrew_month( h.hd_mon, h.hd_year_type) -1  );
      header.g_month_2 = h.gd_mon;
      header.g_year_2 = h.gd_year;
      header.h_month_2 = h.hd_mon;
      header.h_year_2 = h.hd_year;
      return header;
    }

  /**************************************************************
  *  begin function: print_header (year and month)
  *  Preliminary - set dates for begining and end of calendar
  **************************************************************/

    /*****************************************************
    *         When opt->three_month == 12
    *
    *  this is an inelegant way to identify the possible
    *  fifth row of a calendar displaying three months
    *  across. This can happen for a Hebrew calendar in a
    *  leap year, or for a calendar displaying gregorian
    *  months for an entire Hebrew year. The possibilities
    *  are to print one or two months on this line.
    *****************************************************/
    if (opt->gregorian == 0)
    {
      current_month = hcal_start_end(month, year);
      if ( (opt->three_month) && (opt->three_month != 12) )
      {
        if ( (month == 5) || (month == 7) )
          hdate_set_hdate (&h, 1, month, year);

        previous_month.h_month_1 = hmonth_previous( month, &h );
        previous_month.h_year_1 = (month==1 ? year-1 : year);
        previous_month.h_month_2 = previous_month.h_month_1;
        previous_month.h_year_2 = previous_month.h_year_1;

        next_month.h_month_1 = hmonth_next( month, &h );
        next_month.h_year_1 = (month==12 ? year+1: year);
        next_month.h_month_2 = next_month.h_month_1;
        next_month.h_year_2 = next_month.h_year_1;
      }
    }
    else if (opt->gregorian == 1)
    {
      current_month = hcal_start_end(month, year);
      if ( (opt->three_month) && (opt->three_month != 12) )
      {
        if (month == 13)
          previous_month = hcal_start_end(5, year);
        else if ( (month == 7) && (h.hd_size_of_year > 355) )
          previous_month = hcal_start_end(14, year);
        else
          previous_month = hcal_start_end((month==1 ? 12 : month-1), (month==1 ? year-1 : year));
        if (month == 14)
          next_month = hcal_start_end(7, year);
        else if ( (month == 5) && (h.hd_size_of_year > 355) )
          next_month = hcal_start_end( 13, year);
        else
          next_month = hcal_start_end((month==12 ? 1 : month+1), (month==12 ? year+1: year));
      }
    }
    else
    {
      current_month = gcal_start_end(month, year);
      if (h.hd_size_of_year > 355) fourteenth_month = TRUE;
      if (opt->three_month)
      {
        if (opt->three_month != 12)
        {
          previous_month = gcal_start_end(
              (month==1 ? 12 : month-1), (month==1 ? year-1 : year));
          next_month = gcal_start_end(
              (month==12 ? 1 : month+1), (month==12 ? year+1: year));
        }
        else
        {
          if (h.hd_size_of_year > 355) previous_month = current_month;
          current_month  = gcal_start_end(
              (month==12 ? 1 : month+1), (month==12 ? year+1: year));
        }
      }

    }

  /******************************************************
  *  HTML output - print month header line and dow line
  ******************************************************/
  if (opt->html)
  {
    html_print_calendar_header(current_month, opt->gregorian, opt->force_hebrew);
  }

  /**************************************************
  *  STDOUT output - month header line
  *************************************************/
  else
  {
    if (opt->three_month)
    {
      if  ( (opt->three_month == 12) &&
        ( (fourteenth_month == FALSE) || (opt->gregorian <2) ) )
      {
          if (!opt->gregorian) calendar_width = CALENDAR_WIDTH_NARROW;
          else calendar_width = CALENDAR_WIDTH_WIDE;
          printf("%*s",calendar_width," ");
      }
      else print_month_line(previous_month, opt);
      printf("%s", opt->border_spacing);
    }
    print_month_line(current_month, opt);
    if (opt->three_month)
    {
      printf("%s", opt->border_spacing);
      if (opt->three_month != 12)
        print_month_line(next_month, opt);
    }
    printf ("\n");
    /**************************************************
    *  print column headings for days of weeks
    **************************************************/
    if (opt->three_month)
    {
      if  ( (opt->three_month == 12) &&
        ( (fourteenth_month == FALSE) || (opt->gregorian <2) ) )
        printf("%*s",calendar_width," ");
      else print_dow_line(opt);
      printf("%s", opt->border_spacing);
    }
    print_dow_line(opt);
    if (opt->three_month)
    {
      printf("%s", opt->border_spacing);
      if (opt->three_month != 12)
        print_dow_line(opt);
    }
    printf ("\n");
  }

  return 0;
}


/**************************************************
*  print HTML calendar entry (ie. a single day)
*************************************************/
void html_print_day ( const hdate_struct h, const int month, option_list* opt)
{

  int halachic_day;
  int* jdn_list_ptr;
  int custom_day_index_to_print = 0;
  char *day_text_ptr = NULL;
  char *hd_day_str = NULL;
  char *holiday_name_class_str = "holiday_name";
  char *holiday_name_align="left";

  halachic_day = hdate_get_halachic_day(&h, opt->diaspora);
  if ((!halachic_day) && (opt->custom_days_cnt))
  {
    for (  opt->custom_days_index = 0,
        jdn_list_ptr = opt->jdn_list_ptr
        ;
        (opt->custom_days_index < opt->custom_days_cnt) &&
        (!halachic_day)
        ;
        opt->custom_days_index = opt->custom_days_index + 1,
        jdn_list_ptr = jdn_list_ptr + 1
      )
    {
      if (h.hd_jd == *jdn_list_ptr)
      {
        halachic_day = BAD_HOLIDAY_TYPE;
        custom_day_index_to_print = opt->custom_days_index;
      }
    }
  }
  if (( (opt->gregorian < 2) && (h.hd_mon != month)) ||
    ( (opt->gregorian > 1) && (h.gd_mon != month)) )
  {
    holiday_name_class_str = "out_of_month";
    printf ("<td class=\"out_of_month\">");
  }
  else if (h.hd_dw == SHABBAT) printf ("<td class=\"sat\">");
  else if (halachic_day) printf ("<td class=\"holiday\">");
  else printf ("<td class=\"regular\">");
  // Print a day
  if (opt->gregorian == 0) // calendar type = Hebrew only
  {
    hd_day_str = hdate_string(HDATE_STRING_INT, h.hd_day, HDATE_STRING_LONG,opt->force_hebrew);
    printf ("<table class=day><tr class=line1><td class=\"primary_day\">%3s</td></tr>\
<tr class=line2><td>&nbsp;</td></tr>", hd_day_str);
  }
  else if (opt->gregorian == 1) // calendar type = Hebrew primary
  {  // TODO - here
    hd_day_str = hdate_string(HDATE_STRING_INT, h.hd_day, HDATE_STRING_LONG,opt->force_hebrew);
    printf ("<table class=day><tr class=line1><td class=\"primary_day\">%3s</td>\
<td class=\"secondary_day\">%2d&nbsp;%s</td></tr><tr class=line2><td>&nbsp;</td></tr>",
      hd_day_str, h.gd_day,
      hdate_string( HDATE_STRING_GMONTH , h.gd_mon, HDATE_STRING_LONG, opt->force_hebrew));
  }
  else // (opt->gregorian > 1) // calendar type = Gregorian primary
  {
    hd_day_str = hdate_string(HDATE_STRING_INT, h.hd_day, HDATE_STRING_LONG,opt->force_hebrew);
    printf ("<table class=day><tr class=line1><td class=\"primary_day\">%2d</td>\
<td class=\"secondary_day\">%3s&nbsp;%s</td></tr><tr class=line2><td>&nbsp;</td></tr>",
      h.gd_day, hd_day_str,
      hdate_string( HDATE_STRING_HMONTH , h.hd_mon, HDATE_STRING_LONG, opt->force_hebrew));
  }
  if (halachic_day)
  {
    if (opt->force_hebrew) holiday_name_align="right";
    if (halachic_day == BAD_HOLIDAY_TYPE)
    {
      day_text_ptr = get_custom_day_text_ptr(custom_day_index_to_print, opt->string_list_ptr);
    }
    else
    {
      day_text_ptr = hdate_string( HDATE_STRING_HOLIDAY, halachic_day, HDATE_STRING_LONG, opt->force_hebrew);
    }
    printf ("<tr class=line3><td align=%s class=\"%s\" colspan=2>%s</td></tr>",
        holiday_name_align, holiday_name_class_str, day_text_ptr);
  }
  else printf ("<tr class=line3><td class=\"%s\">&nbsp;</td></tr>",holiday_name_class_str);

  printf ("</table></td>\n");

  if (hd_day_str != NULL) free(hd_day_str);
}


/*************************************************
*  print stdout calendar entry (ie. a single day)
*************************************************/
void print_day ( const hdate_struct h, const int month, option_list* opt, const int printing_footnote,  char* custom_day_flag)
{
  char *hd_day_str = NULL;
  size_t hd_day_buffer_str_len;
  int holiday_type = 0;
  char* day_flag;

  /*****************************************************
  *  embedded sub-function: colorize_prefix
  *****************************************************/
  void colorize_prefix()
  {
    if (opt->colorize)
    {
      if (h.hd_dw==7) colorize_element(opt->colorize, ELEMENT_SHABBAT_DAY);
      else if ( (holiday_type) || (custom_day_flag) ) colorize_element(opt->colorize, ELEMENT_HOLIDAY_DAY);
      else            colorize_element(opt->colorize, ELEMENT_WEEKDAY_H);
    }
    else if ( (!printing_footnote) && (opt->bold) &&
          ( (h.hd_dw==7) || (holiday_type) || (custom_day_flag) ) )
      printf(CODE_BOLD_VIDEO);
  }


  /*****************************************************
  *  embedded sub-function: get_todays_holiday_data
  *****************************************************/
  void get_todays_holiday_data()
  {
    if (custom_day_flag)
    {
      day_flag = custom_day_flag;
      return;
    }
    holiday_type = hdate_get_halachic_day_type(hdate_get_halachic_day(&h, opt->diaspora));
    day_flag = &holiday_flag[holiday_type];
    if ((!holiday_type) && (opt->custom_days_cnt))
    {
      // there may be more than one custom day for a single
      // calendar day, so we use the symbol for the first
      // but will need to iterate over the others for the
      // next calendar day's symbol

      // This next snippet might be good in the future
      // but for now the jdn_list isn't sorted ..
      //for (  ;
      //    (opt->custom_days_index < opt->custom_days_cnt) &&
      //    (h.hd_jd > *jdn_list_ptr) ;
      //    opt->custom_days_index = opt->custom_days_index + 1,
      //    jdn_list_ptr = jdn_list_ptr + 1
      //    ) ;
      //if (h.hd_jd == *jdn_list_ptr)
      //{
      //  day_flag = get_custom_day_symbol_ptr(opt->custom_days_index, opt->string_list_ptr);
      //  holiday_type = BAD_HOLIDAY_TYPE;
      //}
      int* jdn_list_ptr;
      for (  opt->custom_days_index = 0,
          jdn_list_ptr = opt->jdn_list_ptr
          ;
          (opt->custom_days_index < opt->custom_days_cnt) &&
          (!holiday_type)
          ;
          opt->custom_days_index = opt->custom_days_index + 1,
          jdn_list_ptr = jdn_list_ptr + 1
        )
      {
        if (h.hd_jd == *jdn_list_ptr)
        {
          day_flag = get_custom_day_symbol_ptr(opt->custom_days_index, opt->string_list_ptr);
          holiday_type = BAD_HOLIDAY_TYPE;
        }
      }
    }
  }


  /*****************************************************
  *  embedded sub-function: print_gregorian_day_and_print_holiday_flag()
  *     print the gregorian part of the day's entry,
  *     and the holiday flag
  *****************************************************/
  void print_gregorian_day_and_print_holiday_flag()
  {
    /*************************************************
    *  holiday flag
    *************************************************/
    if (opt->gregorian == 1) printf ("%c", *day_flag);

    /*************************************************
    *  Gregorian date entry - color prefix
    *************************************************/
    if (h.hd_jd == opt->jd_today_g)
        printf (CODE_REVERSE_VIDEO);
    else colorize_prefix();

    /*************************************************
    *  Gregorian date entry - the day of the month
    *************************************************/
    // this next line is necessary to align numbers
    // correctly with bidi (tested using mlterm)
    if ( (h.gd_day < 10) && (hdate_is_hebrew_locale()) )
       printf ("%d ", h.gd_day);
    else printf ("%2d", h.gd_day);

    /*************************************************
    *  Gregorian date entry - color cleanup
    *************************************************/
    if ((h.hd_jd == opt->jd_today_g) || (opt->colorize) ||
      ( (opt->bold) && ( (h.hd_dw==7) || (holiday_type) ) ) )
      printf (CODE_RESTORE_VIDEO);

    /*************************************************
    *  holiday flag
    *************************************************/
    if (opt->gregorian > 1) printf ("%c", *day_flag);
  }

  /*****************************************************
  *  embedded sub-function: print_hebrew()
  *     print the Hebrew part of the day's entry,
  *****************************************************/
  void print_hebrew()
  {
    /*************************************************
    *  Hebrew date entry - color prefix
    *************************************************/
    if (h.hd_jd == opt->jd_today_h)
        printf (CODE_REVERSE_VIDEO);
    else colorize_prefix();

    /*************************************************
    *  Hebrew date entry - day of the month
    *************************************************/
    // BUGFIX: sunset awareness errs, starts two or three hours early!
    // BUGFIX: in program hdate, sunset awareness not working at all!
    // function check_for_sunset defined in local_functions.c
    hd_day_str = hdate_string(HDATE_STRING_INT, h.hd_day,HDATE_STRING_SHORT,opt->force_hebrew);
    if (opt->bidi)
    {
      hd_day_buffer_str_len = (size_t) strlen(hd_day_str);
      if ( hd_day_buffer_str_len > HEBREW_NUMBER_BUFFER_SIZE)
        hd_day_buffer_str_len = HEBREW_NUMBER_BUFFER_SIZE - 1;
      revstr(hd_day_str, hd_day_buffer_str_len);
    }
    if  ( (opt->force_hebrew) || (hdate_is_hebrew_locale()) )
		{
      // need to pad Hebrew dates 1-10, 20, 30 with spaces
			if (h.hd_day < 11)
			{
				if ((h.hd_dw == 1) && (h.hd_day > 4) && (! opt->mlterm))
  				// This is an unfortunate kludge to compensate for how most
	  			// terminal emulators handle bidi.
					printf("%s",hd_day_str);
				else
					printf ("%s%s"," ",hd_day_str);
				// This is an unfortunate kludge to compensate for how most
				// terminal emulators handle bidi.
				if ((! opt->mlterm) && (h.hd_day == 10))
				  printf(" ");
			}
			else if ( (h.hd_day == 20) || (h.hd_day == 30) )
				printf ("%s%s",hd_day_str," ");
			else printf ("%2s", hd_day_str);
    }
    else printf ("%2s", hd_day_str);

    /*************************************************
    *  Hebrew date entry - color cleanup
    *************************************************/
    if ((h.hd_jd == opt->jd_today_h) || (opt->colorize) ||
      ( (opt->bold) && ( (h.hd_dw==7) || (holiday_type) ) ) )
      printf (CODE_RESTORE_VIDEO);
  }

  /*****************************************************
  ******************************************************
  *  print_day() beginning of actual function
  ******************************************************
  *****************************************************/

  //  out of month - needs padding
  if ( ( (opt->gregorian >  1)  && (h.gd_mon != month) ) ||
       ( (opt->gregorian == 1)  && (h.hd_mon != month) ) )
    printf("     ");
  else if ( (opt->gregorian == 0) && (h.hd_mon != month) )
		printf("  ");
  //  in month - print the data
  else
  {
    get_todays_holiday_data();
    if (opt->gregorian > 1)  print_gregorian_day_and_print_holiday_flag();
    print_hebrew();
    if (opt->gregorian == 1) print_gregorian_day_and_print_holiday_flag();
  }
  if (hd_day_str != NULL) free(hd_day_str);
}



/*************************************************
*  print padding spaces to the left of final line
*************************************************/
void print_last_line_padding ( int jd )
// Calculate padding for trailing days out-of-month. ie. for the final
// calendar line. This is an unfortunate kludge to compensate for how
// most terminal emulators handle bidi. Limit use of this kludge to
// condition option mlterm == FALSE, checked currently by callers to
// this function.
//
// JD :: The Julian day number for the first Hebrew column of the last
//       line, ie. the final Sunday of the Hebrew month
{
	hdate_struct h;
  hdate_set_jd (&h, jd);
	int size_of_month = hdate_get_size_of_hebrew_month( h.hd_mon, h.hd_year_type);
	int padding = 7 - (size_of_month - h.hd_day + 1);
	// This extra space is needed since 'ל' is a single character
  if (size_of_month == 30)
		printf(" ");
	for ( ; padding > 0; padding--)
  {
		printf("   ");
	}
}



/*************************************************
*  print a calendar week's entry (ie. seven columns)
*************************************************/
void print_week( int jd, const int month, option_list* opt, int final_line)
{
  #define long_parasha_name 0
  hdate_struct h;
  int calendar_column;
  // for opt->shabbat and opt->parasha
  hdate_struct yom_shishi;
  // for opt->shabbat
  int sun_hour, first_light, talit, sunrise;
  int midday, sunset, first_stars, three_stars;
  int this_week;
  // for opt->parasha
  int shabbat_name;
  char *shabbat_name_str, *shabbat_name_buffer;
  size_t shabbat_name_str_len;
  // for bidi column alignment
  int print_len;

  /*****************************************************
  *  embedded sub-function: do_calendar_column()
  *****************************************************/
  void do_calendar_column()
  {
    hdate_set_jd (&h, jd);
    if ( ((opt->shabbat) || (opt->parasha)) && (calendar_column == 5) )
      yom_shishi = h;
    if (opt->html) html_print_day ( h, month, opt );
    else print_day ( h, month, opt, FALSE, NULL);
    if (calendar_column != 6)  printf (" ");
  }
  /*****************************************************
  *  END: embedded sub-function: do_calendar_column()
  *****************************************************/

	if ( (final_line == 1) &&
			 (! opt->three_month) &&  // FIXME !!
			 (! opt->mlterm) &&
			 ( (opt->force_hebrew) || (hdate_is_hebrew_locale()) ) )
		print_last_line_padding( jd );

  if (opt->bidi)
  {
    jd=jd+6;
    for (calendar_column = 6; calendar_column >= 0; calendar_column--)
    {
      do_calendar_column();
      jd--;
      if (calendar_column == 6) printf(" ");
    }
    jd=jd+7;
    hdate_set_jd (&h, jd);
  }
  else
  {
    for (calendar_column = 0; calendar_column < 7; calendar_column++)
    {
      do_calendar_column();
      jd++;
    }
  }
  if (!opt->html)
  {
    /********************************************************
    *  determine whether this line gets special highlighting
    ********************************************************/
    if (   ((opt->shabbat) || (opt->parasha))
      && ((jd - 1) > opt->jd_today_h)
      && (((jd - 1) - opt->jd_today_h) < 7) )
        this_week = TRUE;
    else  this_week = FALSE;

    /*************************************************
    *  print shabbat times
    *************************************************/
    if (opt->shabbat)
    {
      // motzay shabat time
      hdate_get_utc_sun_time_full (h.gd_day, h.gd_mon, h.gd_year, opt->lat,
                     opt->lon, &sun_hour, &first_light, &talit,
                     &sunrise, &midday, &sunset,
                     &first_stars, &three_stars);
      // FIXME - allow for further minhag variation
      if (opt->havdalah != 1) three_stars = sunset + opt->havdalah;
      // else three_stars = three_stars;

      // candlelighting times
      hdate_get_utc_sun_time (yom_shishi.gd_day, yom_shishi.gd_mon, yom_shishi.gd_year,
                  opt->lat, opt->lon, &sunrise, &sunset);
      // FIXME - allow for further minhag variation
      if (opt->candles != 1) sunset = sunset - opt->candles;
      else sunset = sunset - DEFAULT_CANDLES_MINUTES;


      // convert havdalah and candlelighting time to epoch time
      // This next snippet is very similar to
      // one in local_functions.c : get_epoch_time_range()
      char * original_system_timezone_string = NULL;
      struct tm tmx;
      time_t sunset_epoch_time;
      time_t three_stars_epoch_time;
      tmx.tm_sec = 0;
      tmx.tm_min = sunset%60;
      tmx.tm_hour = sunset/60;
      tmx.tm_mday = yom_shishi.gd_day;
      tmx.tm_mon =  yom_shishi.gd_mon - 1;
      tmx.tm_year = yom_shishi.gd_year - 1900;
      tmx.tm_wday = 0;
      tmx.tm_yday = 0;
      tmx.tm_isdst = 0;
      original_system_timezone_string = getenv("TZ");
      // per man 3 tzset "If the TZ variable does appear in the environment
      // but its value is empty ... (UTC) is used"
      setenv("TZ", "", 1);
      tzset();
      sunset_epoch_time = mktime(&tmx);
      tmx.tm_sec = 0;
      tmx.tm_min = three_stars%60;
      tmx.tm_hour = three_stars/60;
      tmx.tm_mday = h.gd_day;
      tmx.tm_mon =  h.gd_mon - 1;
      tmx.tm_year = h.gd_year - 1900;
      three_stars_epoch_time  = mktime(&tmx);
      if (original_system_timezone_string)
      {
        setenv("TZ", original_system_timezone_string, 1);
        tzset();
      }
      sunset = sunset + get_tz_adjustment( sunset_epoch_time,
                      opt->tz, &opt->tzif_index,
                      opt->tzif_entries, opt->tzif_data );
      three_stars = three_stars + get_tz_adjustment(three_stars_epoch_time,
                      opt->tz, &opt->tzif_index,
                      opt->tzif_entries, opt->tzif_data );


      // print candleligting time
      if (opt->colorize)
      {
        if (this_week) colorize_element(opt->colorize, ELEMENT_THIS_SHABBAT_TIMES);
        else colorize_element(opt->colorize, ELEMENT_SHABBAT_TIMES);
      }
      else if (this_week) printf(CODE_BOLD_VIDEO);
        printf ("  %02d:%02d", sunset / 60, sunset % 60);
      if ( (opt->colorize) || (this_week) ) printf (CODE_RESTORE_VIDEO);

      printf(" - ");

      // print havdalah time
      if (opt->colorize)
      {
        if (this_week) colorize_element(opt->colorize, ELEMENT_THIS_SHABBAT_TIMES);
        else colorize_element(opt->colorize, ELEMENT_SHABBAT_TIMES);
      }
      else if (this_week) printf(CODE_BOLD_VIDEO);
      printf ("%02d:%02d", three_stars / 60, three_stars % 60);
      if ( (opt->colorize) || (this_week) ) printf (CODE_RESTORE_VIDEO);
    }


    /*************************************************
    *  print shabbat name
    *************************************************/
    if (opt->parasha)
    {

      /*************************************************
      *  print shabbat name - force-hebrew setup
      *************************************************/
      shabbat_name = hdate_get_parasha (&h, opt->diaspora);
      if (shabbat_name) shabbat_name_str =
          hdate_string( HDATE_STRING_PARASHA, shabbat_name,
                  HDATE_STRING_SHORT, opt->force_hebrew | opt->bidi);
      else
      {
        shabbat_name = hdate_get_halachic_day(&h, opt->diaspora);
        if (shabbat_name) shabbat_name_str =
          hdate_string( HDATE_STRING_HOLIDAY,
              shabbat_name,
              HDATE_STRING_SHORT, opt->force_hebrew | opt->bidi);
      }
      if (shabbat_name)
      {
        if (opt->colorize)
        {
          if (this_week) colorize_element(opt->colorize, ELEMENT_THIS_PARASHA);
          else colorize_element(opt->colorize, ELEMENT_PARASHA);
        }
        else if (this_week) printf(CODE_BOLD_VIDEO);

        if (opt->bidi)
        {
          shabbat_name_str_len = strlen(shabbat_name_str);
          shabbat_name_buffer = malloc(shabbat_name_str_len+1);
          memcpy(shabbat_name_buffer, shabbat_name_str, shabbat_name_str_len);
          shabbat_name_buffer[shabbat_name_str_len] = '\0';
          print_len = revstr(shabbat_name_buffer, shabbat_name_str_len);
          #define SHABBAT_MARGIN_MAX 16
          printf("%*s%s", (SHABBAT_MARGIN_MAX - print_len)," ", shabbat_name_buffer);
          free(shabbat_name_buffer);
        }
        else printf("  %s", shabbat_name_str);

        if ( (opt->colorize) || (this_week) ) printf (CODE_RESTORE_VIDEO);
      }
    }
  }
}


/*************************************************************
* how_many_calendar_lines()
*************************************************************/
int how_many_calendar_lines( const int opt_gregorian, const hdate_struct* h)
{
  int number_of_calendar_lines = 4;
  if (opt_gregorian > 1)
  {
    switch (h->gd_mon)
    {
    case  4: case  6: case  9: case 11:
          if (h->hd_dw == 7) number_of_calendar_lines = 6;
          else if (number_of_calendar_lines == 4) number_of_calendar_lines = 5;
          break;
    case  1: case  3: case  5: case  7: case  8: case 10: case 12:
          if (h->hd_dw > 5) number_of_calendar_lines = 6;
          else if (number_of_calendar_lines == 4) number_of_calendar_lines = 5;
          break;
    case 2:    if (h->hd_dw > 1) number_of_calendar_lines = 5;
          break;
    }
  }
  else
  {
    number_of_calendar_lines = 5;
    switch (h->hd_mon)
    {
    case  1: switch (h->hd_year_type)
             { case 2: case 7: case 10: case 14: number_of_calendar_lines = 6; }; break;
    case  2: switch (h->hd_year_type)
             { case 6: case 13:                  number_of_calendar_lines = 6; }; break;
    case  5: switch (h->hd_year_type)
             { case 1: case 7: case 8: case 14:  number_of_calendar_lines = 6; }; break;
    case  7: switch (h->hd_year_type)
             { case 4: case 11: case 12:         number_of_calendar_lines = 6; }; break;
    case 11: switch (h->hd_year_type)
             { case 2: case 6: case 9:           number_of_calendar_lines = 6; }; break;
    case 13: switch (h->hd_year_type)
             { case 10: case 13:                 number_of_calendar_lines = 6; }; break;
    }
  }
  return number_of_calendar_lines;
}



/**************************************************************
*  print month table
**************************************************************/
int print_calendar ( int current_month, int current_year, option_list* opt)
{
  hdate_struct h;
  int calendar_line;
  int max_calendar_lines = 4;
  int prev_month_num_of_lines = 4;
  int next_month_num_of_lines = 4;
  #define BAD_MONTH 999 // for initialization purposes
  int previous_month = BAD_MONTH;
  int next_month = BAD_MONTH;
  int previous_year, next_year;
  int jd_current_month = BAD_MONTH;
  int jd_previous_month = BAD_MONTH;
  int jd_next_month = BAD_MONTH;

  /*********************************************************
  *  Preliminaries:
  *  - Find the first sunday(s) of each calendar
  *  - Find number of calendar lines
  *********************************************************/
  if (opt->gregorian > 1)
  {
    hdate_set_gdate (&h, 1, current_month, current_year);
  }
  else
    hdate_set_hdate (&h, 1, current_month, current_year);
  jd_current_month = h.hd_jd - h.hd_dw + 1;
  max_calendar_lines = how_many_calendar_lines(opt->gregorian, &h);

  /*********************************************************
  *  three months, side-by-side
  *********************************************************/
  if (opt->three_month)
  {
    /*********************************************************
    *  previous month
    *********************************************************/
    if (opt->gregorian < 2)
    {
      if (opt->three_month != 12)
      {
        previous_month = hmonth_previous( current_month, &h );
        previous_year = (current_month==1 ? current_year-1 : current_year);
        hdate_set_hdate (&h, 1, previous_month, previous_year);
        jd_previous_month = h.hd_jd - h.hd_dw + 1;
      }
    }
    else
    {
      if ( opt->three_month == 12)
      {
        if (h.hd_size_of_year >355)
        {
          previous_month = current_month;
          previous_year = current_year;
          jd_previous_month = jd_current_month;
        }
        current_month = (current_month==12 ? 1: current_month+1);
        current_year = (current_month==12 ? current_year+1: current_year);
        hdate_set_gdate (&h, 1, current_month, current_year);
        jd_current_month = h.hd_jd - h.hd_dw + 1;
      }
      else
      {
        previous_month = (current_month==1 ? 12 : current_month-1);
        previous_year  = (current_month==1 ? current_year-1 : current_year);
        hdate_set_gdate (&h, 1, previous_month, previous_year);
        jd_previous_month = h.hd_jd - h.hd_dw + 1;
      }
    }
    prev_month_num_of_lines = how_many_calendar_lines(opt->gregorian, &h);

    /*********************************************************
    *  next month
    *********************************************************/
    if (opt->gregorian < 2)
    {
      if (opt->three_month != 12)
      {
        next_month = hmonth_next( current_month, &h );
        next_year = (next_month==1 ? current_year+1: current_year);
        hdate_set_hdate (&h, 1, next_month, next_year);
      }
    }
    else
    {
      next_month = (current_month==12 ? 1 : current_month+1);
      next_year  = (current_month==12 ? current_year+1: current_year);
      hdate_set_gdate (&h, 1, next_month, next_year);
    }
    jd_next_month = h.hd_jd - h.hd_dw + 1;
    next_month_num_of_lines = how_many_calendar_lines(opt->gregorian, &h);
    max_calendar_lines = ( prev_month_num_of_lines > max_calendar_lines ? prev_month_num_of_lines: max_calendar_lines);
    max_calendar_lines = ( next_month_num_of_lines > max_calendar_lines ? next_month_num_of_lines: max_calendar_lines);
  }

  /**************************************************
  *  maximum six lines of calendar
  **************************************************/

  for (calendar_line = max_calendar_lines; calendar_line > 0; calendar_line--)
  {
    if (opt->html) printf ("<tr>\n");

    if (opt->three_month)
    {
			if ((! opt->mlterm) && (calendar_line == 1))
				// insert here padding for final calendar line of
				// "next_month", ie. third of the three months. This is an
				// unfortunate kludge to compensate for how most terminal
				// emulators handle bidi.
				print_last_line_padding( jd_next_month );
      print_week(jd_previous_month, previous_month, opt, calendar_line);
      jd_previous_month = jd_previous_month + 7;
      printf("%s", opt->border_spacing);
    }

    print_week(jd_current_month, current_month, opt, calendar_line);
    jd_current_month = jd_current_month + 7;


    if (opt->three_month)
    {
      printf("%s", opt->border_spacing);
      if (opt->three_month != 12)
      {
        print_week(jd_next_month, next_month, opt, calendar_line);
        jd_next_month = jd_next_month + 7;
      }
    }

    if (opt->html) printf ("</tr>\n");
    else printf ("\n");
  }


  /**************************************************
  *  print end of calendar
  *************************************************/
  if (opt->html) printf ("</table>\n</span>");
  return 0;
}


/****************************************************
* print footnote
****************************************************/
void print_footnote( const hdate_struct h, const int footnote_month,
        option_list* opt, const char* text_ptr, char* custom_day_flag )
{
  // all these for bidi alignment
  char *bidi_buffer;
  size_t text_ptr_len;
  int print_len;

  // BEGIN: embedded function
  void colorize_footnote_text()
  {
    if (opt->colorize)
    {
      if (opt->jd_today_h == h.hd_jd)
        colorize_element(opt->colorize, ELEMENT_TODAY_HOLIDAY_NAME);
      else
        colorize_element(opt->colorize, ELEMENT_HOLIDAY_NAME);
    }
    else if ( (opt->bold) && (opt->jd_today_h == h.hd_jd) )
      printf(CODE_BOLD_VIDEO);
  }
  // END: embedded function

  if (opt->bidi)
  {
    text_ptr_len = strlen(text_ptr);
    bidi_buffer = malloc(text_ptr_len+1);
    memcpy(bidi_buffer, text_ptr, text_ptr_len);
    bidi_buffer[text_ptr_len] = '\0';
    print_len = revstr(bidi_buffer, text_ptr_len);
    #define FOOTNOTE_MARGIN_MAX 20
    colorize_footnote_text();
    printf("%*s%s  ", (FOOTNOTE_MARGIN_MAX - print_len)," ", bidi_buffer);
    if ( (opt->colorize) ||
       ( (opt->bold) && (opt->jd_today_h == h.hd_jd) ) )
      printf (CODE_RESTORE_VIDEO);
    free(bidi_buffer);
  }

  // print_day handles its own colorization
  print_day ( h, footnote_month, opt, TRUE, custom_day_flag);

  if (opt->bidi) printf ("\n");
  else
  {
    colorize_footnote_text();
    printf ("  %s\n", text_ptr);
    if ( (opt->colorize) ||
       ( (opt->bold) && (opt->jd_today_h == h.hd_jd) ) )
      printf (CODE_RESTORE_VIDEO);
  }
}


/****************************************************
* print month header, month table, month footnotes
****************************************************/
int print_month ( const int month, const int year, option_list* opt)
{

  hdate_struct h;
  hdate_struct h_final_day;  // misnomer: It really is the day AFTER the
                // the final day of this calendar request.
                // We use it to retrieve the epoch time at
                // midnight of the requested timezone, and
                // then subtract one second, thus
                // epochtime(h) to epochtime(h_final_day)-1
                // is the interval to check for daylight
                // savings time transitions.
  char calendar_type;           // for custom days
  int jd_counter, holiday, footnote_month;// for opt->footnote
  char* tz_name_input;           // for timezone parsing
  // check if hebrew year
  if (year > HDATE_HEB_YR_LOWER_BOUND)
  {
    hdate_set_hdate (&h, 1, month, year);
    calendar_type = 'H';
  }
  else
  {
    hdate_set_gdate (&h, 1, month, year);
    calendar_type = 'G';
  }

  // custom days
  // first check that this snippet hasn't been performed before:
  //    ie. this is not the first month of a year's output
  // also: should we do this even when not requested options
  //    --color, --footnote (and maybe others)?
  if (opt->custom_days_cnt == BAD_CUSTOM_DAY_CNT)
  {
    // TODO - 'three-month mode custom days...
    //     at this point only for color mode highlighting because
    //     we don't allow foornotes for three month mode
    FILE *custom_file = NULL;
    if (get_custom_days_file( "/hcal", "/custom_days_v1.8",
                  opt->tz_name_str, opt->quiet_alerts,
                  &custom_file))
    {
      opt->custom_days_cnt = read_custom_days_file(custom_file,
                  &opt->jdn_list_ptr, &opt->string_list_ptr,
                  0, month, year,
                  calendar_type, h,
                  HDATE_STRING_LONG, opt->force_hebrew);
      fclose(custom_file);
    }
  }

  // get dst transition information, if necessary
  if ( (opt->tzif_data == NULL) &&
     ( (opt->shabbat) || (opt->candles) || (opt->havdalah) ) )
  {
    if (calendar_type == 'H')
    {
      if (opt->one_year) hdate_set_hdate (&h_final_day, 1, 1, year+1);
      // POSSIBLE BUGS - interpreting bumping to month 13,14,15
      else if (opt->three_month) hdate_set_hdate (&h_final_day, 1, month+3, year);
      else hdate_set_hdate (&h_final_day, 1, month+1, year);
    }
    else // calendar_type = 'G'
    {
      if (opt->one_year) hdate_set_gdate (&h_final_day, 1, 1, year+1);
      // POSSIBLE BUGS - interpreting bumping to month 13,14,15
      else if (opt->three_month) hdate_set_gdate (&h_final_day, 1, month+3, year);
      else hdate_set_gdate (&h_final_day, 1, month+1, year);
    }
    get_epoch_time_range( &opt->epoch_start, &opt->epoch_end,
          opt->tz_name_str, opt->tz,
          h.gd_year, h.gd_mon, h.gd_day,
          h_final_day.gd_year, h_final_day.gd_mon, h_final_day.gd_day);
    tz_name_input = opt->tz_name_str;
    process_location_parms(  &opt->lat, &opt->lon, opt->tz_lon,
                &opt->tz, tz_name_input, &opt->tz_name_str,
                opt->epoch_start, opt->epoch_end,
                &opt->tzif_entries, &opt->tzif_data,
                opt->quiet_alerts);
    // remember to free() tz_name_str
  }


  if (opt->gregorian >1)
  {
    print_header (h.gd_mon, h.gd_year, opt);
    print_calendar (h.gd_mon, h.gd_year, opt);
  }
  else
  {
    print_header (h.hd_mon, h.hd_year, opt);
    print_calendar (h.hd_mon, h.hd_year, opt);
  }
  if (opt->separator == NULL) printf("\n");


  /****************************************************
  * print footnotes
  ****************************************************/
  if (opt->footnote)
  {
    jd_counter = h.hd_jd;
    if (opt->gregorian < 2) footnote_month = h.hd_mon;
    else footnote_month = h.gd_mon;  // bugfix: was = month -  but CHECK THIS for accuracy !
    while ( ( (opt->gregorian > 1) && (footnote_month == h.gd_mon ) ) ||
        ( (opt->gregorian < 2) && (footnote_month == h.hd_mon ) )  )
    {
      holiday = hdate_get_halachic_day(&h, opt->diaspora);
      if (holiday)
      {
        print_footnote( h, footnote_month, opt,
                hdate_string( HDATE_STRING_HOLIDAY,
                        holiday,
                        HDATE_STRING_LONG,
                        opt->force_hebrew),
                NULL);
      }
      if (opt->custom_days_cnt)
      {
        //opt->custom_days_index = 0;
        //int* jdn_list_ptr = opt->jdn_list_ptr;
        //while (opt->custom_days_index < opt->custom_days_cnt)
        //{
          //if (h.hd_jd == *jdn_list_ptr)
          //{
            //print_footnote( h, footnote_month, opt,
                    //get_custom_day_text_ptr(opt->custom_days_index, opt->string_list_ptr));
          //}
          //opt->custom_days_index = opt->custom_days_index + 1;
          //jdn_list_ptr = jdn_list_ptr + 1;
        //}
        int* jdn_list_ptr;
        for (  opt->custom_days_index = 0,
            jdn_list_ptr = opt->jdn_list_ptr
            ;
            (opt->custom_days_index < opt->custom_days_cnt)
            ;
            opt->custom_days_index = opt->custom_days_index + 1,
            jdn_list_ptr = jdn_list_ptr + 1
          )
        {
          if (h.hd_jd == *jdn_list_ptr)
          {
            char day_flag = * get_custom_day_symbol_ptr(opt->custom_days_index, opt->string_list_ptr);
            print_footnote( h, footnote_month, opt,
                    get_custom_day_text_ptr(opt->custom_days_index, opt->string_list_ptr), &day_flag);

          }
        }
      }
      jd_counter++;
      hdate_set_jd (&h, jd_counter);
    }
  }
  return 0;
}


/****************************************************
* read and parse config file
****************************************************/
void read_config_file(  FILE *config_file,
            option_list *opt,
            double*  latitude,
            double*  longitude,
            int*  tz,
            char*  tz_name_str )

{
  double tz_lat;
  char  *input_string = NULL;
  size_t  input_str_len;  // unnecessary to initialize, per man(3) getline
//  size_t  input_str_len = 200;  // WARNING: if you change this value
                  // you will still have to also
                  // change a matching value below
                  // in the statement that includes:
                  // match_count = sscanf(input_string
  char  *input_key;    // unnecessary to initialize, per man(3) sscanf
//  char  *input_key = NULL;
  char  *input_value;  // unnecessary to initialize, per man(3) sscanf
//  char  *input_value = NULL;
  int    line_count = 0;
  int    menu_item = 0;
  size_t  menu_len = 0;
  int    match_count;
  int    end_of_input_file = FALSE;
  int    key_index = 0;
  int    temp_base_year = 0;
  const int  num_of_keys = 20;
  const char*  key_list[] = {  "SUNSET_AWARE",    // 0
                "LATITUDE",
                "LONGITUDE",    // 2
                "TIMEZONE",
                "DIASPORA",      // 4
                "FORCE_ISRAEL",
                "PARASHA_NAMES",  // 6
                "SHABBAT_INFO",
                "FOOTNOTE",    // 8
                "FORCE_HEBREW",
                "OUTPUT_BIDI",    //10
                "SUPPRESS_REVERSE_VIDEO",
                "COLORIZE",      //12
                "OUTPUT_HTML",
                "USE_EXTERNAL_CSS_FILE",
                "QUIET_ALERTS",
                "THREE_MONTH",    //16
                "MENU",
                "CANDLE_LIGHTING",  //18
                "HAVDALAH",
                "PREFER_HEBREW",  //20
                "BASE_YEAR_HEBREW",
                "BASE_YEAR_GREGORIAN"//22
                };
//  TODO - parse these!
//  opt.prefer_hebrew = TRUE;
//  opt.base_year_h = 5700;    // TODO - Make this user-selectable
//  opt.base_year_g = 2000;    // TODO - Make this user-selectable

  while ( end_of_input_file!=TRUE )
  {
    end_of_input_file = getline(&input_string, &input_str_len, config_file);
    if ( end_of_input_file!=TRUE )
    {
      errno = 0;
      match_count = sscanf(input_string,"%m[A-Z_]=%m[^\n]",&input_key,&input_value);
      line_count++;
      if (errno != 0) error(0,errno,"scan error at line %d", line_count);
// DEBUG -   printf("line number = %d, matches made = %d, key = %s, value = %s, string = %s",
//          line_count, match_count, input_key, input_value, input_string);
      if (match_count == 2)
      {
        for (key_index=0; key_index<num_of_keys; key_index++)
        {
          if (strcmp(input_key, key_list[key_index]) == 0)
          {
            switch(key_index)
            {

    case  0:if      (strcmp(input_value,"FALSE") == 0) opt->not_sunset_aware = 1;
        else if (strcmp(input_value,"TRUE") == 0) opt->not_sunset_aware = 0;
        break;
    case  1:
        parse_coordinate(1, input_value, latitude);
        break;
    case  2:
        parse_coordinate(2, input_value, longitude);
        break;
    case  3:
        if  (!parse_timezone_numeric(input_value, tz))
        {
          if (parse_timezone_alpha(input_value, &tz_name_str, tz, &tz_lat, &opt->tz_lon))
          {
            // TODO - really, at this point, shouldn't either both be bad or botha be good?
            if (*latitude  == BAD_COORDINATE) *latitude = tz_lat;
            if (*longitude == BAD_COORDINATE) *longitude = opt->tz_lon;
          }
        }
        break;
    case  4:if      (strcmp(input_value,"FALSE") == 0) opt->diaspora = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->diaspora = 1;
        break;
    case  5:if      (strcmp(input_value,"FALSE") == 0) opt->force_israel = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->force_israel = 1;
        break;
    case  6:if      (strcmp(input_value,"FALSE") == 0) opt->parasha = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->parasha = 1;
        break;
    case  7:if      (strcmp(input_value,"FALSE") == 0) opt->shabbat = 0;
        else if (strcmp(input_value,"TRUE") == 0)
        {
          opt->shabbat = 1;
          opt->parasha = 1;
        }
        break;
    case  8:if      (strcmp(input_value,"FALSE") == 0) opt->footnote = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->footnote = 1;
        break;
    case  9:if      (strcmp(input_value,"FALSE") == 0) opt->force_hebrew = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->force_hebrew = 1;
        break;
    case 10:if      (strcmp(input_value,"FALSE") == 0) opt->bidi = 0;
        else if (strcmp(input_value,"TRUE") == 0)
            {
              opt->bidi = 1;
              opt->force_hebrew = 1;
            }
        break;
    case 11:if      (strcmp(input_value,"FALSE") == 0) opt->no_reverse = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->no_reverse = 1;
        break;
    case 12:if      (strcmp(input_value,"FALSE") == 0) opt->colorize = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->colorize = 1;
        break;
    case 13:if      (strcmp(input_value,"FALSE") == 0) opt->html = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->html = 1;
        break;
    case 14:if      (strcmp(input_value,"FALSE") == 0) opt->external_css = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->external_css = 1;
        break;
    case 15:if      (strcmp(input_value,"FALSE") == 0) opt->quiet_alerts = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->quiet_alerts = 1;
        break;
    case 16:if      (strcmp(input_value,"FALSE") == 0) opt->three_month = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->three_month = 1;
        break;

//    MENU
    case 17:if (menu_item < MAX_MENU_ITEMS)
        {
          menu_len = strlen(input_value);
          opt->menu_item[menu_item] = malloc(menu_len+1);
          memcpy(opt->menu_item[menu_item], input_value,menu_len);
          menu_item++;
        }
        break;
//    CANDLE_LIGHTING
    case 18:if      (strcmp(input_value,"FALSE") == 0) opt->candles = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->candles = 1;
        else if (fnmatch( "[[:digit:]]?([[:digit:]])", input_value, FNM_EXTMATCH) == 0)
        {
          opt->candles = atoi(input_value);
          if (opt->candles < MIN_CANDLES_MINUTES) opt->candles = MIN_CANDLES_MINUTES;
          else if (opt->candles > MAX_CANDLES_MINUTES) opt->candles = MAX_CANDLES_MINUTES;
        }
        break;

//    HAVDALAH
    case 19:if      (strcmp(input_value,"FALSE") == 0) opt->havdalah = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->havdalah = 1;
        else if (fnmatch( "[[:digit:]]?([[:digit:]])", input_value, FNM_EXTMATCH) == 0)
        {
          opt->havdalah = atoi(input_value);
          if (opt->havdalah < MIN_MOTZASH_MINUTES) opt->havdalah = MIN_MOTZASH_MINUTES;
          else if (opt->havdalah > MAX_MOTZASH_MINUTES) opt->havdalah = MAX_MOTZASH_MINUTES;
        }
        break;
//    PREFER_HEBREW
    case 20:if      (strcmp(input_value,"FALSE") == 0) opt->prefer_hebrew = 0;
        else if (strcmp(input_value,"TRUE") == 0) opt->prefer_hebrew = 1;
        break;
//    BASE_YEAR_HEBREW
    case 21:if (fnmatch( "[3456][[:digit:]]", input_value, FNM_EXTMATCH) == 0)
        {
          temp_base_year = atoi(input_value);
          if ( (temp_base_year < (HDATE_HEB_YR_LOWER_BOUND/100)) ||
             (temp_base_year > (HDATE_HEB_YR_UPPER_BOUND/100)) )
             opt->base_year_h = HDATE_DEFAULT_BASE_YEAR_H;
          else opt->base_year_h = temp_base_year * 100;
        }
        break;
//    BASE_YEAR_GREGORIAN
    case 22:if (fnmatch( "[12][[:digit:]]", input_value, FNM_EXTMATCH) == 0)
        {
          temp_base_year = atoi(input_value);
          if ( (temp_base_year < (HDATE_HEB_YR_LOWER_BOUND/100)) ||
             (temp_base_year > (HDATE_HEB_YR_UPPER_BOUND/100)) )
             opt->base_year_h = HDATE_DEFAULT_BASE_YEAR_H;
          else opt->base_year_h = temp_base_year * 100;
        }
        break;

            }  // end of switch(i)
          break;  // if found a match don't continue for loop
          }
        }
      free(input_value);
      }
      if (match_count > 0 ) free(input_key);
    }
  }
  if (input_string != NULL ) free(input_string);
  return;
}


/****************************************************
* exit elegantly
****************************************************/
// TODO - This app needs to be checked thoroughly for free()ing
void exit_main( option_list *opt, const int exit_code)
{
  int i;
  for (i=0; i<MAX_MENU_ITEMS; i++)
  {
    if (opt->menu_item[i] == NULL) break;
    free(opt->menu_item[i]);
  }
  if (opt->jdn_list_ptr != NULL) free(opt->jdn_list_ptr);
  if (opt->string_list_ptr != NULL) free(opt->string_list_ptr);
  exit (exit_code);
}


/****************************************************
* parse a command-line or a config-file menu line
*
* It was appropriate to make this a function, outside
* of main, because of its dual use and dual reference
****************************************************/
// above is vague. I think I meant, because it is called
// for parsing both the command line and the menu options'
// command lines
int hcal_parser( const int switch_arg, option_list *opt,
        double *lat, double *lon, int *tz, char* tz_name_str,
        int long_option_index)

{
  double tz_lat;
  static char *timezone_text  = N_("z (timezone)");

  int error_detected = 0;

  switch (switch_arg)
  {

  case 0: // long options
    switch (long_option_index)
    {
/** --version  */  case 0:  print_version (); exit_main(opt, 0); break;
/** --help    */  case 1:  print_help (); exit_main(opt, 0); break;
/** --no-reverse*/  case 2:  opt->no_reverse = 1; break;
/** --html    */  case 3: break;
/** --parasha  */  case 4: break;
/** --shabbat  */  case 5:  break;
/** --three-month*/  case 6: break;
/** --colorize  */  case 7: break;
/** --footnote  */  case 8: break;
/** --hebrew  */  case 9: break;
/** --israel  */  case 10:break;
/** --latitude  */  case 11:break;
/** --longitude  */  case 12:break;
/** --timezone  */  case 13:break;
/** --not-sunset-aware */  case 14: opt->not_sunset_aware = 1;  break;
/** --quiet-alerts*/  case 15: break;
/** --visual  */  case 16:
/** --bidi    */  case 17: break;
/** --one-month  */  case 18: break;
/** --no-visual  */  case 19:
/** --no-bidi  */  case 20: opt->bidi = 0; opt->force_hebrew = 0; break;
/** --no-color  */  case 21: opt->colorize = 0; opt->bold = 0; break;
/** --no-footnote*/  case 22: opt->footnote = 0; break;
/** --menu    */  case 23: break;
/** --candles  */  case 24:
      if ( (optarg == NULL) && (opt->candles == 0) ) opt->candles = 1;
      else
      {
        // BUG - Does not support numbers with leading zeros,
        //       which could be valid in range
        //       (eg. 0022 for 22 minutes
        if (fnmatch( "[[:digit:]]?([[:digit:]])", optarg, FNM_EXTMATCH) == 0)
          opt->candles = atoi(optarg);
        // explanation: if the parse fails, then opt->candles must be
        // zero, which will be out of bounds
        if   ( (opt->candles < MIN_CANDLES_MINUTES) ||
            (opt->candles > MAX_CANDLES_MINUTES) )
        {
          print_parm_error("--candles");
          error_detected++;
        }
      }
      break;
/** --havdalah */  case 25:
      if ( (optarg == NULL) && (opt->havdalah == 0) ) opt->havdalah = 1;
      else
      {
        // BUG - Does not support numbers with leading zeros,
        //       which could be valid in range
        //       (eg. 0022 for 22 minutes
        if (fnmatch( "[[:digit:]]?([[:digit:]])", optarg, FNM_EXTMATCH) == 0)
          opt->havdalah = atoi(optarg);
        // explanation: if the parse fails, then opt->candles must be
        // zero, which will be out of bounds
        if   ( (opt->havdalah < MIN_MOTZASH_MINUTES) ||
            (opt->havdalah > MAX_MOTZASH_MINUTES) )
        {
          print_parm_error("--havdalah");
          error_detected++;
        }
      }
      break;
/** --separator */      case 26:
			opt->separator = optarg;
			// BB 2022-01-16: I'm deciding to remove this feature as not
			//                visually appealing and just more weird bidi
			//                work to do.
			error_detected++;
			print_option_unknown_error("separator");
			break;
/** --gregorian */      case 27: break;
/** --no-gregorian */    case 28: break;
/** --borders      */    case 29: opt->three_month = 1;
                opt->border_spacing = default_borders_spacing;
                opt->separator = default_borders_separator;
                break;
/** --prefer-hebrew    */  case 30: opt->prefer_hebrew = TRUE; break;
/** --prefer-gregorian  */  case 31: opt->prefer_hebrew = FALSE; break;
/** --bold              */  case 32: break;
/** --usage             */  case 33: break;
/** --mlterm           */   case 34: opt->mlterm = TRUE; break;
    } // end switch for long_options
    break;


  case '0': opt->gregorian = 0; break;
  case '1': opt->three_month = 0; break;
  case '3': opt->three_month = 1; break;
  case 'b': opt->bidi = 1; opt->force_hebrew = 1; break;
  case 'B': opt->bold = 1; opt->colorize = 0; break;
  case 'c':
    if (opt->colorize < 2) opt->colorize = opt->colorize + 1;
    opt->bold = 0;
    break;
  case 'd': opt->diaspora = 1; break;
  case 'f': opt->footnote = 1; break;
  case 'g':
    if (opt->gregorian < 2) opt->gregorian = opt->gregorian + 1;
    break;
  case 'h': opt->html = 1;
    // TODO - give this parameter an option argument 'filename'
    //        to use for output instead of stdout
    break;
  case 'H': opt->force_hebrew = 1; break;
  case 'I': opt->force_israel = 1; break;
  case 'i': opt->external_css = 1; break;
  case 'm': opt->menu = 1; break;
  case 'p': opt->parasha = 1; break;
  case 'q': opt->quiet_alerts = 1; break;
  case 'l':
    error_detected = error_detected + parse_coordinate(1, optarg, lat);
    break;
  case 'L':
    error_detected = error_detected + parse_coordinate(2, optarg, lon);
    break;
  case 's':
    opt->shabbat = 1;
    opt->parasha = 1;
    if (opt->candles == 0) opt->candles = 1;
    if (opt->havdalah == 0) opt->havdalah = 1;
    break;
  case 'z':
    if  ((!optarg)
      || ( (optarg[0] == '-')
           && ( (optarg[1] == '0')
                || (strspn(&optarg[1], "0123456789.:") == 0) ) ) )
    {
       error_detected = error_detected + 1;
      print_parm_missing_error(timezone_text);
    }
    else if (!parse_timezone_numeric(optarg, tz))
    {
      if (!parse_timezone_alpha(optarg, &tz_name_str, tz, &tz_lat, &opt->tz_lon))
      {
        error_detected = error_detected + 1;
        print_parm_error(timezone_text);
      }
      else
      {
        opt->tz_name_str = tz_name_str;
        if (*lat  == BAD_COORDINATE) *lat = tz_lat;
        if (*lon == BAD_COORDINATE) *lon = opt->tz_lon;
      }
    }
    break;
  case '?':
    if (( optopt != '?') && (long_option_index != 33) ) print_option_unknown_error ( (char*) &optopt );
  default:
    print_usage_hcal();
    print_try_help_hcal();
    exit_main(opt, 0);
    break;
  }
  return error_detected;
}

/**************************************************
***************************************************
***************************************************
* main
***************************************************
***************************************************
**************************************************/
int main (int argc, char *argv[])
{
  hdate_struct h;
  int error_detected = FALSE;  // exit after reporting ALL bad parms
  int data_sink;        // store unwanted stuff here
  int month, year;
  const int num_of_months = 12;  // how many months in the year
  option_list opt;
  opt.prefer_hebrew = TRUE;
  opt.base_year_h = HDATE_DEFAULT_BASE_YEAR_H;    // TODO - Make this user-selectable
  opt.base_year_g = HDATE_DEFAULT_BASE_YEAR_G;    // TODO - Make this user-selectable
  opt.gregorian = 0;      // -0 don't display any gregorian information
  opt.bidi = 0;        // visual bidi, implies --force-hebrew
  opt.html = 0;        // -h html format flag
  opt.diaspora = 0;      // -d Diaspora holidays
  opt.external_css = 0;    // -i External css file
  opt.parasha = 0;      // -p print parasha alongside calendar
  opt.shabbat = 0;      // -c print candle-lighting alongside calendar
  opt.candles = 0;
  opt.havdalah = 0;
  opt.no_reverse = 0;      // don't highlight today in reverse video
  opt.three_month = 0;    // print previous and next months also
  opt.one_year = 0;
  opt.border_spacing = NULL;      // horizontal spacing string in 3-month mode
  opt.separator = NULL;    // vertical separator string in 3-month mode
  opt.colorize = 0;      // display calendar in muted, more pleasing tones
  opt.bold = 0;        // display special days in boldface (no color)
  opt.footnote = 0;      // display description of month's holidays
  opt.force_hebrew = 0;    // force display of Hebrew data in Hebrew
  opt.force_israel = 0;    // override diaspora-awareness
  opt.not_sunset_aware = 0;  // override sunset-awareness
  opt.quiet_alerts = 0;
	opt.mlterm = FALSE;        // use bidi kludges to attempt to display things nicely

  opt.lat = BAD_COORDINATE;
  opt.lon = BAD_COORDINATE;
  // explain why the duplication of these next variables
  double lat = BAD_COORDINATE;  // set to this value for error handling
  double lon = BAD_COORDINATE;  // set to this value for error handling
  int tz = BAD_TIMEZONE;
  /******************************************************************
  *
  * TIME ZONE POLICY EXPLAINED:
  *
  * -z number (absolute over-ride of system time zone)
    *      longitude mis-sync -> just alert
    *      no longitude -> guess (including zone.tab for current)
    *      alert no dst awareness
  * -z name   (absolute over-ride of system time zone)
    *      longitude mis-sync -> just alert
    *      latitude mis-sync (re:zone.tab) - just alert
    *      no longitude -> use zone.tab entry
  * -z null   system time zone (/etc/localtime)
    *      longitude mis-sync -> just alert
    *      latitude mis-sync (re:zone.tab) - just alert
    *      no longitude -> use zone.tab entry
    *
  *****************************************************************/
  opt.tz = BAD_TIMEZONE;
  opt.tz_name_str = NULL;
  opt.tz_lon = BAD_COORDINATE;
  /******************************************************************
  *
  * CUSTOM DAYS DATA STRUCTUES EXPLAINED:
  *
  * custom_days_cnt - number of custom days found for interval,
  *        and the number of entries in each of the associated
  *        data structures.
  * custom_days_index - presuming custom days are sorted in
  *        ascending date order, this is useful for resuming a
  *        a scan/search of custom days. However, currently
  *        sorting has not been coded.
    * jdn_list_ptr - a malloc'ed list of {custom_days_cnt} julian
    *        day numbers of the custom days found for interval.
    * string_list_ptr - a malloc'ed list of {custom_days_cnt}
    *        uniform lenth strings describing the custom days.
    *        The first {sizeof(size_t)} bytes is the size of each
    *        element in this array
    *
  *****************************************************************/
  opt.custom_days_cnt = BAD_CUSTOM_DAY_CNT;
  opt.custom_days_index = 0;
  opt.jdn_list_ptr = NULL;
  opt.string_list_ptr = NULL;
  /******************************************************************
  *
  * DAYLIGHT SAVINGS TIME DATA STRUCTUES EXPLAINED:
  *
  * These elements are necessary for displaying Shabbat times,
  * and for sunset-awareness.
  *
  * tzif_entries - number of DST transitions found for the interval
  *        and the number of entries in each of the associated
  *        data structures.
  * tzif_index - DST transition information will always be sorted
  *        in ascending date order, so this is useful for resuming
  *        a scan/search
  * tzif_data -
  * epoch_start - date, in epoch format for the beginning of the
  *        interval
  * epoch_end - date, in epoch format for the end of the interval.
    *
  *****************************************************************/
  opt.tzif_entries = 0;
  opt.tzif_index = 0;
  opt.tzif_data = NULL;
  opt.epoch_start = 0;
  opt.epoch_end = 0;
  // -m print menus for user-selection
  opt.menu = 0;
  int i;
  for (i=0; i<MAX_MENU_ITEMS; i++) opt.menu_item[i] = NULL;
  size_t  menu_len = 0;
  int menu_index;
  char *menuptr, *optptr;
  // support for getopt short options
  const char * short_options = "013bBcdfghHiImpqsl:L:z:";
  // support for long options
  int long_option_index = 0;
  int switch_arg;
  const struct option long_options[] = {
  //   name,  has_arg, flag, val
    {"version", no_argument, 0, 0},
    {"help", no_argument, 0, 0},
    {"no-reverse", no_argument, 0, 0},
    {"html", no_argument, 0, 'h'},
    {"parasha", no_argument, 0, 'p'},
    {"shabbat", no_argument, 0, 's'},
    {"three-month", no_argument, 0, '3'},
    {"colorize", no_argument, 0, 'c'},
    {"footnote", no_argument,0,'f'},
    {"hebrew", no_argument, 0,'H'},
    {"israel", no_argument, 0,'I'},
    {"latitude", required_argument, 0, 'l'},
    {"longitude", required_argument, 0, 'L'},
    {"timezone", required_argument, 0, 'z'},
    {"not-sunset-aware", no_argument, 0, 0},
    {"quiet-alerts", no_argument, 0,'q'},
    {"bidi", no_argument, 0,'b'},
    {"visual", no_argument, 0,'b'},
    {"one-month", no_argument, 0,'1'},
    {"no-bidi", no_argument, 0, 0},
    {"no-visual", no_argument, 0, 0},
    {"no-color", no_argument, 0, 0},
    {"no-footnote", no_argument, 0, 0},
    {"menu", no_argument, 0,'m'},
    {"candles", optional_argument, 0, 0},
    {"havdalah", optional_argument, 0, 0},
    {"separator", optional_argument, 0, 0},
    {"gregorian", no_argument, 0,'g'},
    {"no-gregorian", no_argument, 0,'0'},
    {"borders", no_argument, 0, 0},
    {"prefer-hebrew", no_argument, 0, 0},
    {"prefer-gregorian", no_argument, 0, 0},
    {"bold", no_argument, 0, 'B'},
    {"usage", no_argument, 0, '?'},
    {"mlterm", no_argument, 0, 0},
    {0, 0, 0, 0}
    };

  /************************************************************
  * init locale
  *
  * I'm not sure why this had to be done. Per an answer on
  * stackoverflow (not yet check out by me):
  *
  * "At program startup, you should call setlocale():
  *      setlocale(LC_CTYPE, "");
  * This will cause the wide character functions to use the
  * appropriate character set defined by the environment - eg.
  * on Unix-like systems, the LANG environment variable. For
  * example, this means that if your LANG variable is set to a
  * UTF8 locale, the wide character functions will handle input
  * and output in UTF8. (This is how the POSIX wc utility is
  * specified to work). You can then use the wide-character
  * versions of all the standard functions. However, if you
  * really do want to count characters rather than bytes, and
  * can assume that your text files are encoded in UTF-8, then
  * the easiest approach is to count all bytes that are not
  * trail bytes (i.e., in the range 0x80 to 0xBF)."
  *
  * Note: the code sets LC_ALL instead of LC_CTYPE; and the
  * code does not use wide_char functions ...
  ************************************************************/
  setlocale (LC_ALL, "");

  /************************************************************
  * parse config file
  ************************************************************/
  FILE *config_file = NULL;
  if (get_config_file("/hcal", "/hcalrc_v1.8", hcal_config_file_text,
            opt.quiet_alerts, &config_file))
  {
    read_config_file(config_file, &opt, &lat, &lon, &tz, opt.tz_name_str);
    fclose(config_file);
  }

  /************************************************************
  * parse command line
  ************************************************************/
   while ((switch_arg = getopt_long(argc, argv,
              short_options, long_options,
              &long_option_index)) != -1)
    error_detected = error_detected
            + hcal_parser(switch_arg, &opt, &lat, &lon,
                  &tz, opt.tz_name_str, long_option_index);

  /**************************************************
  * BEGIN - enable user-defined menu
  *************************************************/
  if (opt.menu)
  {
    i = menu_select( &opt.menu_item[0], MAX_MENU_ITEMS );
    if (i == -1) exit_main(&opt, 0);
    else if ((i < MAX_MENU_ITEMS) && (opt.menu_item[i] != NULL))
    {

      /**************************************************
      * parse user's menu selection
      *************************************************/
      menuptr = opt.menu_item[i];
      menu_len = strlen( menuptr );
      menu_index = 0;
      optptr = NULL;
      optarg = NULL;

      while (( switch_arg = menu_item_parse( menuptr, menu_len, &menu_index,
                &optptr, short_options, long_options,
                &long_option_index, &error_detected) ) != -1)
      {
        error_detected = error_detected +
          hcal_parser(switch_arg, &opt, &lat, &lon,
                &tz, opt.tz_name_str, long_option_index);
      }
    }
  }
  /**************************************************
  * END   - enable user-defined menu
  *************************************************/


  /**************************************************
  * three month mode checks
  *************************************************/
  if  (opt.three_month)
  {
    if ((opt.parasha) || (opt.shabbat) || (opt.footnote) )
    {
      error(0,0,"%s", N_("ALERT: options --parasha, --shabbat, --footnote are not supported in 'three-month' mode"));
      opt.parasha = 0;
      opt.shabbat = 0;
      opt.footnote = 0;
    }
    if (opt.border_spacing == NULL) opt.border_spacing = default_spacing;
  }

  /**************************************************
  * html month mode checks
  *************************************************/
  if  (opt.html)
  {
    if ((opt.parasha) || (opt.shabbat) || (opt.footnote) )
    {
      error(0,0,"%s", N_("ALERT: options --parasha, --shabbat, --footnote are not supported in 'html' mode"));
      opt.parasha = 0;
      opt.shabbat = 0;
      opt.footnote = 0;
    }
  }

  // MISSING - validation of lat lon tz !!

  // exit after reporting all bad parameters found */
  if (error_detected)
  {
    print_usage_hcal();
    print_try_help_hcal();
    exit(EXIT_CODE_BAD_PARMS);
  }

  opt.lat = lat;
  opt.lon = lon;
  opt.tz = tz;

  /************************************************************
  * determine what day to highlight (ie. what is today)
  ************************************************************/
  if ((opt.no_reverse) || (opt.html))
  {
    // user doesn't want any highlighting
    opt.jd_today_g = 0;
    opt.jd_today_h = 0;
  }
  else
  {
    hdate_set_gdate (&h, 0, 0, 0);
    opt.jd_today_g = h.hd_jd;
    if  ((!opt.not_sunset_aware) &&
       (check_for_sunset(&h, lat, lon, tz)) )
          opt.jd_today_h = h.hd_jd + 1;
    else     opt.jd_today_h = h.hd_jd;
  }



/**************************************************
* parse and vaildate date parameters
**************************************************/
  /**/
  /**************************************************
  * no date parameter provided - use current mm yyyy
  * and no need to validate parameters
  **************************************************/
  if (argc == (optind))
  {
    if ((opt.not_sunset_aware) || (opt.no_reverse) || (opt.html))
      // call function only if not already called
      // for sunset awareness, above.
      hdate_set_gdate (&h, 0, 0, 0);  // today
    if (opt.gregorian > 1)
    {
      month = h.gd_mon;
      year = h.gd_year;
    }
    else
    {
      month = h.hd_mon;
      year = h.hd_year;
    }
  }
  else
  {
    /********************************************************
    * additional parameters provided - get them and validate
    ********************************************************/
    if (argc == (optind + 2))
    {
      if (!hdate_parse_date( argv[optind], argv[optind+1],
               "", &year, &month, &data_sink, 2,
               opt.prefer_hebrew, HDATE_PREFER_YM,
               opt.base_year_h, opt.base_year_g))
        exit_main(&opt,0);
      // The parse_date function returns Hebrew
      // month values in the range 101 - 114
      month = month%100;
    }
    else if (argc == (optind + 1))
    {
      month = 0;
      if (!hdate_parse_date( argv[optind], "", "",
               &year, &month, &data_sink, 1,
               opt.prefer_hebrew, HDATE_PREFER_YM,
               opt.base_year_h, opt.base_year_g))
        exit_main(&opt,0);
      // The parse_date function returns Hebrew
      // month values in the range 101 - 114
      month = month%100;
      if (!month) opt.one_year = 1;
    }
    else
    {
      error(0,0,"%s: %s", N_("error"), N_("too many parameters received (expected at most month and year after options list)"));
      exit_main(&opt, 0);
    }

    /********************************************************
    * parameter validation - year
    ********************************************************/
    if (!validate_hdate(CHECK_YEAR_PARM, 0, month, year, FALSE, &h))
    {
      print_parm_error(year_text);
      exit_main(&opt, 0);
    }
    /********************************************************
    * parameter validation - month
    ********************************************************/
    if ((month!=0) && (!validate_hdate(CHECK_MONTH_PARM, 0, month, year, FALSE, &h)))
    {
      print_parm_error(month_text);
      exit_main(&opt, 0);
    }
  }

/************************************************************
* begin processing the user request
************************************************************/

  /************************************************************
  * diaspora awareness
  ************************************************************/
  if (opt.force_israel) opt.diaspora = 0;
    // why no tzset logic here?
  else
  {
    tzset();
    // But the user may be requesting a calendar for a timezone
    // other than the system one, and we need DST awareness if
    // displaying Shabbat times.
    // system timezone is denominated in seconds
    if ( (timezone/-3600) != 2) opt.diaspora = 1;
  }

  /************************************************************
  * print HTML header
  ************************************************************/
  if (opt.html) html_print_header (opt.external_css, opt.force_hebrew);


  /************************************************************
  * print one year
  ************************************************************/
  if (opt.one_year)
  {
    if (opt.three_month)
    {
      if (year > HDATE_HEB_YR_LOWER_BOUND) hdate_set_hdate( &h, 1, 1, year);
      else hdate_set_gdate( &h, 1, 1, year);

      for (month=2; month<num_of_months; month=month+3)
      {
        if (opt.separator != NULL ) print_separator( opt );
        if ( (h.hd_size_of_year > 355 )  &&
           (year > HDATE_HEB_YR_LOWER_BOUND) &&
           (month == 8)               )
          month = 7;
        print_month (month, year, &opt);
      }

      if  ( (year > HDATE_HEB_YR_LOWER_BOUND) &&
          ( (h.hd_size_of_year > 355 ) || (opt.gregorian > 1)) )
      {
        if (opt.separator != NULL ) print_separator( opt );
        opt.three_month = 12;
        print_month (12, year, &opt);
      }
      if (opt.separator != NULL )
      {
        if ( (h.hd_size_of_year > 355 ) && (opt.gregorian > 1) )
          // check that this next line isn't a typo
          // and document why =13 in same discussio
          // as where explained =12
          opt.three_month = 13;
         print_separator( opt );
      }
    }
    else
    {
      if  ( opt.gregorian < 2 )
        hdate_set_hdate( &h, 1, 1, year);

      for (month=1; month<=num_of_months; month++)
      {
        if ( ( opt.gregorian < 2 )       &&
           ( h.hd_size_of_year > 355 ) )
        {
          if (month == 6) print_month(13, year, &opt);
          else if (month == 7)
          {
            print_month(14, year, &opt);
            printf("\n");
            print_month (month, year, &opt);
          }
          else print_month (month, year, &opt);
        }
        else print_month (month, year, &opt);
         if ((opt.footnote) && (month<=num_of_months)) printf("\n");
      }
      if ( (opt.gregorian > 1) && (year > HDATE_HEB_YR_LOWER_BOUND) )
      {
         if (opt.footnote) printf("\n");
        print_month( 1, year+1, &opt) ;
      }
    }
  }


  /************************************************************
  * print only this month
  ************************************************************/
  else print_month (month, year, &opt);


  /************************************************************
  * print HTML footer
  ************************************************************/
  if (opt.html) html_print_footer ();

  exit_main(&opt, 0);
  return 0;
}
