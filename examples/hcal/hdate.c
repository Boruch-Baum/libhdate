/** hdate.c            https://libhdate.sourceforge.net
 * Hebrew date/times information (part of package libhdate)
 *
 *  Copyright (C) 2011-2018, 2022 Boruch Baum  <boruch_baum@gmx.com>
 *                2004-2010 Yaacov Zamir <kzamir@walla.co.il>
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hdate.c -o hdate
 *
 * build:
 * gcc -Wall -g -I "../../src" -L"../../src/.libs" -lhdate -o "%e" "%f"
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

#include <hdate.h>    // For hebrew date  (gcc -I ../../src)
#include <support.h>  // libhdate general macros, including for gettext
#include <stdlib.h>   // For atoi, getenv, setenv
#include <stdio.h>    // For printf, FILE, asprintf
#include <locale.h>   // For setlocale
#include <getopt.h>   // For getopt_long
#include <string.h>   // For strchr, mempcpy, asprintf
#include <fnmatch.h>  // For fnmatch
#include <time.h>     // For time
#include <sys/stat.h> // for stat
#include <error.h>    // For error
#include <errno.h>    // For errno
#include "local_functions.h"    // hcal,hdate common_functions
#include "custom_days.h"        // hcal,hdate common_functions
#include <zdump3.h>             // zdump, zdumpinfo
#include "timezone_functions.h" // for get_tz_adjustment


#define DATA_WAS_NOT_PRINTED 0
#define DATA_WAS_PRINTED 1

// for opt.menu[MAX_MENU_ITEMS]
#define MAX_MENU_ITEMS 10


// quiet levels
#define QUIET_ALERTS         1 // suppress only alert messages
#define QUIET_GREGORIAN      2 // suppress also gregorian date
#define QUIET_DESCRIPTIONS   3 // suppress also time labels
#define QUIET_HEBREW         4 // suppress also Hebrew date


// This is the 'holiday code' used in hdate_holyday.c and hdate_strings.c
#define EREV_PESACH 38


char *debug_var;        // system environment variable

static char * day_text   = N_("day");
static char * month_text = N_("month");
static char * year_text  = N_("year");
static char * sunrise_text = N_("sunrise");
static char * sunset_text  = N_("sunset");
static char * first_light_text = N_("first_light");
static char * talit_text = N_("talit");
static char * shema_text = N_("end_Shema_(GR\"A)");
static char * magen_avraham_text = N_("end_Shema_(M\"A)");
static char * amidah_text = N_("end_amidah");
static char * midday_text = N_("mid-day");
static char * mincha_gedola_text = N_("mincha_gedola");
static char * mincha_ketana_text = N_("mincha_ketana");
static char * plag_hamincha_text = N_("plag_hamincha");
static char * first_stars_text = N_("first_stars");
static char * three_stars_text = N_("three_stars");
static char * sun_hour_text = N_("sun_hour");
static char * candles_text = N_("candle-lighting");
static char * havdalah_text = N_("havdalah");
static char * daf_yomi_text = N_("daf_yomi");
static char * omer_text = N_("omer");
static char * parasha_text = N_("parasha");
static char * holiday_text = N_("holiday_today");
static char * holiday_tabular_text = N_("holiday");
static char * custom_day_text = N_("today_is_also");
static char * custom_day_tabular_text = N_("custom_day");
//  Decided to limit range of dates
// static char * creation_year_text = N_("Creation Year");
static char *sof_achilat_chametz_ma_text  = N_("end_eating_chometz_(M\"A)");
static char *sof_achilat_chametz_gra_text  = N_("end_eating_chometz_(GR\"A)");
static char *sof_biur_chametz_ma_text  = N_("end_owning_chometz_(M\"A)");
static char *sof_biur_chametz_gra_text  = N_("end_owning_chometz_(GR\"A)");


typedef struct  {
  int prefer_hebrew;
  int base_year_h;
  int base_year_g;
  int hebrew;
  int bidi;
  int yom;
  int leShabbat;
  int leSeder;
  int tablular_output;
  int not_sunset_aware;
  int print_tomorrow;    // TRUE if currently after sunset
  int quiet;
  int sun_hour;
  int time_option_requested;
  int first_light;
  int talit;
  int sunrise;
  int magen_avraham;
  int shema;
  int amidah;
  int midday;
  int mincha_gedola;
  int mincha_ketana;
  int plag_hamincha;
  int sunset;
  int first_stars;
  int three_stars;
  int candles;
  int havdalah;
  int times;
  int short_format;
  int only_if_holiday;
  int holidays;
  int omer;
  int only_if_parasha;
  int parasha;
  int julian;
  int diaspora;
  int iCal;
  int daf_yomi;
  int afikomen;
  int  end_eating_chometz_ma;
  int end_eating_chometz_gra;
  int end_owning_chometz_ma;
  int end_owning_chometz_gra;
  int la_omer;
  int emesh;
  int custom_days_cnt;
  int* jdn_list_ptr;    // for custom_days
  char* string_list_ptr;  // for custom_days
  int data_first;      // as opposed to print label first
  int print_epoch;    // time in epoch format
  int menu;
  char* menu_item[MAX_MENU_ITEMS];
  char* tz_name_str;
// double tz_lat;    // unnecessary, here for parallelism
  double tz_lon;      // for sanity checking user input longitude
  int tzif_entries;
  int tzif_index;      // counter into tzif_entries
  void* tzif_data;
  int tz_offset;
  double lat;
  double lon;
// Following are in hcal opt, but not (yet) here:
// int tz;  <--- here it is called tz_offset
// char* tz_name_str;
  time_t epoch_start;    // for dst transition calc
  time_t epoch_end;    // for dst transition calc
  time_t epoch_today;
  int epoch_parm_received;
} option_list;



// TODO - move daf yomi and limud yomi into libhdate api
// Daf Yomi cycle began 02 March 2005 = Julian day 2453432
#define DAF_YOMI_START 2453432
#define DAF_YOMI_LEN      2711

typedef struct {
  int start_day;
  int num_of_days;
  char* e_masechet;
  char* h_masechet; } limud_unit;

static const limud_unit daf_yomi[] = {
{   0,  63, N_("Berachot"), "ברכות" },
{  63, 156, N_("Shabbat"), "שבת" },
{ 219, 104, N_("Eiruvin"), "עירובין" },
{ 323, 120, N_("Pesachim"), "פסחים" },
{ 443,  21, N_("Shekalim"), "שקלים" },
{ 464,  87, N_("Yoma"), "יומא" },
{ 551,  55, N_("Sukkah"), "סוכה" },
{ 606,  39, N_("Beitzah"), "ביצה" },
{ 645,  34, N_("Rosh_HaShannah"), "ראש_השנה" },
{ 679,  30, N_("Taanit"), "תענית" },
{ 709,  31, N_("Megillah"), "מגילה" },
{ 740,  28, N_("Moed_Katan"), "מועד_קטן" },
{ 768,  26, N_("Chagigah"), "חגיגה" },
{ 794, 121, N_("Yevamot"), "יבמות" },
{ 915, 111, N_("Ketubot"), "כתובות" },
{1026,  90, N_("Nedarim"), "נדרים" },
{1116,  65, N_("Nazir"), "נזיר" },
{1181,  48, N_("Sotah"), "סוטה" },
{1229,  89, N_("Gittin"), "גיטין" },
{1318,  81, N_("Kiddushin"), "קידושין" },
{1399, 118, N_("Bava_Kamma"), "בבא_קמא" },
{1517, 118, N_("Bava_Metzia"), "בבא_מציעא" },
{1635, 175, N_("Bava_Batra"), "בבא_בתרא" },
{1810, 112, N_("Sanhedrin"), "סנהדרין" },
{1922,  23, N_("Makkot"), "מכות" },
{1945,  48, N_("Shevuot"), "שבועות" },
{1993,  75, N_("Avodah_Zara"), "עבודה_זרה" },
{2068,  13, N_("Horayot"), "הוריות" },
{2081, 119, N_("Zevachim"), "זבחים" },
{2200, 109, N_("Menachot"), "מנחות" },
{2309, 141, N_("Chullin"), "חולין" },
{2450,  60, N_("Bechorot"), "בכורות" },
{2510,  33, N_("Erchin"), "ערכין" },
{2543,  33, N_("Temurah"), "תמורה" },
{2576,  27, N_("Keritut"), "כריתות" },
{2603,  20, N_("Meilah"), "מעילה" },
{2623,   1, N_("Meilah-Kinnim"), "מעילה_-_קינים" },
{2625,   2, N_("Kinnim"), "קינים" },
{2626,   1, N_("Kinnim-Tamid"), "קינים_-_תמיד" },
{2627,   8, N_("Tamid"), "תמיד" },
{2635,   4, N_("Middot"), "מדות" },
{2639,  72, N_("Niddah"), "נדה" },
{2711,   0,    "",        ""} };


static const char* afikomen[9] = {
  N_("There are no easter eggs in this program. Go away."),
  N_("There is no Chanukah gelt in this program. Leave me alone."),
  N_("Nope, no Hamentashen either. Scram kid, your parents are calling you"),
  N_("Nada. No cheesecake, no apples with honey, no pomegranate seeds. Happy?"),
  N_("Afikomen? Afikomen! Ehrr... Huh?"),
  N_("(grumble...) You win... (moo)"),
  N_("Okay! enough already! MOO! MOO! MoooooH!\n\nSatisfied now?"),
  N_("Etymology: Pesach\n   Derived from the ancient Egyptian word \"Feh Tzach\" meaning \"purchase a new toothbrush\" --based upon a hieroglyphic steele dating from the 23rd dynasty, depicting a procession of slaves engaging in various forms of oral hygiene."),
  N_("You have reached Wit's End.")};



/************************************************************
* print version information
************************************************************/
int print_version ()
{
  printf ("hdate (libhdate) 1.8\n\
Copyright (C) 2011-2013, 2022 Boruch Baum; 2004-2010 Yaacov Zamir\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n");
  return 0;
}

/************************************************************
* print usage information
************************************************************/
void print_usage_hdate ()
{
  printf ("%s\n",
N_("Usage: hdate [options] [coordinates] [timezone] [date_spec]\n\
       hdate [options] [coordinates] [timezone] [julian_day|time_t]\n\n\
       coordinates: -l [NS]yy[.yyy]     -L [EW]xx[.xxx]\n\
                    -l [NS]yy[:mm[:ss]] -L [EW]xx[:mm[:ss]]\n\n\
       timezone:    -z [ [nn[.nn|:mm]] | tz_name ]\n\n\
       date_spec:   [ [[[day] month] year] | flexible_entry ]\n"));
}

/************************************************************
* print "try --help" message
************************************************************/
void print_try_help_hdate ()
{
  printf ("%s\n", N_("Try \'hdate --help\' for more information"));
}


/************************************************************
* print help information
************************************************************/
void print_help ()
{
  print_usage_hdate();

  printf ("%s\n", N_("hdate - display Hebrew date information\nOPTIONS:\n\
   -b --bidi          prints hebrew in reverse (visual)\n\
      --visual\n\
   -d --diaspora      force diaspora readings and holidays.\n\
      --israel        force Eretz Yisroel readings an holidays\n\
      --daf-yomi\n\
   -e --emesh         begin printing times-of-day at sunset or candles.\n\
      --erev          this is the default and can be over-ridden with\n\
                      --no-emesh or --no-erev\n\
   -E --epoch         print times of day in epoch format\n\
   -E --epoch time_t  get information for this epoch date\n\
   -h --holidays      print holiday information.\n\
   -H                 print only if day is a holiday.\n\
   -i --ical          use iCal formated output.\n\
   -j --julian        print Julian day number.\n\
   -m --menu          prompt user-defined menu from config file\n\
   -o --omer          print Sefirat Ha-Omer, number of days only.\n\
                      -oo  \"today is n days in the omer\"\n\
                      -ooo the full text, with weeks and remainder days\n\
      --ba-omer       full omer text in Hebrew\n\
      --la-omer       full omer text in Hebrew\n\
   -q --quiet         suppresses optional output (four levels):\n\
      --quiet-alerts       -q    suppresses only warning messages\n\
      --quiet-gregorian    -qq   also suppresses gregorian date\n\
      --quiet-descritpions -qqq  also suppresses labels and descripions\n\
      --quiet_hebrew       -qqqq also suppresses Hebrew date\n\
   -r --parasha       print weekly reading if day is Shabbat.\n\
   -R                 print only if day is a Shabbat on which there\n\
                      is a weekly reading (ie. not a special holiday\n\
                      reading), and print that regular weekly reading.\n\
   -s --shabbat       print Shabbat start/end times.\n\
      --shabbat-times\n\
      --candles       modify default minhag of 20 minutes. (17<n<91)\n\
      --havdalah      modify default minhag of 3 stars. (19<n<91 minutes)\n\
      --sun           print sunrise/sunset times.\n\n\
   -S --short-format  print using short format.\n\
   -t --times         print day times (three preset verbosity levels):\n\
      --times-of-day  -t    first light, talit, sunrise, midday,\n\
      --day-times           sunset, first stars, three stars, sun hour.\n\
                      -tt   adds sof zman kriyat Shema, sof zman amidah,\n\
                            mincha gedolah, mincha ketana, plag hamincha\n\
                      -ttt  adds sof zman kriyat Shema per Magen Avraham\n\
                      instead of using the presets, customize, with:\n\
                      --first-light  --midday         --shekia\n\
                      --alot         --noon           --tzeit-hakochavim\n\
                      --talit        --chatzot        --first-stars\n\
                      --netz         --mincha-gedola  --three-stars\n\
                      --shema        --mincha-ketana  --magen-avraham\n\
                      --amidah       --plag-hamincha  --sun-hour\n\
                      \n\
                      --sunrise      --sunset         --candle-lighting\n\
                      --end-eating-chometz-ma   --end-eating-chometz-gra\n\
                      --end-owning-chometz-ma   --end-owning-chometz-gra\n\
                    \n\
   -T --table         tabular output, comman delimited, and most suitable\n\
      --tabular       for piping or spreadsheets\n\n\
   -z --timezone nn   timezone, +/-UTC\n\
   -l --latitude yy   latitude yy degrees. Negative values are South\n\
   -L --longitude xx  longitude xx degrees. Negative values are West\n\n\
   --hebrew           forces Hebrew to print in Hebrew characters\n\
   --yom              force Hebrew prefix to Hebrew day of week\n\
   --leshabbat        insert parasha between day of week and day\n\
   --leseder          insert parasha between day of week and day\n\
   --not-sunset-aware don't display next day if after sunset\n\
   --data-first       display data, followed by it's description\n\
   --labels-first     display data descriptions before the data itself\n\
   --prefer-hebrew    interpret ambiguous mm yy as Hebrew date\n\
   --prefer-gregorian interpret ambiguous mm yy as gregorian date\n\n\
All options can be made default in the config file, or menu-ized for\n\
easy selection.\n\
Report bugs to: <http://sourceforge.net/tracker/?group_id=63109&atid=502872>\n\
libhdate home page: <http://libhdate.sourceforge.net>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>\n"));
}


/************************************************************
* begin - error message functions
************************************************************/
void print_alert_sunset ()
{
  error(0,0,"%s", N_("ALERT: The information displayed is for today's Hebrew date.\n \
             Because it is now after sunset, that means the data is\n \
             for the Gregorian day beginning at midnight."));
}
/************************************************************
* end - error message functions
************************************************************/



/************************************************************
* generic print astronomical time
************************************************************/
int print_astronomical_time( char *description,  const int timeval_0, option_list* opt)
{
  time_t timeval_1 = 0;
  char  delim = '\0';
  char* descr = "";

  if (opt->quiet < QUIET_DESCRIPTIONS)
  {
    delim =':';
    descr = description;
  }
  if (timeval_0 < 0)
  {
    if (!opt->print_epoch)
    {
      if (!opt->data_first) printf("%s%c --:--\n", descr,delim);
      else printf("--:-- %s\n", descr);
    }
    else // (opt->print_epoch)
    {
      if (!opt->data_first) printf("%s%c ----------\n", descr,delim);
      else printf("---------- %s\n", descr);
    }
    return DATA_WAS_NOT_PRINTED;
  }
  timeval_1 = opt->epoch_today + timeval_0;
  if (!opt->print_epoch)
  {
    timeval_1 = ( timeval_0 / 60 ) +
        get_tz_adjustment( timeval_1, opt->tz_offset, &opt->tzif_index,
                  opt->tzif_entries, opt->tzif_data );
    if (timeval_1 < 0) timeval_1 = SECONDS_PER_DAY + timeval_1;
  }
  if (opt->data_first)
  {
    if (opt->print_epoch) printf("%10ld %s\n", timeval_1, descr);
    else printf("%02ld:%02ld %s\n", (timeval_1 / 60) % 24, timeval_1 % 60, descr );
  }
  else // (!opt->data_first)
  {
    if (opt->print_epoch) printf("%s%c %10ld\n", descr, delim, timeval_1);
    else printf("%s%c %02ld:%02ld\n", descr, delim, (timeval_1 / 60) % 24, timeval_1 % 60 );
  }
  return DATA_WAS_PRINTED;
}

/************************************************************
* generic print astronomical time for tablular output
************************************************************/
void print_astronomical_time_tabular( const int  timeval_0, option_list* opt)
{
  time_t timeval_1 = 0;

  if (timeval_0 < 0)
  {
    printf(",--:--");
    return;
  }
  timeval_1 = opt->epoch_today + timeval_0;
  if (!opt->print_epoch)
  {
    timeval_1 = ( timeval_0 / 60 ) +
        get_tz_adjustment( timeval_1, opt->tz_offset, &opt->tzif_index,
                  opt->tzif_entries, opt->tzif_data );
    if (timeval_1 < 0) timeval_1 = SECONDS_PER_DAY + timeval_1;
    printf(",%02ld:%02ld", (timeval_1 / 60) % 24, timeval_1 % 60 );
  }
  else printf(",%10ld", timeval_1);
  return;
}


/************************************************************
* get daf yomi information
************************************************************/
int daf_yomi_info( const int julian_day, int* daf, char** masechet, int force_hebrew)
{
  int index, i;

  if (julian_day < 2453432) return FALSE;

  index = (julian_day % DAF_YOMI_START) % DAF_YOMI_LEN;

  for ( i=0; daf_yomi[i+1].start_day <= index ; i++) ;

  // These masechaot ketanot don't all start at 2
  if ( (index > 2622) && (index < 2639) ) *daf = index - 2601;
  else *daf = index - daf_yomi[i].start_day + 2;

  if (force_hebrew) *masechet = daf_yomi[i].h_masechet;
  else *masechet = daf_yomi[i].e_masechet;

  return TRUE;
}

/************************************************************
* print daf yomi information
************************************************************/
int print_daf_yomi( const int julian_day,
          const int force_hebrew, const int force_bidi,
          const int tabular_output, const int data_first )
{
  int daf;
  char* masechet;

  char *daf_str;
  char *bidi_buffer, *bidi_buffer2;
  int   bidi_buffer_len;

  if (!daf_yomi_info( julian_day, &daf, &masechet, force_hebrew))
    return DATA_WAS_NOT_PRINTED;

  if (!force_hebrew)
  {
    if (!tabular_output)
    {
      if (!data_first) printf("%s: %s %d\n", daf_yomi_text, masechet, daf);
      else printf("%s %d %s\n", masechet, daf, daf_yomi_text);
    }
    else  printf(",%s %d", masechet, daf);
  }
  else
  {
    daf_str = hdate_string(HDATE_STRING_INT, daf, HDATE_STRING_LONG, force_hebrew);
    if (!force_bidi)
    {
      if (!tabular_output)
      {
        if (!data_first) printf("%s: %s %s\n", daf_yomi_text, masechet, daf_str);
        else printf("%s %s %s\n", masechet, daf_str, daf_yomi_text);
      }
      else printf(",%s %d", masechet, daf);
    }
    else
    {
      bidi_buffer_len = asprintf(&bidi_buffer, "%s %s", masechet, daf_str);
      bidi_buffer2 = malloc(bidi_buffer_len+1);
      mempcpy(bidi_buffer2, bidi_buffer, bidi_buffer_len);
      bidi_buffer2[bidi_buffer_len]='\0';
      revstr(bidi_buffer2, bidi_buffer_len);
      if (!tabular_output)
      {
        if (!data_first) printf("%s: %s\n",daf_yomi_text, bidi_buffer2);
        else printf("%s %s\n", bidi_buffer2 ,daf_yomi_text);
      }
      else printf(",%s",bidi_buffer2);
      if (bidi_buffer  != NULL)  free(bidi_buffer);
      if (bidi_buffer2 != NULL)  free(bidi_buffer2);
    }
    if (daf_str  != NULL)  free(daf_str);
  }
  return DATA_WAS_PRINTED;
}


/************************************************************
* print iCal header
************************************************************/
int print_ical_header ()
{
  // Print start of iCal format
  printf ("BEGIN:VCALENDAR\n");
  printf ("VERSION:2.0\n");
  printf ("CALSCALE:GREGORIAN\n");
  printf ("METHOD:PUBLISH\n");

  return 0;
}




/************************************************************
* print iCal footer
************************************************************/
int print_ical_footer ()
{
  // Print end of iCal format
  printf ("END:VCALENDAR\n");

  return 0;
}





/************************************************************
* find Shabbat
*  this function is to become obsolete - I want a feature
*  for this standard in libhdate
************************************************************/
int find_shabbat (hdate_struct * h, int opt_d)
{
  hdate_struct coming_Shabbat;

  hdate_set_jd (&coming_Shabbat, h->hd_jd+(7-h->hd_dw));
  //  this return value is the reading number, used to print parshiot
  return hdate_get_parasha (&coming_Shabbat, opt_d);
  }



/************************************************************
* print one date - both Gregorian and Hebrew
************************************************************/
int print_date (hdate_struct* h, hdate_struct* tomorrow, const option_list* opt)
{
  char *motzash    = "";  // prefix for Saturday night
  char *eve_before    = "";   // prefix if after sunset
  char *for_day_of_g  = "";  // Hebrew prefix for day of week
  char *apostrophe  = "";  // Hebrew suffix for day of week
  char *bet_g     = "";  // Hebrew prefix for Gregorian month
  char *bet_h         = "";  // Hebrew prefix for Hebrew month
  char *psik_mark     = "";  // --force-yom
  char *for_day_of_h  = "";  // --force-yom
  char *for_week_of   = "";  // --force-leshabbat --force-leseder
  int  is_parasha_read;
  int h_dow_form    = HDATE_STRING_SHORT;
  char *h_dow;

  char *hday_int_str, *hyear_int_str;
  hday_int_str = hdate_string(HDATE_STRING_INT, h->hd_day, HDATE_STRING_LONG, opt->hebrew);
  hyear_int_str = hdate_string(HDATE_STRING_INT, h->hd_year, HDATE_STRING_LONG, opt->hebrew);

  /************************************************************
  * preliminary - if it's after sunset, it's tomorrow already
  * but we will only acknowledge this if printing in the long
  * format (which is the default)
  ************************************************************/
  if (opt->print_tomorrow)
  {
    if (opt->hebrew)
    {
      if (h->hd_dw==7) motzash = "מוצ\"ש, ";
      if (h->hd_dw==6) eve_before = "ליל";
      else  eve_before = "אור ל";
      if (!opt->yom) for_day_of_h="-";
    }
    else eve_before = N_("eve of ");
  }


  /************************************************************
  * preliminary - prepare Hebrew prefixes if forcing any Hebrew
  ************************************************************/
  if (opt->hebrew)
  {
    bet_h="ב";
    // preliminary - prepare Yom prefix
    if (opt->yom)
    {
      if ( !( opt->print_tomorrow && (h->hd_dw==6) )) for_day_of_h="יום";
// This version does not put an apostrophe on shabbat
// - good if we are not abbreviating days of the week, but we are
//      if  ( ((h->hd_dw!=7) || ((h->hd_dw==7) && (opt->print_tomorrow))) &&
//        ( !((opt->print_tomorrow) && (h->hd_dw==6))) ) apostrophe = "'";
//
// This version does put an apostrophe on shabbat
// - good if we are abbreviating shabbat as a single letter
//   but the single letter we are using is Shin, not Zayin, which seems weird
//      if ( !((opt->print_tomorrow) && (h->hd_dw==6))) apostrophe = "'";
      if ( !( opt->print_tomorrow && (h->hd_dw==6) )) apostrophe = "'";
      if ((h->hd_dw==6) && opt->print_tomorrow ) h_dow_form = HDATE_STRING_LONG;
      psik_mark=",";

      if (opt->leShabbat)
        {
        if (h->hd_dw==7)    for_week_of="פרשת";
        else          for_week_of="לשבת";
        }
      else if (opt->leSeder)  for_week_of="לסדר";
    }
  }

  /************************************************************
  *  preliminary - prepare the Hebrew prefixes
  *  for the Gregorian month and day of week
  ************************************************************/
  if (hdate_is_hebrew_locale())
    { bet_g="ב"; for_day_of_g="יום ";}



  /************************************************************
  * Three major print formats: iCal, short, long
  ************************************************************/


  /************************************************************
  * iCal format
  ************************************************************/
  if (opt->iCal)
  {
    printf ("%s%s %s%s ", for_day_of_g,  hday_int_str, bet_h,
        hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew));

    printf ("%s", hyear_int_str);
  }


  /************************************************************
  * short format
  ************************************************************/
  else if (opt->short_format)
  {
    printf ("%d.%d.%d  ", h->gd_day, h->gd_mon, h->gd_year);


    if ( !hdate_is_hebrew_locale() && !opt->hebrew )
    {
      printf ("%d", h->hd_day);
    }
    else
    {
      printf ("%s", hday_int_str);
    }

    printf (" %s %s\n",
      hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew),
      hyear_int_str);
  }


  /************************************************************
  * long (normal) format
  ************************************************************/
  else
  {
    /************************************************************
    * Gregorian date - the easy part
    ************************************************************/
    if (opt->quiet < QUIET_GREGORIAN)
      printf ("%s%s, %d %s%s %d, ",
        for_day_of_g,
        hdate_string( HDATE_STRING_DOW, h->hd_dw, opt->short_format, HDATE_STRING_LOCAL),
        h->gd_day,
        bet_g,
        hdate_string( HDATE_STRING_GMONTH, h->gd_mon, opt->short_format, HDATE_STRING_LOCAL),
        h->gd_year);


    /************************************************************
    * Start of the Hebrew date
    ************************************************************/
    if (opt->print_tomorrow) *h = *tomorrow;


    /************************************************************
    * All these definitions were backports to enable bidi
    ************************************************************/
    char *hebrew_buffer, *hebrew_buffer1, *hebrew_buffer2, *hebrew_buffer3, *hebrew_buffer4, *hebrew_buffer_next;
    size_t hebrew_buffer_len = 0;
    size_t hebrew_buffer1_len = 0;
    size_t hebrew_buffer2_len = 0;
    size_t hebrew_buffer3_len = 0;
    size_t hebrew_buffer4_len = 0;


    /************************************************************
    * prepare buffers with  Hebrew day of week, including
    * possibly Shabbat name
    ************************************************************/
    if (opt->hebrew)
    {
      if (opt->yom)
        {
				if ((h->hd_dw == 7) && (h_dow_form == HDATE_STRING_SHORT))
					h_dow = "ז";
				else
					h_dow = hdate_string( HDATE_STRING_DOW, h->hd_dw, h_dow_form, opt->hebrew);
        hebrew_buffer1_len = asprintf(&hebrew_buffer1, "%s%s%s %s%s", motzash, eve_before, for_day_of_h, h_dow, apostrophe);

        if ( opt->leShabbat || opt->leSeder )
        {
          is_parasha_read = find_shabbat(h, opt->diaspora);
          if (is_parasha_read)
              hebrew_buffer2_len = asprintf(&hebrew_buffer2, " %s %s", for_week_of,
              hdate_string( HDATE_STRING_PARASHA, is_parasha_read, opt->short_format, opt->hebrew)
              );
        }
        hebrew_buffer3_len = asprintf (&hebrew_buffer3, "%s ", psik_mark);
      }
    }
    else hebrew_buffer1_len = asprintf(&hebrew_buffer1, "%s", eve_before);


    hday_int_str  = hdate_string(HDATE_STRING_INT, h->hd_day,  HDATE_STRING_LONG, opt->hebrew);

    //  Decided to limit range of dates
    // year 0 is the six-day long 'year' of creation
    //  if (h->hd_year == 0) hyear_int_str = creation_year_text;
    //  else hyear_int_str = hdate_string(HDATE_STRING_INT, h->hd_year, HDATE_STRING_LONG, opt->hebrew);
    hyear_int_str = hdate_string(HDATE_STRING_INT, h->hd_year, HDATE_STRING_LONG, opt->hebrew);
    /************************************************************
    * prepare buffers with Hebrew dd mmmm yyyy
    ************************************************************/
    hebrew_buffer4_len =
      asprintf (&hebrew_buffer4, "%s %s%s %s",
        hday_int_str,
        bet_h,
        hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew),
        hyear_int_str);

    /************************************************************
    * prepare another GrUB - grand unified buffer
    ************************************************************/
    hebrew_buffer_len = hebrew_buffer1_len + hebrew_buffer2_len + hebrew_buffer3_len + hebrew_buffer4_len;
    hebrew_buffer = malloc(hebrew_buffer_len+1);

    hebrew_buffer_next = mempcpy(hebrew_buffer, hebrew_buffer1, hebrew_buffer1_len);
    if (hebrew_buffer2_len > 0) hebrew_buffer_next = mempcpy(hebrew_buffer_next, hebrew_buffer2, hebrew_buffer2_len);
    if (hebrew_buffer3_len > 0) hebrew_buffer_next = mempcpy(hebrew_buffer_next, hebrew_buffer3, hebrew_buffer3_len);
    hebrew_buffer_next = mempcpy(hebrew_buffer_next, hebrew_buffer4, hebrew_buffer4_len);
    hebrew_buffer[hebrew_buffer_len]='\0';
    /************************************************************
    * bidi support (what all the buffering was for)
    ************************************************************/
    if (opt->bidi) revstr(hebrew_buffer, hebrew_buffer_len);

    /************************************************************
    * Finally. print the information
    ************************************************************/
    printf ("%s\n", hebrew_buffer);

    /************************************************************
    * CLEANUP - free allocated memory
    ************************************************************/
    if (hebrew_buffer1_len > 0) free(hebrew_buffer1);
    if (hebrew_buffer2_len > 0) free(hebrew_buffer2);
    if (hebrew_buffer3_len > 0) free(hebrew_buffer3);
    if (hebrew_buffer4 != NULL) free(hebrew_buffer4);
    if (hebrew_buffer  != NULL) free(hebrew_buffer);
  }

  /************************************************************
  * CLEANUP - free allocated memory
  ************************************************************/
  if (hday_int_str  != NULL)  free(hday_int_str);
  if ((hyear_int_str != NULL) && (h->hd_year != 0))  free(hyear_int_str);

  return 0;
}


/************************************************************
* print candle-lighting time; knissat Shabbat
************************************************************/
int print_candles (option_list* opt,  int sunset)
{
  int candles_time;
  // FIXME - allow for further minhag variation
  if (opt->candles != 1) candles_time = sunset - (opt->candles * 60);
  else candles_time = sunset - (DEFAULT_CANDLES_MINUTES * 60);

  // print candlelighting times
  print_astronomical_time( candles_text, candles_time, opt);
  return DATA_WAS_PRINTED;
}

/************************************************************
* print havdalah time; tzeit Shabbat
************************************************************/
int print_havdalah (option_list* opt, int sunset)
{
  int havdalah_time;
  // FIXME - allow for further minhag variation
  if (opt->havdalah != 1) havdalah_time = sunset + (opt->havdalah * 60);
  else havdalah_time = sunset + (DEFAULT_MOTZASH_MINUTES * 60);

  // print motzay shabat
  print_astronomical_time( havdalah_text, havdalah_time, opt);
  return DATA_WAS_PRINTED;
}

/************************************************************
* option 't' - day times
************************************************************/
int print_times ( hdate_struct * h, option_list* opt)
{
  hdate_struct h_emesh;
  int sun_hour, first_light, talit, sunrise;
  int midday, sunset, first_stars, three_stars;
  int data_printed = 0;
  int ma_sun_hour;
  int place_holder;

  /** Originally, we got times using this next single system call; however this function
   *  returns values rounded (formerly truncated) to the minute, and I want better because
   *  I want accuracy in sha'a zmanit in order to derive other times using it. Refer to the
   *  source code of hdate_get_utc_sun_time_full for comparison. We now use multiple calls
   *  to hdate_get_utc_sun_time_deg_seconds
   *
  hdate_get_utc_sun_time_full (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
                 &sun_hour, &first_light, &talit, &sunrise,
                 &midday, &sunset, &first_stars, &three_stars);
  *
  **  Now that we are checking for dst adjustments, we need input seconds for all time values
  */

  if (opt->emesh)
  {
    if (opt->print_epoch) opt->epoch_today = opt->epoch_today - SECONDS_PER_DAY;
    if ( opt->candles || opt->havdalah )
    {
      hdate_set_jd (&h_emesh, h->hd_jd-1);
    }
    else
    {
      hdate_jd_to_gdate (h->hd_jd-1, &h_emesh.gd_day, &h_emesh.gd_mon, &h_emesh.gd_year);
    }
    hdate_get_utc_sun_time_deg_seconds(h_emesh.gd_day, h_emesh.gd_mon, h_emesh.gd_year,
                      opt->lat, opt->lon, 90.833, &sunrise, &sunset);
    if ( opt->candles && (h_emesh.hd_dw == 6) )
    {
      data_printed = data_printed | print_candles(opt, sunset);
    }
    if (opt->sunset)
    {
      data_printed = data_printed | print_astronomical_time( sunset_text, sunset, opt);
    }
    if (opt->first_stars)
    {
      hdate_get_utc_sun_time_deg_seconds(h_emesh.gd_day, h_emesh.gd_mon, h_emesh.gd_year,
                        opt->lat, opt->lon, 96.0, &place_holder, &first_stars);
      data_printed = data_printed | print_astronomical_time( first_stars_text, first_stars, opt);
    }
    if ( opt->havdalah && (h_emesh.hd_dw == 7) )
    {
      data_printed = data_printed | print_havdalah(opt, sunset);
    }
    if (opt->three_stars)
    {
      hdate_get_utc_sun_time_deg_seconds (h_emesh.gd_day, h_emesh.gd_mon, h_emesh.gd_year,
                        opt->lat, opt->lon, 98.5, &place_holder, &three_stars);
      data_printed = data_printed | print_astronomical_time( three_stars_text, three_stars, opt);
    }
    if (opt->print_epoch) opt->epoch_today = opt->epoch_today + SECONDS_PER_DAY;
  }

  hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 90.833, &sunrise, &sunset);
  hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 106.01, &first_light, &place_holder);
  hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 101.0, &talit, &place_holder);
  hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 96.0, &place_holder, &first_stars);
  hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 98.5, &place_holder, &three_stars);

  // sha'a zmanit according to the GR"A, in seconds
  sun_hour = (sunset - sunrise) / 12;
  // sha'a zmanit according to the Magen Avraham, in seconds
  ma_sun_hour = (first_stars - first_light)/12;
  //
  midday = (sunset + sunrise) / 2;


  if (opt->first_light) data_printed = data_printed | print_astronomical_time( first_light_text, first_light, opt);
  if (opt->talit)       data_printed = data_printed | print_astronomical_time( talit_text, talit, opt);
  if (opt->sunrise)     data_printed = data_printed | print_astronomical_time( sunrise_text, sunrise, opt);

  // FIXME - these times will be moved into the libhdate api
  // sof zman kriyat Shema (verify that the Magen Avraham calculation is correct!)
  if (opt->magen_avraham)
            data_printed = data_printed | print_astronomical_time( magen_avraham_text, first_light + (3 * ma_sun_hour), opt);
  if (opt->shema)    data_printed = data_printed | print_astronomical_time( shema_text, sunrise + (3 * sun_hour), opt);
  // sof zman tefilah
  if (opt->amidah)  data_printed = data_printed | print_astronomical_time( amidah_text, sunrise + (4 * sun_hour), opt);


  // TODO - if an erev pesach time was explicitly requested (ie. NOT by -t option),
  //    for a day that is not erev pesach, give the requested information, for the
  //    coming erev pesach, along with a (detailed) message  explaining that the
  //    requested day was not erev pesach. Also give the gregorian date of that
  //    erev pesach if the user wanted gregorian output.
  //    - however, if the explicit erev pesach time request was made with a command
  //    line specifying a month range that includes erev pesach, or a year range,
  //    then just use the code as is now (ie. ignore on days that are not erev pesach
  // TODO - erev pesach times for tabular output
  if ( (h->hd_mon = 7) && (h->hd_day == 14) )
  {
    //  print_astronomical_time( "Magen Avraham sun hour", ma_sun_hour, 0);
    if (opt->end_eating_chometz_ma)
      data_printed = data_printed | print_astronomical_time( sof_achilat_chametz_ma_text, first_light + (4 * ma_sun_hour), opt);
    if (opt->end_eating_chometz_gra)
      data_printed = data_printed | print_astronomical_time( sof_achilat_chametz_gra_text, sunrise + (4 * sun_hour), opt);
    if (opt->end_owning_chometz_ma)
      data_printed = data_printed | print_astronomical_time( sof_biur_chametz_ma_text, first_light + (5 * ma_sun_hour), opt);
    if (opt->end_owning_chometz_gra)
      data_printed = data_printed | print_astronomical_time( sof_biur_chametz_gra_text, sunrise + (5 * sun_hour), opt);
  }


  if (opt->midday)      data_printed = data_printed | print_astronomical_time( midday_text, midday, opt);


  // FIXME - these times will be moved into the libhdate api
  // mincha gedola
  // TODO - There are two other shitot for this:
  //     shaot zmaniot, and shaot zmaniot lechumra
  if (opt->mincha_gedola) data_printed = data_printed | print_astronomical_time( mincha_gedola_text, midday + (30*60), opt);

  if (opt->mincha_ketana) data_printed = data_printed | print_astronomical_time( mincha_ketana_text, sunrise + (9.5 * sun_hour), opt);
  if (opt->plag_hamincha) data_printed = data_printed | print_astronomical_time( plag_hamincha_text, sunrise + (10.75 * sun_hour), opt);
  if ( opt->candles && (h->hd_dw == 6) )
              data_printed = data_printed | print_candles(opt, sunset);
  if (opt->sunset)        data_printed = data_printed | print_astronomical_time( sunset_text, sunset, opt);
  if (opt->first_stars)   data_printed = data_printed | print_astronomical_time( first_stars_text, first_stars, opt);
  if ( opt->havdalah && (h->hd_dw == 7) )
              data_printed = data_printed | print_havdalah(opt, sunset);
  if (opt->three_stars)   data_printed = data_printed | print_astronomical_time( three_stars_text, three_stars, opt);

  // if (opt->sun_hour)     data_printed = data_printed | print_astronomical_time( opt->quiet, sun_hour_text, sun_hour/60, 0, opt->data_first);
  if (opt->sun_hour)
  {
    data_printed = TRUE;
    if (opt->quiet >= QUIET_DESCRIPTIONS)
    {
      if (opt->print_epoch) printf(" %05d\n", sun_hour );
      else printf(" %02d:%02d:%02d\n", sun_hour/3600, (sun_hour%3600)/60, sun_hour%60 );
    }
    else
    {
      if (opt->print_epoch)
      {
        if (!opt->data_first) printf("%s: %05d\n", sun_hour_text, sun_hour );
        else printf("%05d %s\n", sun_hour, sun_hour_text);
      }
      else // (!opt->print_epoch)
      {
        if (!opt->data_first) printf("%s: %02d:%02d:%02d\n", sun_hour_text,
                      sun_hour/3600, (sun_hour%3600)/60, sun_hour%60 );
        else printf("%02d:%02d:%02d %s\n", sun_hour/3600, (sun_hour%3600)/60, sun_hour%60,
                    sun_hour_text);
      }
    }
  }
  return data_printed;
}


/************************************************************
* option 'o' - sefirat ha'omer
************************************************************/
// parameter hebrew_form is currently unnecessary, but retained
// for future feature to force a fully hebrew string
int print_omer (hdate_struct * h, const option_list* opt)
{
  int omer_day;
  char* omer_int_str;
  char* bidi_buffer;
  int   bidi_buffer_len;
  char* empty_text = "";
  char* days_text = N_("days");
  char* in_the_omer_text = N_("in_the_omer");
  char* today_is_day_text = N_("today_is_day");
//char* h_in_the_omer_text = "";
//char* h_today_is_day_text = "";

  omer_day = hdate_get_omer_day(h);
  if (omer_day == 0)   return DATA_WAS_NOT_PRINTED;

  if ( !opt->data_first && (opt->quiet < QUIET_DESCRIPTIONS) )
    printf ("%s: ", omer_text);

  // short format; just the numeric value
  if (opt->omer == 1)
  {
    if (!opt->hebrew) printf ("%5d", omer_day);
    else
    {
      omer_int_str = hdate_string(HDATE_STRING_INT, omer_day, HDATE_STRING_LONG, HDATE_STRING_HEBREW);
      if (!opt->bidi) printf("  %*s", (strlen(omer_int_str)==5?5:4), omer_int_str);
      else
      {
        bidi_buffer_len = asprintf (&bidi_buffer, "%s",  omer_int_str);
        revstr(bidi_buffer, bidi_buffer_len);
        printf("  %*s", (int)(bidi_buffer_len==5?5:4), bidi_buffer);
        free(bidi_buffer);
      }
      free(omer_int_str);
    }
  }

  // short text format
  else if (opt->omer == 2)
  {
		if (opt->hebrew)
		{
      char* b_l_omer = "בעומר";
      if (opt->la_omer) { b_l_omer = "לעומר"; }
			printf ("  %2d_%s_%s", omer_day, "ימים", b_l_omer);
		}
		else
		{
			if (omer_day == 1) days_text = day_text;
			printf ("%2d_%s_%s", omer_day, days_text, in_the_omer_text);
		}
  }
  else // long text format
  {

    if (opt->hebrew)
    {
      // Let's try to construct all the options
      // for this Hebrew sentence as atoms
      // for a single call to asprintf ....
      char* hayom = "היום_";
      char* yom = "";
      char* days = "";
      char* that_are = "שהם_";
      char* weeks = "שבועות";
      char* vav = "_ו";
      char* days2 = "ימים,_";
      char* b_l_omer = "בעומר";

      char* n1 = NULL;
      char* n2 = NULL;
      char* n3 = NULL;

      int n2_needs_free = FALSE;
      int n3_needs_free = FALSE;

      if (opt->la_omer) b_l_omer = "לעומר";

      n1 = hdate_string(HDATE_STRING_OMER, omer_day, HDATE_STRING_LONG, HDATE_STRING_HEBREW);

      if (omer_day == 1) yom = "יום_";
      else
      {
        if (omer_day > 10) days = "יום,_";
        else if (omer_day > 6) days = "ימים,_";
        else days = "ימים_";
      }

      if  (omer_day < 7)
      {
        that_are = empty_text;
        weeks = empty_text;
        vav = empty_text;
        days2 = empty_text;
        n2 = empty_text;
        n3 = empty_text;
      }
      else
      {
        if (omer_day < 14)
        {
          weeks = "שבוע_אחד";
          n2 = empty_text;
        }
        else
        {
          n2 = hdate_string(HDATE_STRING_OMER, omer_day/7, HDATE_STRING_LONG, HDATE_STRING_HEBREW);
          n2_needs_free = TRUE;
        }

        if (omer_day%7 == 0)
        {
          vav = ",_";
          days2 = empty_text;
          n3 = empty_text;
        }
        else
        {
          if (omer_day%7 == 1)
          {
            vav = "_ויום_אחד,_";
            days2 = empty_text;
            n3 = empty_text;
          }
          else
          {
            n3 = hdate_string(HDATE_STRING_OMER, omer_day%7, HDATE_STRING_LONG, HDATE_STRING_HEBREW);
            n3_needs_free = TRUE;
          }
        }
      }

      bidi_buffer_len = asprintf(&bidi_buffer, "%s%s%s%s%s%s%s%s%s%s%s",
        hayom, yom,  n1,  days, that_are,  n2,  weeks, vav,  n3,  days2, b_l_omer);

      if (opt->bidi) revstr(bidi_buffer, bidi_buffer_len);
      printf("%s", bidi_buffer);

      if (n1 != NULL) free(n1);
      if (n2_needs_free && (n2 != NULL)) free(n2);
      if (n3_needs_free && (n3 != NULL)) free(n3);
      if (bidi_buffer_len != -1) free(bidi_buffer);
    }
    else // !opt->hebrew
    {
      if (omer_day == 1) printf ("%s ", today_is_day_text);
      else printf ("%s_", N_("today_is"));

      printf("%d",omer_day);

      if (omer_day > 1) printf("_%s", days_text);

      if (omer_day > 6)
      {
        printf("%s_%d_", N_(",_which_is"),omer_day/7);

        if (omer_day < 14) printf ("%s", N_("week"));
        else printf("%s", N_("weeks"));

        if (omer_day%7 != 0)
        {
          printf ("_%s_%d", N_("and"), omer_day%7);
          if (omer_day%7 != 1) printf (" %s",  days_text);
          else printf ("_%s",  day_text);
        }
        printf("%s", N_(","));
      }
      printf("_%s", in_the_omer_text);
    }
  }
  if ((opt->data_first) && (opt->quiet < QUIET_DESCRIPTIONS))
    printf (" %s", omer_text);
  printf("\n");
  return DATA_WAS_PRINTED;
}



/************************************************************
* option 'r' - parashat hashavua
************************************************************/
// now part of function print_day
// this function name may be restored when/if bidi/force hebrew
// becomes supported for option -r
//
/* int print_reading (hdate_struct * h, int opt.diaspora, int short_format, int opt_i)
{
  int reading;

  reading = hdate_get_parasha (h, opt.diaspora);
  if (reading)
  {
    printf ("%s  %s\n", N_("Parashat"), hdate_string (reading, short_format));
    return DATA_WAS_PRINTED;
  }
  else return DATA_WAS_NOT_PRINTED;
}
*/


/************************************************************
* print tabular header
************************************************************/
void print_tabular_header( const option_list* opt)
{
  if (opt->quiet >= QUIET_DESCRIPTIONS) return;

  if (opt->quiet < QUIET_GREGORIAN) printf("%s,",N_("Gregorian date"));
  printf("%s", N_("Hebrew Date"));

  if (opt->emesh)
  {
    if (opt->candles) printf(",%s", candles_text);
    if (opt->sunset) printf(",%s",sunset_text);
    if (opt->first_stars) printf(",%s",first_stars_text);
    if (opt->three_stars) printf(",%s",three_stars_text);
    if (opt->havdalah) printf(",%s", havdalah_text);
  }
  if (opt->first_light) printf(",%s",first_light_text);
  if (opt->talit) printf(",%s",talit_text);
  if (opt->sunrise) printf(",%s",sunrise_text);
  if (opt->magen_avraham) printf(",%s",magen_avraham_text);
  if (opt->shema) printf(",%s",shema_text);
  if (opt->amidah) printf(",%s",amidah_text);

  if (opt->end_eating_chometz_ma)   printf(",%s", sof_achilat_chametz_ma_text);
  if (opt->end_eating_chometz_gra) printf(",%s", sof_achilat_chametz_gra_text);
  if (opt->end_owning_chometz_ma)   printf(",%s", sof_biur_chametz_ma_text);
  if (opt->end_owning_chometz_gra) printf(",%s", sof_biur_chametz_gra_text);


  if (opt->midday) printf(",%s",midday_text);
  if (opt->mincha_gedola) printf(",%s",mincha_gedola_text);
  if (opt->mincha_ketana) printf(",%s",mincha_ketana_text);
  if (opt->plag_hamincha) printf(",%s",plag_hamincha_text);
  if (opt->candles) printf(",%s", candles_text);
  if (opt->sunset) printf(",%s",sunset_text);
  if (opt->first_stars) printf(",%s",first_stars_text);
  if (opt->three_stars) printf(",%s",three_stars_text);
  if (opt->havdalah) printf(",%s", havdalah_text);
  if (opt->sun_hour) printf(",%s",sun_hour_text);
  if (opt->daf_yomi) printf(",%s",daf_yomi_text);
  if (opt->omer) printf(",%s",omer_text);
  if (opt->parasha) printf(",%s", parasha_text);
  if (opt->holidays) printf(",%s", holiday_tabular_text);
  if ( opt->holidays && opt->custom_days_cnt ) printf(";%s", custom_day_tabular_text);
  printf("\n");
return;
}

/************************************************************
* print one day - tabular output *
************************************************************/
int print_day_tabular (hdate_struct* h, option_list* opt)
{
  int sun_hour = -1;
  int first_light = -1;
  int talit = -1;
  int sunrise = -1;
  int midday = -1;
  int sunset = -1;
  int first_stars = -1;
  int three_stars = -1;
  int candles_time;
  int havdalah_time;

  hdate_struct h_emesh;
  hdate_struct tomorrow;
  int data_printed = 0;

  int parasha = 0;
  int holiday = 0;
  int omer_day = 0;

  char *hday_str  = NULL;
  char *hyear_str = NULL;

  char *hebrew_buffer = NULL;    // for bidi (revstr)
  size_t hebrew_buffer_len = 0;  // for bidi (revstr)

  int ma_sun_hour;
  int place_holder;

  /************************************************************
  * options -R, -H are restrictive filters, so if there is no
  * parasha reading / holiday, print nothing.
  ************************************************************/
  // parasha = hdate_get_parasha (h, opt->diaspora);
  // holiday = hdate_get_holyday (h, opt->diaspora);
  // if (opt->only_if_parasha && opt->only_if_holiday && !parasha && !holiday)
  //   return 0;
  // if (opt->only_if_parasha && !opt->only_if_holiday && !parasha)
  //   return 0;
  // if (opt->only_if_holiday && !opt->only_if_parasha && !holiday)
  //   return 0;
  if ( opt->parasha || opt->only_if_parasha )
    parasha = hdate_get_parasha (h, opt->diaspora);

  if ( opt->holidays || opt->only_if_holiday )
    holiday = hdate_get_halachic_day (h, opt->diaspora);

  if ( (opt->only_if_parasha && opt->only_if_holiday && !parasha && !holiday)  || // eg. Shabbat Chanukah
     (opt->only_if_parasha && !opt->only_if_holiday && !parasha)        || // eg. regular Shabbat
     (opt->only_if_holiday && !opt->only_if_parasha && !holiday)        )  // eg. Holidays
    return 0;

  // TODO - decide how to handle custom_days in the context of
  //        opt->only_if_holiday. Possibly add options: custom_day
  //        only_if_custom_day, and suppress_custom_day

  // TODO - possibly add options: custom_day, suppress_custom_day


  /************************************************************
  * print Gregorian date
  ************************************************************/
  if (opt->quiet < QUIET_GREGORIAN)
    printf ("%d.%d.%d,", h->gd_day, h->gd_mon, h->gd_year);


  /************************************************************
  * begin - print Hebrew date
  ************************************************************/
  if (opt->quiet < QUIET_HEBREW)
  {
    // BUG - looks like a bug - why sunset awareness in tabular output?
    if (opt->print_tomorrow)  hdate_set_jd (&tomorrow, (h->hd_jd)+1);

    if (opt->bidi)
    {
      hday_str  = hdate_string(HDATE_STRING_INT, h->hd_day,  HDATE_STRING_LONG, opt->hebrew);
      hyear_str = hdate_string(HDATE_STRING_INT, h->hd_year, HDATE_STRING_LONG, opt->hebrew);
      hebrew_buffer_len =
        asprintf (&hebrew_buffer, "%s %s %s",
          hday_str,
          hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew),
          hyear_str);
      revstr(hebrew_buffer, hebrew_buffer_len);
      printf ("%s", hebrew_buffer);
      if (hebrew_buffer != NULL) free(hebrew_buffer);
    }
    else
    {
      // review sometime whether both checks are still necessary (see below for hyear)
      //if ((!hdate_is_hebrew_locale()) && (!opt->hebrew))
      //{  /* non hebrew numbers */
      //  printf ("%d", h->hd_day);
      //}
      //else /* Hebrew */
      //{
        hday_str = hdate_string(HDATE_STRING_INT, h->hd_day, HDATE_STRING_LONG,opt->hebrew);
        printf ("%s", hday_str);
      //}

      printf (" %s ",
        hdate_string( HDATE_STRING_HMONTH , h->hd_mon, HDATE_STRING_LONG, opt->hebrew));

      hyear_str = hdate_string(HDATE_STRING_INT, h->hd_year, HDATE_STRING_LONG,opt->hebrew);
      printf ("%s", hyear_str);
    }
    if (hday_str  != NULL) free(hday_str);
    if (hyear_str != NULL) free(hyear_str);
  }
  /************************************************************
  * end - print Hebrew date
  ************************************************************/


  /************************************************************
  * begin - print times of day
  ************************************************************/
  if (opt->emesh)
  {
    if (opt->print_epoch) opt->epoch_today = opt->epoch_today - SECONDS_PER_DAY;
    if ( opt->candles || opt->havdalah )
    {
      // also for day of week
      hdate_set_jd (&h_emesh, h->hd_jd-1);
    }
    else
    {
      hdate_jd_to_gdate (h->hd_jd-1, &h_emesh.gd_day, &h_emesh.gd_mon, &h_emesh.gd_year);
    }
    hdate_get_utc_sun_time_deg_seconds(h_emesh.gd_day, h_emesh.gd_mon, h_emesh.gd_year,
                      opt->lat, opt->lon, 90.833, &sunrise, &sunset);
    if (opt->candles)
    {
      if ( (h_emesh.hd_dw != 6) && (!opt->only_if_parasha) )
        print_astronomical_time_tabular( -1, opt);
      else
      {
        // FIXME - allow for further minhag variation
        if (opt->candles != 1) candles_time = sunset - (opt->candles * 60);
        else candles_time = sunset - (DEFAULT_CANDLES_MINUTES * 60);
        print_astronomical_time_tabular( candles_time, opt);
      }
    }
    if (opt->sunset) print_astronomical_time_tabular( sunset, opt);
    if (opt->first_stars)
    {
      hdate_get_utc_sun_time_deg_seconds(h_emesh.gd_day, h_emesh.gd_mon, h_emesh.gd_year,
                        opt->lat, opt->lon, 96.0, &place_holder, &first_stars);
      print_astronomical_time_tabular( first_stars, opt);
    }
    if (opt->three_stars)
    {
      hdate_get_utc_sun_time_deg_seconds (h_emesh.gd_day, h_emesh.gd_mon, h_emesh.gd_year,
                        opt->lat, opt->lon, 98.5, &place_holder, &three_stars);
      print_astronomical_time_tabular( three_stars, opt);
    }
    if (opt->havdalah)
    {
      if ( (h_emesh.hd_dw != 7)  && !opt->only_if_parasha )
        print_astronomical_time_tabular( -1, opt);
      else
      {
        // FIXME - allow for further minhag variation
        if (opt->havdalah != 1) havdalah_time = sunset + (opt->havdalah * 60);
        else havdalah_time = sunset + (DEFAULT_MOTZASH_MINUTES * 60);
        print_astronomical_time_tabular( havdalah_time, opt);
      }
    }
    if (opt->print_epoch) opt->epoch_today = opt->epoch_today + SECONDS_PER_DAY;
  }

  hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 90.833, &sunrise, &sunset);
  hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 106.01, &first_light, &place_holder);
  hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 101.0, &talit, &place_holder);
  hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 96.0, &place_holder, &first_stars);
  hdate_get_utc_sun_time_deg_seconds (h->gd_day, h->gd_mon, h->gd_year, opt->lat, opt->lon, 98.5, &place_holder, &three_stars);

  // sha'a zmanit according to the GR"A, in seconds
  sun_hour = (sunset - sunrise) / 12;
  // sha'a zmanit according to the Magen Avraham, in seconds
  ma_sun_hour = (first_stars - first_light)/12;
  //
  midday = (sunset + sunrise) / 2;


  // print astronomical times
  if (opt->first_light) print_astronomical_time_tabular( first_light, opt);
  if (opt->talit) print_astronomical_time_tabular( talit, opt);
  if (opt->sunrise) print_astronomical_time_tabular( sunrise, opt);


  // FIXME - these times will be moved into the libhdate api
  // sof zman kriyat Shema (verify that the Magen Avraham calculation is correct!)
  if (opt->magen_avraham) print_astronomical_time_tabular( first_light + (3 * ((first_stars - first_light)/12) ), opt);
  if (opt->shema) print_astronomical_time_tabular( sunrise + (3 * sun_hour), opt);
  // sof zman tefilah
  if (opt->amidah) print_astronomical_time_tabular( sunrise + (4 * sun_hour), opt);

  // This next snippet is a duplication from procedure print_tiems()
  //
  // TODO - erev pesach times (don't forget to also do tabular header line!)
  // TODO - if an erev pesach time was explicitly requested (ie. NOT by -t option),
  //    for a day that is not erev pesach, give the requested information, for the
  //    coming erev pesach, along with a (detailed) message  explaining that the
  //    requested day was not erev pesach. Also give the gregorian date of that
  //    erev pesach if the user wanted gregorian output.
  //    - however, if the explicit erev pesach time request was made with a command
  //    line specifying a month range that includes erev pesach, or a year range,
  //    then just use the code as is now (ie. ignore on days that are not erev pesach
  if ( (h->hd_mon = 7) && (h->hd_day == 14) )
  {
    //  print_astronomical_time( "Magen Avraham sun hour", ma_sun_hour, 0);
    if (opt->end_eating_chometz_ma)  print_astronomical_time_tabular( first_light + (4 * ma_sun_hour), opt);
    if (opt->end_eating_chometz_gra) print_astronomical_time_tabular( sunrise + (4 * sun_hour), opt);
    if (opt->end_owning_chometz_ma)  print_astronomical_time_tabular( first_light + (5 * ma_sun_hour), opt);
    if (opt->end_owning_chometz_gra) print_astronomical_time_tabular( sunrise + (5 * sun_hour), opt);
  }
  else
  {
    if (opt->end_eating_chometz_ma)  printf(",--:--");
    if (opt->end_eating_chometz_gra) printf(",--:--");
    if (opt->end_owning_chometz_ma)  printf(",--:--");
    if (opt->end_owning_chometz_gra) printf(",--:--");
  }

  if (opt->midday) print_astronomical_time_tabular( midday, opt);


  // FIXME - these times will be moved into the libhdate api
  // mincha gedola
  // TODO - There are two other shitot for this:
  //             shaot zmaniot, and shaot zmaniot lechumra
  if (opt->mincha_gedola) print_astronomical_time_tabular( midday + 30, opt);
  if (opt->mincha_ketana) print_astronomical_time_tabular( sunrise + (9.5 * sun_hour), opt);
  if (opt->plag_hamincha) print_astronomical_time_tabular( sunrise + (10.75 * sun_hour), opt);

  if (opt->candles)
  {
    if ( (h->hd_dw != 6) && !opt->only_if_parasha ) candles_time = -1;
    else
    {
      // FIXME - allow for further minhag variation
      if (opt->candles != 1) candles_time = sunset - opt->candles;
      else candles_time = sunset - (DEFAULT_CANDLES_MINUTES * 60);
    }
    print_astronomical_time_tabular( candles_time, opt);
  }

  if (opt->sunset) print_astronomical_time_tabular( sunset, opt);
  if (opt->first_stars) print_astronomical_time_tabular( first_stars, opt);
  if (opt->three_stars) print_astronomical_time_tabular( three_stars, opt);
  if (opt->havdalah)
  {
    if ( (h->hd_dw != 7)  && !opt->only_if_parasha ) havdalah_time = -1;
    else
    {
      // FIXME - allow for further minhag variation
      if (opt->havdalah != 1) havdalah_time = sunset + opt->havdalah;
      else havdalah_time = sunset + (DEFAULT_MOTZASH_MINUTES * 60);
    }
    print_astronomical_time_tabular( havdalah_time, opt);
  }
  if (opt->sun_hour) printf(",%02d:%02d:%02d", sun_hour/3600, (sun_hour%3600)/60, sun_hour%60 );

  /************************************************************
  * end - print times of day
  ************************************************************/

  // necessary only for printing month or year
  opt->epoch_today = opt->epoch_today + SECONDS_PER_DAY;


  if (opt->daf_yomi) print_daf_yomi(h->hd_jd, opt->hebrew, opt->bidi, TRUE, opt->data_first);

  if (opt->omer)
  {
    omer_day = hdate_get_omer_day(h);
    if (omer_day != 0) printf (",%d", omer_day);
    else printf(",");
  }

  if (opt->parasha)
  {
    if (parasha)
    {
      if (!opt->bidi) printf (",%s",
            hdate_string( HDATE_STRING_PARASHA, parasha, opt->short_format, opt->hebrew));
      else
      {
        hebrew_buffer_len =  asprintf (&hebrew_buffer, "%s",
            hdate_string( HDATE_STRING_PARASHA, parasha, opt->short_format, opt->hebrew));
        revstr(hebrew_buffer, hebrew_buffer_len);
        printf (",%s", hebrew_buffer);
        free(hebrew_buffer);
      }
    }
    else printf(",");
  }

  if (opt->holidays)
  {
    if (holiday)
    {
      if (!opt->bidi)
        printf(",%s", hdate_string( HDATE_STRING_HOLIDAY, holiday, opt->short_format, opt->hebrew));
      else
      {
        hebrew_buffer_len =  asprintf (&hebrew_buffer, "%s",
                  hdate_string( HDATE_STRING_HOLIDAY, holiday, opt->short_format, opt->hebrew));
        revstr(hebrew_buffer, hebrew_buffer_len);
        printf (",%s", hebrew_buffer);
        free(hebrew_buffer);
      }
    }
    else printf (",");

    // TODO - I could shrink the jdn_list array upon finding a match. This would require a
    //        a second array with the index into the custom_day text string array, or having
    //        the jdn_list array in pairs (jdn & 'index into custom_day text string array'
    if (opt->custom_days_cnt)
    {
      int i;
      int* jdn_list_ptr = opt->jdn_list_ptr;
      for (i=0; i<opt->custom_days_cnt; i++)
      {
        if (h->hd_jd == *jdn_list_ptr)
        {
          if (!opt->bidi)
            printf(";%s", get_custom_day_text_ptr(i, opt->string_list_ptr));
          else
          {
            hebrew_buffer_len =  asprintf (&hebrew_buffer, "%s",
                      get_custom_day_text_ptr(i, opt->string_list_ptr));
            revstr(hebrew_buffer, hebrew_buffer_len);
            printf (";%s", hebrew_buffer);
            free(hebrew_buffer);
          }
        }
        else jdn_list_ptr = jdn_list_ptr + 1;
      }
    }
  }

  printf("\n");

  if ( opt->print_tomorrow && data_printed && !opt->quiet ) print_alert_sunset();

  return 0;
}




/************************************************************
* print one day - regular output
************************************************************/
int print_day (hdate_struct * h, option_list* opt)
{

  time_t t;
  hdate_struct tomorrow;
  char *hebrew_buffer = NULL;    // for bidi (revstr)
  size_t hebrew_buffer_len = 0;  // for bidi (revstr)

  int iCal_uid_counter = 0;
  int data_printed = 0;
  int parasha = 0;
  int holiday = 0;

  /************************************************************
  * options -R, -H are restrictive filters, so if there is no
  * parasha reading / holiday, print nothing.
  ************************************************************/
  // parasha = hdate_get_parasha (h, opt->diaspora);
  // holiday = hdate_get_holyday (h, opt->diaspora);
  // if (opt->only_if_parasha && opt->only_if_holiday && !parasha && !holiday)
  //   return 0;
  // if (opt->only_if_parasha && !opt->only_if_holiday && !parasha)
  //   return 0;
  // if (opt->only_if_holiday && !opt->only_if_parasha && !holiday)
  //   return 0;
  if ( opt->parasha || opt->only_if_parasha)
    parasha = hdate_get_parasha (h, opt->diaspora);

  if ( opt->holidays || opt->only_if_holiday )
    holiday = hdate_get_halachic_day (h, opt->diaspora);

  if ( (opt->only_if_parasha && opt->only_if_holiday && !parasha && !holiday)  || // eg. Shabbat Chanukah
     (opt->only_if_parasha && !opt->only_if_holiday && !parasha)        || // eg. regular Shabbat
     (opt->only_if_holiday && !opt->only_if_parasha && !holiday)        )  // eg. Holidays
    return 0;
  // TODO - decide how to handle custom_days in the context of
  //        opt->only_if_holiday. Possibly add options: custom_day
  //        only_if_custom_day, and suppress_custom_day

  // TODO - possibly add options: custom_day, suppress_custom_day

  /************************************************************
  * print the iCal event header
  ************************************************************/
  if (opt->iCal)
  {
    printf ("BEGIN:VEVENT\n");
    printf ("UID:hdate-%ld-%d\n", time(&t), ++iCal_uid_counter);
    printf ("DTSTART;VALUE=DATE:%04d%02d%02d\n", h->gd_year,
        h->gd_mon, h->gd_day);
    printf ("SUMMARY:");
  }


  /************************************************************
  * print the Julian Day Number
  ************************************************************/
  if (opt->julian) printf ("JDN-%d ", h->hd_jd);


  /************************************************************
  * print the date
  ************************************************************/
  if (opt->print_tomorrow)  hdate_set_jd (&tomorrow, (h->hd_jd)+1);
  if (opt->quiet < QUIET_HEBREW) print_date (h, &tomorrow, opt);



  /************************************************************
  * begin - print additional information for day
  ************************************************************/
  if (opt->print_tomorrow) *h = tomorrow;
  // FIXME - now that we have opt.time_option_requested, use it
  data_printed = data_printed | print_times (h, opt);

  // necessary only for printing month or year
  opt->epoch_today = opt->epoch_today + SECONDS_PER_DAY;

  if (opt->holidays)
  {
    if (holiday)
    {
      if (opt->quiet < QUIET_DESCRIPTIONS) printf ("%s: ", holiday_text);
      if (!opt->bidi)
        printf ("%s\n",  hdate_string( HDATE_STRING_HOLIDAY, holiday, opt->short_format, opt->hebrew));
      else
      {
        hebrew_buffer_len =  asprintf (&hebrew_buffer, "%s",
                  hdate_string( HDATE_STRING_HOLIDAY, holiday, opt->short_format, opt->hebrew));
        revstr(hebrew_buffer, hebrew_buffer_len);
        printf ("%s\n", hebrew_buffer);
        free(hebrew_buffer);
      }
      data_printed = DATA_WAS_PRINTED;
    }
    if (opt->custom_days_cnt)
    {
      int i;
      int* jdn_list_ptr = opt->jdn_list_ptr;
      for (i=0; i<opt->custom_days_cnt; i++)
      {
        if (h->hd_jd == *jdn_list_ptr)
        {
          if (opt->quiet < QUIET_DESCRIPTIONS) printf ("%s: ", custom_day_text);
          if (!opt->bidi)
            printf("%s\n", get_custom_day_text_ptr(i, opt->string_list_ptr));
          else
          {
            hebrew_buffer_len =  asprintf (&hebrew_buffer, "%s",
                      get_custom_day_text_ptr(i, opt->string_list_ptr));
            revstr(hebrew_buffer, hebrew_buffer_len);
            printf ("%s\n", hebrew_buffer);
            free(hebrew_buffer);
          }
          data_printed = DATA_WAS_PRINTED;
        }
        jdn_list_ptr = jdn_list_ptr + 1;
      }
    }
  }
  if (opt->omer) data_printed = data_printed | print_omer (h, opt);
  if (opt->parasha && parasha)
  {
    if ((opt->quiet < QUIET_DESCRIPTIONS) && (!opt->data_first)) printf ("%s: ", parasha_text);
    if (!opt->bidi) printf ("%s",
          hdate_string( HDATE_STRING_PARASHA, parasha, opt->short_format, opt->hebrew));
    else
    {
      hebrew_buffer_len =  asprintf (&hebrew_buffer, "%s",
          hdate_string( HDATE_STRING_PARASHA, parasha, opt->short_format, opt->hebrew));
      revstr(hebrew_buffer, hebrew_buffer_len);
      printf ("%s", hebrew_buffer);
      free(hebrew_buffer);
    }
    if ((opt->quiet < QUIET_DESCRIPTIONS) && (opt->data_first)) printf (" %s", parasha_text);
    printf("\n");
    data_printed = DATA_WAS_PRINTED;
  }
  if (opt->daf_yomi) data_printed = data_printed | print_daf_yomi(h->hd_jd, opt->hebrew, opt->bidi, FALSE, opt->data_first);

  if ( opt->print_tomorrow  && data_printed && !opt->quiet ) print_alert_sunset();

  /************************************************************
  * end - print additional information for day
  ************************************************************/




  /************************************************************
  * print the iCal event footer
  ************************************************************/
  if (opt->iCal)
  {
    printf ("\nCLASS:PUBLIC\n");
    printf ("DTEND;VALUE=DATE:%04d%02d%02d\n", h->gd_year,
        h->gd_mon, h->gd_day);
    printf ("CATEGORIES:Holidays\n");
    printf ("END:VEVENT\n");
  }
  else printf("\n");

  return 0;
}



/************************************************************
* print one Gregorian month - tabular output
************************************************************/
int print_gmonth_tabular( option_list* opt, const int month, const int year)
{
  hdate_struct h;
  int jd;

  // get date of month start
  hdate_set_gdate (&h, 1, month, year);
  jd = h.hd_jd;

  // print month days
  while (h.gd_mon == month)
  {
    print_day_tabular (&h, opt);
    jd++;
    hdate_set_jd (&h, jd);
  }

  return 0;
}



/************************************************************
* print one Gregorian month - regular output
************************************************************/
int print_gmonth ( option_list* opt, int month, int year)
{
  hdate_struct h;
  int jd;

  // get date of month start
  hdate_set_gdate (&h, 1, month, year);
  jd = h.hd_jd;

  // print month header
  if (!opt->iCal && !opt->short_format)
    printf ("\n%s:\n",
      hdate_string( HDATE_STRING_GMONTH, h.gd_mon, opt->short_format, HDATE_STRING_LOCAL));

  // print month days
  while (h.gd_mon == month)
  {
    print_day (&h, opt);
    jd++;
    hdate_set_jd (&h, jd);
  }

  return 0;
}



/************************************************************
* print one Hebrew month - tabular output *
************************************************************/
int print_hmonth_tabular
  (  option_list* opt, const int month, const int year)
{
  hdate_struct h;
  int jd;

  // get date of month start
  hdate_set_hdate (&h, 1, month, year);
  jd = h.hd_jd;

  // print month days
  while (h.hd_mon == month)
  {
    print_day_tabular (&h, opt);
    jd++;
    hdate_set_jd (&h, jd);
  }

  return 0;
}


/************************************************************
* print one Hebrew month - regular output
************************************************************/
int print_hmonth (hdate_struct * h, option_list* opt,
          const int month)
{
  int jd;
  int bidi_buffer_len;
  char* bidi_buffer;

  // get date of month start
  jd = h->hd_jd;

  // print month header
  if (!opt->iCal && !opt->short_format)
  {
    if (opt->bidi)
    {
      bidi_buffer_len = asprintf(&bidi_buffer, "%s",
        hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew));
      revstr(bidi_buffer, bidi_buffer_len);
      if (bidi_buffer_len != -1)
      {
        printf("\n%s:\n", bidi_buffer);
        if (bidi_buffer != NULL) free(bidi_buffer);
      }
    }
    else printf ("\n%s:\n",
      hdate_string( HDATE_STRING_HMONTH , h->hd_mon, opt->short_format, opt->hebrew));
  }

  // print month days
  while (h->hd_mon == month)
  {
    print_day (h, opt);

    jd++;
    hdate_set_jd (h, jd);
  }
  return 0;
}



/************************************************************
* print one Gregorian year - tabular output *
************************************************************/
int print_gyear_tabular
  ( option_list* opt, const int year)
{
  int month = 1;
  while (month < 13)
  {
    print_gmonth_tabular(opt, month, year);
    month++;
  }
  return 0;
}


/************************************************************
* print one Hebrew year - tabular output *
************************************************************/
int print_hyear_tabular
  ( option_list* opt, const int year)
{
  hdate_struct h;
  int month = 1;

  // print year months
  while (month < 13)
  {
    // get date of month start
    hdate_set_hdate (&h, 1, month, year);

    // if leap year, print both Adar months
    if (h.hd_size_of_year > 365 && month == 6)
    {
      hdate_set_hdate (&h, 1, 13, year);
      print_hmonth_tabular(opt, 13, year);
      hdate_set_hdate (&h, 1, 14, year);
      print_hmonth_tabular(opt, 14, year);
    }
    else
    {
      print_hmonth_tabular(opt, month, year);
    }
    month++;
  }
  return 0;
}



/************************************************************
* print one Gregorian year - regular output
************************************************************/
int print_gyear ( option_list* opt, const int year)
{

  int month = 1;

  // print year header
  if (!opt->iCal && !opt->short_format)
    printf ("%d:\n", year);

  // print year months
  while (month < 13)
  {
    print_gmonth ( opt, month, year);
    month++;
  }

  return 0;
}



/************************************************************
* print one hebrew year - regular output
************************************************************/
int print_hyear ( option_list* opt, const int year)
{
  hdate_struct h;
  int month = 1;

  char *h_int_str;
  h_int_str = hdate_string(HDATE_STRING_INT, year,HDATE_STRING_LONG,opt->hebrew);
  // FIXME - error check for NULL return value

  // print year header
  if (!opt->iCal && !opt->short_format)  printf ("%s:\n", h_int_str);

  // print year months
  while (month < 13)
  {
    // get date of month start
    hdate_set_hdate (&h, 1, month, year);

    // if leap year, print both Adar months
    if (h.hd_size_of_year > 365 && month == 6)
    {
      hdate_set_hdate (&h, 1, 13, year);
      print_hmonth (&h, opt, 13);
      hdate_set_hdate (&h, 1, 14, year);
      print_hmonth (&h, opt, 14);
    }
    else
    {
      print_hmonth (&h, opt, month);
    }
    month++;
  }

  return 0;
}


/****************************************************
* parse config file
****************************************************/
void parse_config_file( option_list *opt )
{
  FILE  *config_file = NULL;
  double tz_lat = BAD_COORDINATE;
  double tz_lon = BAD_COORDINATE;
  size_t input_str_len;  // unnecessary to initialize, per man(3) getline
  char  *input_key;      // unnecessary to initialize, per man(3) sscanf
  char  *input_value;    // unnecessary to initialize, per man(3) sscanf
  char  *input_string = NULL;
  int    menu_item = 0;
  size_t  menu_len = 0;
  int    line_count = 0;
  int    match_count;
  int    end_of_input_file = FALSE;
  int    key_index = 0;
  int    temp_base_year = 0;
  const char*  key_list[] = {
    "SUNSET_AWARE",    // 0
    "LATITUDE",
    "LONGITUDE",    // 2
    "TIMEZONE",
    "DIASPORA",      // 4
    "FORCE_ISRAEL",
    "PARASHA_NAMES",  // 6
    "SHABBAT_INFO",
    "FORCE_HEBREW",    // 8
    "OUTPUT_BIDI",
    "QUIET_ALERTS",    //10
    "YOM",
    "LESHABBAT",    //12
    "LESEDER",
    "TABULAR",      //14
    "ICAL",
    "SEFIRAT_HAOMER",  //16
    "SHORT_FORMAT",
    "TIMES_OF_DAY",    //18
    "SUN_RISE_SET",
    "ONLY_IF_PARASHA_IS_READ",
    "ONLY_IF_HOLIDAY",
    "JULIAN_DAY",    //22
    "CANDLE_LIGHTING",
    "HAVDALAH",      //24
    "MENU",
    "PREFER_HEBREW",  //26
    "BASE_YEAR_HEBREW",
    "BASE_YEAR_GREGORIAN"//28
    };
  const int  num_of_keys = sizeof(key_list) / sizeof(char*);
	static const char* config_dir_name  = "/hcal";
  static const char* config_file_name = "/hcalrc_v1.8";
  static const char* hdate_config_file_text = N_("\
# configuration file for hdate - Hebrew date information program\n\
# part of package libhdate\n\
#\n# Should you mangle this file and wish to restore its default content,\n\
# rename or delete this file and run hdate; hdate will automatically\n\
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
#\n\
# Location awareness\n\
# hdate wants to accurately highlight the current Hebrew day, including\n\
# during the hours between sunset and secular midnight. If you don't\n\
# provide it with latitude, longitude, and time zone information, hdate\n\
# will try to guess the information, based upon your system timezone,\n\
# and its (limited, and maybe biased) of the dominant Jewish community\n\
# in that timezone. When hdate is forced to guess, it alerts the user\n\
# with a message that includes the guessed location.\n\
# hdate's guesses will also affect its default behaviour for ouput of\n\
# Shabbat times, parshiot, and choice of Israel/diaspora hoidays.\n\
#SUNSET_AWARE=TRUE\n\n\
# Base Years\n\
# hdate, by default, interprets two-digit year parameters as Hebrew\n\
# years, and adds the value BASE_YEAR_HEBREW * 100 to them. Set\n\
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
# Output in hebrew characters\n\
# hdate defaults to output all information in your default language, so\n\
# if your default language is Hebrew, you're all set. Otherwise, you can\n\
# set FORCE_HEBREW to true to output Hebrew information in Hebrew, while\n\
# still outputting gregorian information in your default language. To\n\
# output ALL information in Hebrew, run something like this:\n\
#    LC_TEMP=LC_ALL; LC_ALL=\"he_IL.UTF-8\"; hdate; LC_ALL=$LC_TEMP\n\
# If setting FORCE_HEBREW to true results in 'garbage' or non-Hebrew\n\
# output, you need to install a terminal font that includes the Hebrew\n\
# character set (hint: unicode).\n\
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
#OUTPUT_BIDI=FALSE\n\n\
# The Hebrew language output of Hebrew information can also be 'styled'\n\
# in the following ways:\n\
# option YOM ~> yom shishi, aleph tishrei ...\n\
# option LESHABBAT ~> yom sheni leshabbat miketz, kof kislev ...\n\
# option LESEDER ~> yom sheni leseder miketz, kof kislev ...\n\
#YOM=FALSE\n\
#LESHABBAT=FALSE\n\
#LESEDER=FALSE\n\
\n\
# This value directs times-of-day to be printed beginning with sunset\n\
# or candle-lighting of the Hebrew date, instead of beginning with\n\
# the first time-of-day in the morning.\n\
#EMESH=TRUE\n\
#SUN_RISE_SET=FALSE\n\
#TIMES_OF_DAY=FALSE\n\
#SHORT_FORMAT=FALSE\n\
#SEFIRAT_HAOMER=FALSE\n\
#DIASPORA=FALSE\n\
\n\
\n\
# Shabbat related\n\
# Setting SHABBAT_INFO true will output parshiot and Shabbat times.\n\
# The command line options for these features are -r (--parasha), and\n\
# -c. The CANDLE_LIGHTING field can accept a value of 18 - 90 (minutes\n\
# before sunset). The HAVDALAH field can accept a value of 20 - 90\n\
# (minutes after sunset).\n\
#PARASHA_NAMES=FALSE\n\
#ONLY_IF_PARASHA_IS_READ=FALSE\n\
#SHABBAT_INFO=FALSE\n\n\
#CANDLE_LIGHTING=FALSE\n\
#HAVDALAH=FALSE\n\n\
# Holiday related\n\
#HOLIDAYS=FALSE\n\
#ONLY_IF_HOLIDAY=FALSE\n\n\
# Tabular output\n\
# This option has hdate output the information you request in a single\n\
# comma-delimited line per day, suitable for piping or import to\n\
# spreadsheet formatting applications, etc. To belabor the obvious,\n\
# try running -   ./hdate 12 2011 -Rt --table |column -s, -t \n\
# The command line option for this feature is, um, --table\n\
#TABULAR=FALSE\n\n\
# iCal format\n\
# hdate can output its information in iCal-compatible format\n\
# ICAL=FALSE\n\
# Suppress alerts and warnings\n\
# hdate alerts the user via STDERR when it guesses the user's location.\n\
#QUIET_ALERTS=FALSE\n\n\
# Julian day number\n\
# The Julian day number is a .... See ... for more details.\n\
# setting the option JULIAN_DAY will have hdate output that number in\n\
# the format JDN-nnnnnnn at the beginning of its first line of output.\n\
#JULIAN_DAY=FALSE\n\n\
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
#MENU= -bd -l -23.55 -L -46.61 -z -3 # parents in Sao Paolo\n\
#MENU= -b -l 32 -L 34 -z 2           # son in bnei brak\n\
#MENU= -bd -l 43.71 -L -79.43 -z -5  # me in Toronto\n\
#MENU= -bd -l 22.26 -L 114.15 -z 8   # supplier in Hong Kong\n\
");

//  TODO - parse these!
//    opt.emesh

  if (! get_config_file( config_dir_name,
												 config_file_name,
												 hdate_config_file_text,
                         opt->quiet,
												 &config_file))
    return;

  while ( end_of_input_file != TRUE )
  {
    end_of_input_file = getline(&input_string, &input_str_len, config_file);
    if ( end_of_input_file != TRUE )
    {
      errno = 0;
      // BUG - FIXME please
      // The '200' in the next statement is a bug; it is meant to
      // be the value of input_str_len. It would be convenient to
      // use the 'a' conversion specifier here; however, the man
      // pages say that it's a non-standard extension specific to
      // GNU. Let's play with the 'm' conversion specifier which
      // on the one hand, is POSIX and GNU compliant, but on the
      // other hand only for glibc 2.7+ and POSIX.1+
//      match_count = sscanf(input_string,"%[A-Z_]=%200[^\n]",input_key,input_value);
      match_count = sscanf(input_string,"%m[A-Z_]=%m[^\n]",&input_key,&input_value);
      line_count++;
      if (errno != 0) error(0,errno,"scan error at line %d", line_count);
      if (match_count ==2)
      {
        for (key_index=0; key_index<num_of_keys; key_index++)
        {
          if (strcasecmp(input_key, key_list[key_index]) == 0)
          {
            switch(key_index)
            {

//    SUNSET_AWARE
    case  0:if      (strcasecmp(input_value,"FALSE") == 0) opt->not_sunset_aware = 1;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->not_sunset_aware = 0;
        break;

//    LATITUDE
    case  1:
        parse_coordinate(1, input_value, &opt->lat);
        break;

//    LONGITUDE
    case  2:
        parse_coordinate(2, input_value, &opt->lon);
        break;

//    TIMEZONE
    case  3:
        if  (!parse_timezone_numeric(input_value, &opt->tz_offset))
        {
          if (parse_timezone_alpha(input_value, &opt->tz_name_str, &opt->tz_offset, &tz_lat, &tz_lon))
          {
            // TODO - really, at this point, shouldn't either both be bad or botha be good?
            if (opt->lat  == BAD_COORDINATE) opt->lat = tz_lat;
            if (opt->lon == BAD_COORDINATE) opt->lon = tz_lon;
          }
        }
        break;

//    DIASPORA
    case  4:if      (strcasecmp(input_value,"FALSE") == 0) opt->diaspora = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->diaspora = 1;
        break;


//    case  5:if      (strcasecmp(input_value,"FALSE") == 0) opt->force_israel = 0;
//        else if (strcasecmp(input_value,"TRUE") == 0) opt->force_israel = 1;
        break;

//    PARASHA_NAMES
    case  6:if      (strcasecmp(input_value,"FALSE") == 0) opt->parasha = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->parasha = 1;
        break;

//    SHABBAT_INFO
     case  7:if      (strcasecmp(input_value,"FALSE") == 0)
        {
          // maybe not do anything if set FALSE
          opt->candles = 0;
          opt->havdalah = 0;
          opt->parasha = 0;
        }
        else if (strcasecmp(input_value,"TRUE") == 0)
        {
          opt->candles = 1;
          opt->havdalah = 1;
          opt->parasha = 1;
        }
        break;

//    FORCE_HEBREW
    case  8:if      (strcasecmp(input_value,"FALSE") == 0) opt->hebrew = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->hebrew = 1;
        break;

//    OUTPUT_BIDI
    case  9:if      (strcasecmp(input_value,"FALSE") == 0) opt->bidi = 0;
        else if (strcasecmp(input_value,"TRUE") == 0)
            {
              opt->bidi = 1;
              opt->hebrew = 1;
            }
        break;

//    QUIET_ALERTS
    case 10:if      (strcasecmp(input_value,"FALSE") == 0) opt->quiet = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->quiet = 1;
        break;

//    YOM
    case 11:if      (strcasecmp(input_value,"FALSE") == 0) opt->yom = 0;
        else if (strcasecmp(input_value,"TRUE") == 0)
            {
              opt->yom = 1;
              opt->hebrew = 1;
            }
        break;

//    LESHABBAT
    case 12:if      (strcasecmp(input_value,"FALSE") == 0) opt->leShabbat = 0;
        else if (strcasecmp(input_value,"TRUE") == 0)
            {
              opt->leShabbat = 1;
              opt->yom = 1;
              opt->hebrew = 1;
            }
        break;

//    LESEDER
    case 13:if      (strcasecmp(input_value,"FALSE") == 0) opt->leSeder = 0;
        else if (strcasecmp(input_value,"TRUE") == 0)
            {
              opt->leSeder = 1;
              opt->yom = 1;
              opt->hebrew = 1;
            }
        break;

//    TABULAR
    case 14:if      (strcasecmp(input_value,"FALSE") == 0) opt->tablular_output = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->tablular_output = 1;
        break;

//    ICAL
    case 15:if      (strcasecmp(input_value,"FALSE") == 0) opt->iCal = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->iCal = 1;
        break;

//    SEFIRAT_HAOMER
    case 16:if      (strcasecmp(input_value,"FALSE") == 0) opt->omer = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->omer = 1;
        break;

//    SHORT_FORMAT
    case 17:if      (strcasecmp(input_value,"FALSE") == 0) opt->short_format = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->short_format = 1;
        break;

//    TIMES_OF_DAY
    case 18:if      (strcasecmp(input_value,"FALSE") == 0) opt->times = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->times = 1;
        break;

//    SUN_RISE_SET
    case 19:if      (strcasecmp(input_value,"FALSE") == 0)
        {
              opt->sunrise = 0;
              opt->sunset = 0;
        }
        else if (strcasecmp(input_value,"TRUE") == 0)
        {
              opt->sunrise = 1;
              opt->sunset = 1;
        }
        break;

//    ONLY_IF_PARASHA_IS_READ
    case 20:if      (strcasecmp(input_value,"FALSE") == 0) opt->only_if_parasha = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->only_if_parasha = 1;
        break;

//    ONLY_IF_HOLIDAY
    case 21:if      (strcasecmp(input_value,"FALSE") == 0) opt->only_if_holiday = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->only_if_holiday = 1;
        break;

//    JULIAN_DAY
    case 22:if      (strcasecmp(input_value,"FALSE") == 0) opt->julian = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->julian = 1;
        break;

//    CANDLE_LIGHTING
    case 23:if      (strcasecmp(input_value,"FALSE") == 0) opt->candles = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->candles = 1;
        else if (fnmatch( "[[:digit:]]?([[:digit:]])", input_value, FNM_EXTMATCH) == 0)
        {
          opt->candles = atoi(input_value);
          if (opt->candles < MIN_CANDLES_MINUTES) opt->candles = MIN_CANDLES_MINUTES;
          else if (opt->candles > MAX_CANDLES_MINUTES) opt->candles = MAX_CANDLES_MINUTES;
        }
        break;

//    HAVDALAH
    case 24:if      (strcasecmp(input_value,"FALSE") == 0) opt->havdalah = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->havdalah = 1;
        else if (fnmatch( "[[:digit:]]?([[:digit:]])", input_value, FNM_EXTMATCH) == 0)
        {
          opt->havdalah = atoi(input_value);
          if (opt->havdalah < MIN_MOTZASH_MINUTES) opt->havdalah = MIN_MOTZASH_MINUTES;
          else if (opt->havdalah > MAX_MOTZASH_MINUTES) opt->havdalah = MAX_MOTZASH_MINUTES;
        }
        break;

//    MENU
    case 25:if (menu_item < MAX_MENU_ITEMS)
        {
          menu_len = strlen(input_value);
          opt->menu_item[menu_item] = malloc(menu_len+1);
          memcpy(opt->menu_item[menu_item], input_value,menu_len);
          menu_item++;
        }
        break;
//    PREFER_HEBREW
    case 26:if      (strcasecmp(input_value,"FALSE") == 0) opt->prefer_hebrew = 0;
        else if (strcasecmp(input_value,"TRUE") == 0) opt->prefer_hebrew = 1;
        break;
//    BASE_YEAR_HEBREW
    case 27:if (fnmatch( "[3456][[:digit:]]", input_value, FNM_EXTMATCH) == 0)
        {
          temp_base_year = atoi(input_value);
          if ( (temp_base_year < (HDATE_HEB_YR_LOWER_BOUND/100)) ||
             (temp_base_year > (HDATE_HEB_YR_UPPER_BOUND/100)) )
             opt->base_year_h = HDATE_DEFAULT_BASE_YEAR_H;
          else opt->base_year_h = temp_base_year * 100;
        }
        break;
//    BASE_YEAR_GREGORIAN
    case 28:if (fnmatch( "[12][[:digit:]]", input_value, FNM_EXTMATCH) == 0)
        {
          temp_base_year = atoi(input_value);
          if ( (temp_base_year < (HDATE_GREG_YR_LOWER_BOUND/100)) ||
             (temp_base_year > (HDATE_GREG_YR_UPPER_BOUND/100)) )
             opt->base_year_g = HDATE_DEFAULT_BASE_YEAR_G;
          else opt->base_year_g = temp_base_year * 100;
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
//  free(input_key);
//  free(input_value);
  fclose(config_file);
  return;
}


/****************************************************
* exit elegantly
****************************************************/
void exit_main( option_list *opt, int exit_code)
{
  int i;
  for (i=0; i<MAX_MENU_ITEMS; i++)
  {
    if (opt->menu_item[i] == NULL) break;
    free(opt->menu_item[i]);
  }
  exit (exit_code);
}



/****************************************************
* parse a command-line or a config-file menu line
*
* It was appropriate to make this a function, outside
* of main, because of its dual use and dual reference
****************************************************/
int parameter_parser( int switch_arg, option_list *opt,
            int long_option_index)
{
  static char *timezone_text  = N_("-z (timezone)");
  double tz_lat = BAD_COORDINATE;
  // double tz_lon = BAD_COORDINATE;
  int error_detected = 0;

  switch (switch_arg)
  {

  case 0: // long options
    switch (long_option_index)
    /*****************************************************
    * cases of THIS switch should be blank if the final
    * field of structure "option long_options[]", defined
    * in main() is not zero, ie. there is a short option,
    * ie. the option will be processed in the short
    * option switch, below.
    *****************************************************/
    {
/** --version    */  case 0:  print_version (); exit_main(opt,0); break;
/** --help      */  case 1:  print_help (); exit_main(opt,0); break;
/** --hebrew    */  case 2: opt->hebrew = 1; break;
/** --yom      */  case 3:
                opt->yom = 1;
                opt->hebrew = 1;
                break;
/** --leshabbat    */  case 4:
                opt->leShabbat = 1;
                opt->yom = 1;
                opt->hebrew = 1;
                break;
/** --leseder    */  case 5:
                opt->leSeder = 1;
                opt->yom = 1;
                opt->hebrew = 1;
                break;
/** --table      */  case 6: break;
/** --not-sunset-aware */case 7:  opt->not_sunset_aware = 1; break;
/** --quiet      */  case 8:  break;
/** --short-format  */  case 9:  break;
/** --parasha    */  case 10:break;
/** --holidays    */  case 11:break;
/** --shabbat-times  */  case 12:break;
/** --sun      */  case 13:opt->sunset = 1;
                opt->sunrise = 1;
                opt->time_option_requested = TRUE;
                break;
/** --sunset    */  case 14:opt->sunset = 1;
                opt->time_option_requested = TRUE;
                break;
/** --sunrise    */  case 15:opt->sunrise = 1;
                opt->time_option_requested = TRUE;
                break;
/** --candle-lighting */case 16:
/** --candles    */  case 17:
      if (optarg == NULL) opt->candles = 1;
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
          parm_error("--candles"); // do not gettext!
          error_detected++;
          break;
        }
      }
      opt->time_option_requested = TRUE;
      break;
/** --havdalah    */  case 18:
      if (optarg == NULL) opt->havdalah = 1;
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
          parm_error("--havdalah"); // do not gettext!
          error_detected++;
          break;
        }
      }
      opt->time_option_requested = TRUE;
      break;
/** --latitude        */  case 19: /** short opt 'l' */ break;
/** --longitude        */  case 20: /** short opt 'L' */ break;
/** --timezone        */  case 21: /** short opt 'z' */ break;
/** --bidi          */  case 22: /** short opt 'b' */
/** --visual        */  case 23: /** short opt 'b' */ break;
/** --omer          */  case 24: /** short opt 'o' */ break;
/** --ical          */  case 25: /** short opt 'i' */ break;
/** --julian        */  case 26: /** short opt 'j' */ break;
/** --diaspora        */  case 27: /** short opt 'd' */ break;
/** --menu          */  case 28: /** short opt 'e' */ break;
/** --alot          */  case 29:
/** --first-light      */  case 30: opt->first_light = 1; opt->time_option_requested = TRUE; break;
/** --talit          */  case 31: opt->talit = 1; opt->time_option_requested = TRUE; break;
/** --netz          */  case 32: opt->sunrise = 1; opt->time_option_requested = TRUE; break;
/** --shema          */  case 33: opt->shema = 1; opt->time_option_requested = TRUE; break;
/** --amidah        */  case 34: opt->amidah = 1; opt->time_option_requested = TRUE; break;
/** --midday        */  case 35:
/** --noon          */  case 36:
/** --chatzot        */  case 37: opt->midday = 1; opt->time_option_requested = TRUE; break;
/** --mincha-gedola      */  case 38: opt->mincha_gedola = 1; opt->time_option_requested = TRUE; break;
/** --mincha-ketana      */  case 39: opt->mincha_ketana = 1; opt->time_option_requested = TRUE; break;
/** --plag-hamincha      */  case 40: opt->plag_hamincha = 1; opt->time_option_requested = TRUE; break;
/** --shekia        */  case 41: opt->sunset = 1; opt->time_option_requested = TRUE; break;
/** --tzeit          */  case 42:
/** --first-stars      */  case 43: opt->first_stars = 1; opt->time_option_requested = TRUE; break;
/** --three-stars      */  case 44: opt->three_stars = 1; opt->time_option_requested = TRUE; break;
/** --magen-avraham      */  case 45: opt->magen_avraham = 1; opt->time_option_requested = TRUE; break;
/** --sun-hour        */  case 46: opt->sun_hour = 1; break;
/** --daf-yomi        */  case 47: opt->daf_yomi = 1; break;
/** --tabular        */  case 48: /** short opt 'T' */ break;
/** --end-eating-chometz-ma  */  case 49: opt->end_eating_chometz_ma = 1; opt->time_option_requested = TRUE; break;
/** --end-eating-chometz-gra*/  case 50: opt->end_eating_chometz_gra = 1; opt->time_option_requested = TRUE; break;
/** --end-owning-chometz-ma  */  case 51: opt->end_owning_chometz_ma = 1; opt->time_option_requested = TRUE; break;
/** --end-owning-chometz-gra*/  case 52: opt->end_owning_chometz_gra = 1; opt->time_option_requested = TRUE; break;
/** --erev-pesach           */   case 53: opt->end_eating_chometz_ma = 1;
                     opt->end_eating_chometz_gra = 1;
                     opt->end_owning_chometz_ma = 1;
                     opt->end_owning_chometz_gra = 1;
                     opt->time_option_requested = TRUE;
                     break;
/** --times-of-day      */  case 54: /** short opt 't' */ break;
/** --day-times        */  case 55: /** short opt 't' */ break;
/** --israel        */  case 56: opt->diaspora = 0;   break;
/** --la-omer        */  case 57: opt->la_omer = 1; break;
/** --ba-omer        */  case 58: break;
/** --quiet-alerts      */  case 59: if (opt->quiet < QUIET_ALERTS) opt->quiet = QUIET_ALERTS; break;

/** --quiet-gregorian    */  case 60: if (opt->quiet < QUIET_GREGORIAN) opt->quiet = QUIET_GREGORIAN; break;
/** --quiet-descriptions  */  case 61: if (opt->quiet < QUIET_DESCRIPTIONS) opt->quiet = QUIET_DESCRIPTIONS; break;
/** --quiet-hebrew      */  case 62: if (opt->quiet < QUIET_HEBREW) opt->quiet = QUIET_HEBREW;
                     opt->hebrew = 1; break;
/** --data-first      */  case 63: opt->data_first = TRUE; break;
/** --labels-first      */  case 64: opt->data_first = FALSE; break;
/** --prefer-hebrew      */  case 65: opt->prefer_hebrew = TRUE; break;
/** --prefer-gregorian    */  case 66: opt->prefer_hebrew = FALSE; break;
/** --emesh          */  case 67:
/** --erev          */  case 68: /** short opt 'e' */ break;
/** --no-emesh        */  case 69:
/** --no-erev        */  case 70: opt->emesh = FALSE;  break;
/** --epoch                 */  case 71: /** short opt 'E' */ break;
/** --usage                 */  case 72: /** short opt '?' */ break;
    } // end switch for long_options
    break;


  case 'b': opt->bidi = 1; opt->hebrew = 1; break;
  case 'd': opt->diaspora = 1; break;
  case 'e': opt->emesh = TRUE; break;
  case 'E': if (optarg == NULL) opt->print_epoch = TRUE;
        else error_detected = error_detected +
              parse_epoch_value( optarg, &opt->epoch_today, &opt->epoch_parm_received );
        break;
  case 'H': opt->only_if_holiday = 1; // There is no break here, because -H implies -h
  case 'h': opt->holidays = 1; break;
  case 'i': opt->iCal = 1; break;
  case 'j': opt->julian = 1; break;
  case 'q': if (opt->quiet < 4) opt->quiet = opt->quiet + 1;
        break;
  case 'm': opt->menu = 1; break;
  case 'o': if (opt->omer < 3) opt->omer = opt->omer + 1;
        break;
  case 'R': opt->only_if_parasha = 1; // There is no break here, because -R implies -r
  case 'r': opt->parasha = 1; break;
  case 'S': opt->short_format = 1; break;
//  case 's': opt->sunrise = 1; opt->sunset = 1; break;
  case 's': // candles & havdalah may have custom minhag in minutes
        if (opt->candles == 0) opt->candles = 1;
        if (opt->havdalah == 0) opt->havdalah = 1;
        opt->time_option_requested = TRUE;
        break;
  case 't': if (opt->times < 3) opt->times = opt->times + 1;
        opt->time_option_requested = TRUE;
        break;
  case 'T': opt->tablular_output = 1; break;
  case 'l': error_detected = error_detected + parse_coordinate(1, optarg, &opt->lat);
        break;
  case 'L': error_detected = error_detected + parse_coordinate(2, optarg, &opt->lon);
        break;
  case 'v': if (opt->afikomen < 9) opt->afikomen = opt->afikomen + 1;
        break;
  case 'z':
    /***************************************************************
    *
    *      Explanation of timezone options and
    *      the relationship between timeone and
    *      longitude.
    *
    *   Timezones aggregate a set of longitudes so that they
    *   share a single time. and there has developed an
    *   expectation that certain times will generally correlate
    *   to certain points in the astronomical day. For example,
    *   we expect sunrise to correlate to about 6 - 7 AM. For
    *   a planet's rotation of 360 degrees, a sensible division
    *   into 24 timezones one hour apart would be 360/24 = 15
    *   degrees. In practice, however, politiciams are always
    *   playing with the timezone definitions, so hdate responds
    *   in the following way to user input:
    *
    *   TIMEZONE INPUT
    *   1] numeric - treat the value as an absolute adjustment
    *                to UTC, over-riding the system time zone and
    *                daylight savings time, and print an alert
    *                that there will be no DST awareness. If the
    *                tz value greatly varies from the user-input
    *                longitude, just print an alert. If no
    *                longitude or latitude is entered, guess.
    *   2] name    - eg. "Asia/Jerusalem", also absolutely
    *                over-rides system timezone.
    *                * longitude mis-sync -> just alert
    *                * latitude mis-sync (re:zone.tab) - just alert
    *                * no longitude -> use zone.tab entry
    *   3] null    - use system timezone. Often, this data
    *                will be linked to /etc/localtime
    *                * longitude mis-sync -> just alert
    *                * latitude mis-sync (re:zone.tab) - just alert
    *                * no longitude -> use zone.tab entry
    *
    ***************************************************************/
    if  ( (!optarg)
        || ( (optarg[0] == '-')
               && (strspn(&optarg[1], "0123456789.:") == 0) ) )
    {
      error_detected = error_detected + 1;
      parm_missing_error(timezone_text);
    }
    else if (!parse_timezone_numeric(optarg, &opt->tz_offset))
    {
      if (!parse_timezone_alpha(optarg, &opt->tz_name_str, &opt->tz_offset, &tz_lat, &opt->tz_lon))
      {
        error_detected = error_detected + 1;
        parm_error(timezone_text);
      }
      else
      {
        if (opt->lat  == BAD_COORDINATE) opt->lat = tz_lat;
        if (opt->lon == BAD_COORDINATE) opt->lon = opt->tz_lon;
      }
    }
    break;
  case '?':
    if (( optopt != '?') && (long_option_index != 72) ) option_unknown_error ( (char*) &optopt );
  default:
    print_usage_hdate();
    print_try_help_hdate();
    exit_main(opt,0);
    break;
  }

  return error_detected;
}


/************************************************************
* getopt options constants
*
*   These are used in TWO places: parsing of the command line, and
*   parsing of menu items as defined in the config files
*
************************************************************/
// support for getopt short options
const char * short_options = "bcdeE::hHjimoqrRsStTvl:L:z:";
// support for getopt long options
const struct option long_options[] = {
//       name,  has_arg, flag, val
/**  0 */{"version", no_argument, 0, 0},
/**  1 */{"help", no_argument, 0, 0},
/**  2 */{"hebrew", no_argument, 0, 0},
/**  3 */{"yom", no_argument, 0, 0},
/**  4 */{"leshabbat", no_argument, 0, 0},
/**  5 */{"leseder", no_argument, 0, 0},
/**  6 */{"table",no_argument,0,'T'},
/**  7 */{"not-sunset-aware",no_argument,0,0},
/**  8 */{"quiet",no_argument,0,'q'},
/**  9 */{"short_format",no_argument,0,'S'},
/** 10 */{"parasha",no_argument,0,'r'},
/** 11 */{"holidays",no_argument,0,'h'},
/** 12 */{"shabbat-times",no_argument,0,'s'},
/** 13 */{"sun",no_argument,0,0},
/** 14 */{"sunset",no_argument,0,0},
/** 15 */{"sunrise",no_argument,0,0},
/** 16 */{"candle-lighting",optional_argument,0,0},
/** 17 */{"candles", optional_argument,0,0},
/** 18 */{"havdalah", optional_argument,0,0},
/** 19 */{"latitude", required_argument, 0, 'l'},
/** 20 */{"longitude", required_argument, 0, 'L'},
/** 21 */{"timezone", required_argument, 0, 'z'},
/** 22 */{"bidi", no_argument, 0, 'b'},
/** 23 */{"visual", no_argument, 0, 'b'},
/** 24 */{"omer", no_argument, 0, 'o'},
/** 25 */{"ical", no_argument, 0, 'i'},
/** 26 */{"julian", no_argument, 0, 'j'},
/** 27 */{"diaspora", no_argument, 0, 'd'},
/** 28 */{"menu",no_argument,0,'m'},
/** 29 */{"alot", no_argument,0,0},
/** 30 */{"first-light", no_argument,0,0},
/** 31 */{"talit", no_argument,0,0},
/** 32 */{"netz", no_argument,0,0},
/** 33 */{"shema", no_argument,0,0},
/** 34 */{"amidah", no_argument,0,0},
/** 35 */{"midday", no_argument,0,0},
/** 36 */{"noon", no_argument,0,0},
/** 37 */{"chatzot", no_argument,0,0},
/** 38 */{"mincha-gedola", no_argument,0,0},
/** 39 */{"mincha-ketana", no_argument,0,0},
/** 40 */{"plag-hamincha", no_argument,0,0},
/** 41 */{"shekia", no_argument,0,0},
/** 42 */{"tzeit-hakochavim", no_argument,0,0},
/** 43 */{"first-stars", no_argument,0,0},
/** 44 */{"three-stars", no_argument,0,0},
/** 45 */{"magen-avraham", no_argument,0,0},
/** 46 */{"sun-hour", no_argument,0,0},
/** 47 */{"daf-yomi", no_argument,0,0},
/** 48 */{"tabular", no_argument,0,'T'},
/** 49 */{"end-eating-chometz-ma", no_argument,0,0},
/** 50 */{"end-eating-chometz-gra", no_argument,0,0},
/** 51 */{"end-owning-chometz-ma", no_argument,0,0},
/** 52 */{"end-owning-chometz-gra", no_argument,0,0},
/** 53 */{"erev-pesach", no_argument,0,0},
/** 54 */{"times-of-day", no_argument,0,'t'},
/** 55 */{"day-times", no_argument,0,'t'},
/** 56 */{"israel", no_argument,0,0},
/** 57 */{"la-omer", no_argument,0,0},
/** 58 */{"ba-omer", no_argument,0,0},
/** 59 */{"quiet-alerts", no_argument,0,0},
/** 60 */{"quiet-gregorian", no_argument,0,0},
/** 61 */{"quiet-descriptions", no_argument,0,0},
/** 62 */{"quiet-hebrew", no_argument,0,0},
/** 63 */{"data-first", no_argument,0,0},
/** 64 */{"labels-first", no_argument,0,0},
/** 65 */{"prefer-hebrew", no_argument,0,0},
/** 66 */{"prefer-gregorian", no_argument,0,0},
/** 67 */{"emesh", no_argument,0,'e'},
/** 68 */{"erev", no_argument,0,'e'},
/** 69 */{"no-emesh", no_argument,0,0},
/** 70 */{"no-erev", no_argument,0,0},
/** 71 */{"epoch",optional_argument,0,'E'},
/** 72 */{"usage", no_argument, 0, '?'},
/** eof*/{0, 0, 0, 0}
  };




/************************************************************
* parse command line
************************************************************/
void parse_command_line (
       int argc,
	  	 char* argv[],
       option_list* opt,
	  	 int* error_detected )
{
  int switch_arg = -1;
  int long_option_index = 0;
  opterr = 0; // We'll do our own error reporting. See getopt_long(3)
  while ((switch_arg = getopt_long(argc,
																			argv,
                                      short_options,
																			long_options,
                                      &long_option_index)) != -1)
    error_detected = error_detected
		               + parameter_parser(switch_arg,
																			opt,
																			long_option_index);
}


/**************************************************
* parse user menu selection
*************************************************/
void parse_user_menu_selection( option_list* opt, int* error_detected )
{
	int response = menu_select( &opt->menu_item[0], MAX_MENU_ITEMS );
  if (response == -1) exit_main(opt, 0);
  else if ((response < MAX_MENU_ITEMS) && (opt->menu_item[response] != NULL))
  {
    char* menuptr = opt->menu_item[response];
    size_t menu_len = strlen( menuptr );
    int menu_index = 0;
    char *optptr = NULL;
    int long_option_index = 0;
    int switch_arg = -1;
    while (( switch_arg = menu_item_parse( menuptr, menu_len, &menu_index,
              &optptr, short_options, long_options,
              &long_option_index, error_detected) ) != -1)
    {
      error_detected = error_detected +
        parameter_parser(switch_arg, opt, long_option_index);
    }
  }
  opt->afikomen = 0;  // only allow this option on the command line
}


/**************************************************
* initialize an option_list data structure
*************************************************/
void initialize_option_list_struct( option_list* opt )
{
  opt->prefer_hebrew = TRUE;
  opt->base_year_h = HDATE_DEFAULT_BASE_YEAR_H;    // TODO - Make this user-selectable from command line
  opt->base_year_g = HDATE_DEFAULT_BASE_YEAR_G;    // TODO - Make this user-selectable from command line
  opt->lat = BAD_COORDINATE;
  opt->lon = BAD_COORDINATE;
  opt->tz_name_str = NULL;
  // opt->tz_lat = BAD_COORDINATE;
  opt->tz_lon = BAD_COORDINATE;
  opt->tz_offset = BAD_TIMEZONE;
  opt->tzif_entries =0;
  opt->tzif_data = NULL;
  opt->tzif_index = 0;
  opt->print_tomorrow = 0;    // TRUE if currently after sunset
  opt->time_option_requested = FALSE;
  opt->epoch_start= 0;      // for checking dst transitions
  opt->epoch_end  = 0;      // for checking dst transitions
  opt->epoch_today  = 0;    // for checking dst transitions
  opt->epoch_parm_received = 0;
  opt->hebrew = 0;
  opt->bidi = 0;
  opt->yom = 0;
  opt->leShabbat = 0;
  opt->leSeder = 0;
  opt->tablular_output = 0;
  opt->not_sunset_aware = 0;
  opt->quiet = 0;
  opt->first_light = 0;
  opt->talit = 0;
  opt->sunrise = 0;
  opt->magen_avraham = 0;
  opt->shema = 0;
  opt->amidah = 0;
  opt->midday = 0;
  opt->mincha_gedola = 0;
  opt->mincha_ketana = 0;
  opt->plag_hamincha = 0;
  opt->sunset = 0;
  opt->first_stars = 0;
  opt->three_stars = 0;
  opt->sun_hour = 0;
  opt->candles = 0;
  opt->havdalah = 0;
  opt->times = 0;        // -t option print times of day
  opt->short_format = 0;    // -S Short format flag
  opt->holidays = 0;      // -h option holidays
  opt->only_if_holiday = 0;  // -H option just holidays
  opt->parasha = 0;      // -r option reading
  opt->only_if_parasha = 0;  // -R option just reading
  opt->julian = 0;        // -j option Julian day number
  opt->diaspora = -1;      // -d option diaspora, if not explicitly set by user or config file
                // to 0/1 will be set based on timezone/location awareness guess.
  opt->iCal = 0;        // -i option iCal
  opt->daf_yomi = 0;
  opt->omer = 0;        // -o option Sfirat Haomer
  opt->la_omer = 0;      // use lame instead of bet
  opt->emesh = TRUE;        // whether times begin at prior sunset or AM
  opt->menu = 0;        // -m print menus for user-selection
  opt->afikomen = 0;      // Hebrew 'easter egg' (lehavdil)
  opt->end_eating_chometz_ma = 0;
  opt->end_eating_chometz_gra = 0;
  opt->end_owning_chometz_ma = 0;
  opt->end_owning_chometz_gra = 0;
  opt->data_first = TRUE;
  opt->print_epoch = FALSE;
  opt->custom_days_cnt = 0;
  opt->jdn_list_ptr = NULL;  // for custom_days
  opt->string_list_ptr= NULL;  // for custom_days
  // TODO - be sure to free() opt->jdn_list_ptr, opt->string_list_ptr upon exit
  for (int i=0; i<MAX_MENU_ITEMS; i++) opt->menu_item[i] = NULL;
}



/************************************************************
 *
 *
 *
 *                         M A I N
 *
 *
 *  -- refer to support.h for macros used here
 *
************************************************************/
int main (int argc, char *argv[])
{
  struct tm    epoch_tm;
  hdate_struct h_start_day;
  hdate_struct h_day_after_final_day;
  hdate_struct h_month_to_print;
  int day   = BAD_DATE_VALUE;  // user-input (Hebrew or gregorian)
  int month = BAD_DATE_VALUE;  // user-input (Hebrew or gregorian)
  int year  = BAD_DATE_VALUE;  // user-input (Hebrew or gregorian)
  const char* digits = "0123456789"; // for checking a parm as numeric
  char* tz_name_verified = NULL;
  int error_detected = FALSE;    // exit after reporting ALL bad parms
  FILE *custom_file = NULL;
  int custom_days_file_ready = FALSE;

  option_list opt;
  initialize_option_list_struct( &opt );

  //  TODO - verify that I'm not needlessly setting locale repeatedly
  setlocale (LC_ALL, ""); // ensure wide-character functions use utf8 (?)
  parse_config_file( &opt );
  parse_command_line( argc, argv, &opt, &error_detected );

  // undocumented feature - afikomen
  if (opt.afikomen)
  {
    printf("%s\n", afikomen[opt.afikomen-1]);
    exit(0);
  }

  if (opt.menu)
		parse_user_menu_selection( &opt, &error_detected );


  // We get custom days list even if the user didn't
  // request holidays, so that if the file doesn't
  // exist, we can create it
  custom_days_file_ready = get_custom_days_file( "/hdate", "/custom_days_v1.8",
                opt.tz_name_str, opt.quiet,
                &custom_file);
  if (!opt.holidays) fclose(custom_file);


  #define PROCESS_NOTHING     0
  #define PROCESS_TODAY       1
  #define PROCESS_HEBREW_DAY  2
  #define PROCESS_GREGOR_DAY  3
  #define PROCESS_JULIAN_DAY  4
  #define PROCESS_MONTH       5
  #define PROCESS_HEBREW_YEAR 6
  #define PROCESS_GREGOR_YEAR 7
  #define PROCESS_EPOCH_DAY  8
  int hdate_action = PROCESS_NOTHING;

  // if user input --epoch=@nnnnnn, ignore date-spec
  if (opt.epoch_parm_received)
  {
    if (argc != optind)
    {
      if (!opt.quiet) error(0,0,"%s",N_("parameter conflict: you entered both a date_spec and an option --epoch date_spec"));
      exit_main(&opt, EXIT_CODE_BAD_PARMS); // need to work on exit codes !
    }
    hdate_action = PROCESS_EPOCH_DAY;
  }
  else if (argc == optind)
  {
    hdate_set_gdate (&h_start_day, 0, 0, 0);
    hdate_action = PROCESS_TODAY;
  }
  else if (argc == (optind + 1))
  {
    if (*argv[optind] == '@')
    {
      if ( parse_epoch_value( argv[optind], &opt.epoch_today, &opt.epoch_parm_received ) == 0)
      {
        // opt.epoch_start = opt.epoch_today;
        // opt.epoch_end = opt.epoch_today + SECONDS_PER_DAY;
        hdate_action = PROCESS_EPOCH_DAY;
      }
    }
    else if ( strspn(argv[optind], digits) == strlen(argv[optind]) )
    {
      year = atoi (argv[optind]);
      if ( (year >= HDATE_JUL_DY_LOWER_BOUND) && (year <= HDATE_JUL_DY_UPPER_BOUND) )
      {
        hdate_set_jd (&h_start_day, year);
        hdate_action = PROCESS_JULIAN_DAY;
      }
      else
      {
        if (!hdate_parse_date( argv[optind], "", "", &year, &month, &day, 1,
                 opt.prefer_hebrew, HDATE_PREFER_MD,
                 opt.base_year_h, opt.base_year_g ))
          exit_main(&opt,0);

        if (day != 0)
        {
          if (month > 100)
          {
            hdate_action = PROCESS_HEBREW_DAY;
            hdate_set_hdate (&h_start_day, day, month-100, year);
          }
          else
          {
            hdate_action = PROCESS_GREGOR_DAY;
            hdate_set_gdate (&h_start_day, day, month, year);
          }
        }
        else if ((year >= HDATE_HEB_YR_LOWER_BOUND) && (year <= HDATE_HEB_YR_UPPER_BOUND))
        {
          hdate_action = PROCESS_HEBREW_YEAR;
          hdate_set_hdate (&h_start_day, 1, 1, year);
        }
        else if ((year >= HDATE_GREG_YR_LOWER_BOUND) && (year <= HDATE_GREG_YR_UPPER_BOUND))
        {
          hdate_action = PROCESS_GREGOR_YEAR;
          hdate_set_gdate (&h_start_day, 1, 1, year);
        }
      }
    }
    else // possibly month name
    {
      if (!hdate_parse_date( argv[optind], "", "",
               &year, &month, &day, 2,
               opt.prefer_hebrew, HDATE_PREFER_MD,
               opt.base_year_h, opt.base_year_g ))
        exit_main(&opt,0);
      hdate_action = PROCESS_MONTH;
    }
  }
  else if (argc == (optind + 2))
  {

    if (!hdate_parse_date( argv[optind], argv[optind+1],
             "", &year, &month, &day, 2,
             opt.prefer_hebrew, HDATE_PREFER_MD,
             opt.base_year_h, opt.base_year_g ))
      exit_main(&opt,0);
    if (!day) hdate_action = PROCESS_MONTH;
    else
    {
      if (month > 100)
      {
        hdate_set_hdate (&h_start_day, day, month-100, year);
        hdate_action = PROCESS_HEBREW_DAY;
      }
      else
      {
        hdate_set_gdate (&h_start_day, day, month, year);
        hdate_action = PROCESS_GREGOR_DAY;
      }
    }
  }
  else if (argc == (optind + 3))
  {
    if (!hdate_parse_date( argv[optind], argv[optind+1],
             argv[optind+2], &year, &month, &day, 3,
             opt.prefer_hebrew, HDATE_PREFER_MD,
             opt.base_year_h, opt.base_year_g ))
      exit_main(&opt,0);
    if (year <= 0) { parm_error(year_text); exit_main(&opt,0); }
    if (year > HDATE_HEB_YR_LOWER_BOUND)
    {
      // The parse_date function returns Hebrew month values in
      // the range 101 - 114
      if (month > 100) month = month - 100;

      // bounds check for month
      if (!validate_hdate(CHECK_MONTH_PARM, 0, month, year, FALSE, &h_start_day))
        { parm_error(month_text); exit_main(&opt,0); }

      // bounds check for day
      if (!validate_hdate(CHECK_DAY_PARM, day, month, year, TRUE, &h_start_day))
        { parm_error(day_text); exit_main(&opt,0); }

      hdate_set_hdate (&h_start_day, day, month, year);
      hdate_action = PROCESS_HEBREW_DAY;
    }
    else
    {
      // bounds check for month
      if (!validate_hdate(CHECK_MONTH_PARM, 0, month, year, FALSE, &h_start_day))
        { parm_error(month_text); exit_main(&opt,0); }

      // bounds check for day
      if (!validate_hdate(CHECK_DAY_PARM, day, month, year, TRUE, &h_start_day))
        { parm_error(day_text); exit_main(&opt,0); }

      hdate_set_gdate (&h_start_day, day, month, year);
      hdate_action = PROCESS_GREGOR_DAY;
    }

  }
  else
  {
    error(0,0,"%s", N_("too many arguments (expected at most day, month and year after options list)"));
    exit_main(&opt,0);
  }


  // remember to free() tz_name_input
  // if user input tz as a value, it is absolute (no dst)
  // else try reading tzif /etc/localtime
  // exit after reporting all bad parameters found */
  if (error_detected)
  {
    print_usage_hdate();
    print_try_help_hdate();
    exit_main(&opt, EXIT_CODE_BAD_PARMS);
  }


  // diaspora-awareness
  //  TODO - this needs to get more sophisticated in step with
  //  increased sophistication in location and timezone awareness.
  if (opt.diaspora == -1)
  {
    if (opt.tz_offset == 2) opt.diaspora = 0;
    else opt.diaspora = 1;
  }


  /************************************************************
  * option "t" has three verbosity levels
  ************************************************************/
  if (opt.times)
  {
    opt.first_light = 1;
    opt.talit = 1;
    opt.sunrise = 1;
    opt.midday = 1;
    opt.sunset = 1;
    opt.first_stars = 1;
    opt.three_stars = 1;
    opt.sun_hour = 1;

    if (opt.times > 1)
    {
      opt.shema = 1;
      opt.amidah = 1;
      opt.mincha_gedola = 1;
      opt.mincha_ketana = 1;
      opt.plag_hamincha = 1;

      if (opt.times > 2)
      {
        opt.magen_avraham = 1;
        if (opt.times > 3)
        {
          opt.end_eating_chometz_ma = 1;
          opt.end_eating_chometz_gra = 1;
          opt.end_owning_chometz_ma = 1;
          opt.end_owning_chometz_gra = 1;
        }
      }
    }
  }

  switch (hdate_action)
  {
  case PROCESS_NOTHING    :
  case PROCESS_EPOCH_DAY  : break;
  case PROCESS_TODAY      :
  case PROCESS_HEBREW_DAY :
  case PROCESS_GREGOR_DAY :
  case PROCESS_JULIAN_DAY :
      // why aren't we calling hdate_set_gdate( &h_start_day ...
      hdate_set_jd ( &h_day_after_final_day, h_start_day.hd_jd + 1 );
      break;
  case PROCESS_MONTH      :
      if (month > 100)
      {
        hdate_set_hdate (&h_start_day, 1, month-100, year);
        hdate_set_jd ( &h_day_after_final_day,
          h_start_day.hd_jd + hdate_get_size_of_hebrew_month( month, h_start_day.hd_year_type )  );
      }
      else
      {
        hdate_set_gdate (&h_start_day, 1, month, year);
        hdate_set_gdate (&h_day_after_final_day, 1, (month==12)?1:month+1, (month==12)?year+1:year);
      }
      break;
  case PROCESS_HEBREW_YEAR:
      hdate_set_hdate (&h_start_day, 1, 1, year);
      hdate_set_hdate (&h_day_after_final_day, 1, 1, year+1);
      break;
  case PROCESS_GREGOR_YEAR:
      hdate_set_gdate (&h_start_day, 1, 1, year);
      hdate_set_gdate (&h_day_after_final_day, 1, 1, year+1);
      break;
  } // end switch (hdate_action)

  if ( (opt.tzif_data == NULL) && opt.time_option_requested &&
     !opt.epoch_parm_received )
  {
    get_epoch_time_range( &opt.epoch_start,
              &opt.epoch_end,
              opt.tz_name_str,
              opt.tz_offset,
              h_start_day.gd_year,
              h_start_day.gd_mon,
              h_start_day.gd_day,
              h_day_after_final_day.gd_year,
              h_day_after_final_day.gd_mon,
              h_day_after_final_day.gd_day);
    opt.epoch_today = opt.epoch_start;
  }

  process_location_parms(  &opt.lat, &opt.lon, opt.tz_lon,
              &opt.tz_offset,  opt.tz_name_str, &tz_name_verified,
              opt.epoch_start, opt.epoch_end,
              &opt.tzif_entries, &opt.tzif_data,
              opt.quiet);
  // not sure about this next one
  opt.tz_name_str = tz_name_verified;

  if (hdate_action == PROCESS_EPOCH_DAY)
  {
    opt.epoch_start = opt.epoch_today + (60 * get_tz_adjustment(  opt.epoch_today, opt.tz_offset,
                    &opt.tzif_index, opt.tzif_entries, opt.tzif_data ));
  /* temp, delete this! */ printf("epoch value = %ld, epoch start = %ld, adjustment = %ld\n", opt.epoch_today,
                        opt.epoch_start, opt.epoch_start - opt.epoch_today);
    opt.epoch_end = opt.epoch_start + SECONDS_PER_DAY;
    opt.epoch_today = opt.epoch_start;
    gmtime_r( &opt.epoch_today, &epoch_tm );
  /* temp, delete this! */ printf("d=%d, m=%d, y=%d, %02d:%02d\n", epoch_tm.tm_mday, epoch_tm.tm_mon+1,
                        1900+epoch_tm.tm_year, epoch_tm.tm_hour, epoch_tm.tm_min);
    hdate_set_gdate (&h_start_day, epoch_tm.tm_mday, epoch_tm.tm_mon+1, 1900+epoch_tm.tm_year);
    hdate_action = PROCESS_HEBREW_DAY;
  }

  switch (hdate_action)
  {
case PROCESS_JULIAN_DAY:
case PROCESS_HEBREW_DAY:
case PROCESS_GREGOR_DAY:
case PROCESS_TODAY:
    if ( opt.holidays && custom_days_file_ready)
    {
      if (hdate_action == PROCESS_GREGOR_DAY)
      {
        opt.custom_days_cnt = read_custom_days_file(
                  custom_file, &opt.jdn_list_ptr, &opt.string_list_ptr,
                  h_start_day.gd_day, h_start_day.gd_mon, h_start_day.gd_year,
                  'G', h_start_day, opt.short_format, opt.hebrew);
      }
      else
      {
        opt.custom_days_cnt = read_custom_days_file(
                  custom_file, &opt.jdn_list_ptr, &opt.string_list_ptr,
                  h_start_day.hd_day, h_start_day.hd_mon, h_start_day.hd_year,
                  'H', h_start_day, opt.short_format, opt.hebrew);
      }
      fclose(custom_file);
    }
    if (opt.tablular_output)
    {
      print_tabular_header( &opt );
      print_day_tabular(&h_start_day, &opt);
    }
    else
    {
      if (opt.iCal) print_ical_header ();
      // not sure about this next else if clause
      // by the time I had combined all these hdate_action options,
      // the clause was only in option PROCESS_JULIAN_DAY ...
      else if (!opt.not_sunset_aware)
        opt.print_tomorrow = check_for_sunset(&h_start_day, opt.lat, opt.lon, opt.tz_offset);
      print_day (&h_start_day, &opt);
      if (opt.iCal) print_ical_footer ();
    }
    break;
case PROCESS_HEBREW_YEAR:
    if ( opt.holidays && custom_days_file_ready )
    {
// (done above)  hdate_set_hdate(&h_start_day, 1, 1, year);
      opt.custom_days_cnt = read_custom_days_file(
                  custom_file, &opt.jdn_list_ptr, &opt.string_list_ptr,
                  0, 0, year,
                  'H', h_start_day, opt.short_format, opt.hebrew);
      fclose(custom_file);
    }

    if (opt.tablular_output)
    {
      print_tabular_header( &opt );
      print_hyear_tabular( &opt, year);
    }
    else
    {
      if (opt.iCal) print_ical_header ();
      print_hyear ( &opt, year);
      if (opt.iCal) print_ical_footer ();
    }
    break;
case PROCESS_GREGOR_YEAR:
    if ( opt.holidays && custom_days_file_ready )
    {
// (done above)  hdate_set_gdate(&h_start_day, 1, 1, year);
      opt.custom_days_cnt = read_custom_days_file(
                  custom_file, &opt.jdn_list_ptr, &opt.string_list_ptr,
                  0, 0, year,
                  'G', h_start_day, opt.short_format, opt.hebrew);
      fclose(custom_file);
    }

    if (opt.tablular_output)
    {
      print_tabular_header( &opt );
      print_gyear_tabular( &opt, year);
    }
    else
    {
      if (opt.iCal) print_ical_header ();
      print_gyear ( &opt, year);
      if (opt.iCal) print_ical_footer ();
    }
    break;
case PROCESS_MONTH:
    /************************************************************
    * process entire Hebrew month
    ************************************************************/
    if (year >= HDATE_HEB_YR_LOWER_BOUND)
    {
      // The parse_date function returns Hebrew month values in
      // the range 101 - 114
      if (month > 100) month = month - 100;


      if ( opt.holidays && custom_days_file_ready )
      {
        // hdate_set_hdate(&h, 1, month, year);
        opt.custom_days_cnt = read_custom_days_file(
                  custom_file, &opt.jdn_list_ptr, &opt.string_list_ptr,
                  0, month, year,
                  'H', h_start_day, opt.short_format, opt.hebrew);
        fclose(custom_file);
      }

      if (opt.tablular_output)
      {
        print_tabular_header( &opt );

        // if leap year, print both Adar months
        if (h_start_day.hd_size_of_year > 365 && month == 6)
        {
          print_hmonth_tabular ( &opt, 13, year);
          print_hmonth_tabular ( &opt, 14, year);
        }
        else
        {
          print_hmonth_tabular ( &opt, month, year);
        }
      }
      else
      {
        if (opt.iCal) print_ical_header ();

        // if leap year, print both Adar months
        if (h_start_day.hd_size_of_year > 365 && month == 6)
        {
          hdate_set_hdate (&h_month_to_print, 1, 13, year);
          print_hmonth (&h_month_to_print, &opt, 13);
          hdate_set_hdate (&h_month_to_print, 1, 14, year);
          print_hmonth (&h_month_to_print, &opt, 14);
        }
        else
        {
          print_hmonth (&h_start_day, &opt, month);
        }
        if (opt.iCal)
          print_ical_footer ();
      }
    }

    /************************************************************
    * process entire Gregorian month
    ************************************************************/
    else
    {
      if ( month <= 0 || month > 12 )
        { parm_error(month_text); exit_main(&opt,0); }

      if ( opt.holidays && custom_days_file_ready )
      {
        opt.custom_days_cnt = read_custom_days_file(
                  custom_file, &opt.jdn_list_ptr, &opt.string_list_ptr,
                  0, month, year,
                  'G', h_start_day, opt.short_format, opt.hebrew);
        fclose(custom_file);
      }

      if (opt.tablular_output)
      {
        print_tabular_header( &opt );
        print_gmonth_tabular( &opt, month, year);
      }
      else
      {
        if (opt.iCal) print_ical_header ();
        print_gmonth (&opt, month, year);
        if (opt.iCal) print_ical_footer ();
      }
    }
    break;
  } // end of switch (hdate_action)
  return 0;
}
