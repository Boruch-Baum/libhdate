/* hdate.c
 * Example file for libhdate.
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hdate.c -o hdate
 *
 * Copyright:  2011 (c) Baruch Baum, 2004 (c) Yaacov Zamir
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

#define _GNU_SOURCE		// For mempcpy, asprintf
#include <stdio.h>		// For printf
//include <hdate.h>		// For hebrew date
#include "../../src/hdate.h"
#include <stdlib.h>		// For atoi, getenv, setenv
#include <locale.h>		// For setlocale
#include <getopt.h>		// For getopt_long
#include <string.h>		// For strchr, mempcpy, asprintf

#include "./local_functions.c"

#define DATA_WAS_NOT_PRINTED 0
#define DATA_WAS_PRINTED 1


// for hdate_get_int_string_
#define NO_COMPRESS 0
#define COMPRESS 1

char *debug_var;				// system environment variable

static char * sunrise_text = N_("sunrise");
static char * sunset_text  = N_("sunset");
static char * first_light_text = N_("first_light");
static char * talit_text = N_("talit");
static char * midday_text = N_("midday");
static char * first_stars_text = N_("first_stars");
static char * three_stars_text = N_("three_stars");

typedef struct  {
				int hebrew;
				int bidi;
				int yom;
				int leShabbat;
				int leSeder;
				int tablular_output;
				int not_sunset_aware;
				int print_tomorrow;
				int quiet_alerts;
				int sun;
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
				} option_list;


const char* hdate_config_file_text = "\
# configuration file for hdate - Hebrew date information program\n\
# part of package libhdate\n\
#\n# Should you mangle this file and wish to restore its default content,\n\
# rename or delete this file and run hdate; hdate will automatically\n\
# regenerate the default content.\n#\n\
# Your system administrator can set system-wide defaults for hcal by\n\
# modifying file <not yet implemented>.\n\
# You may override all defaults by changing the contents of this file.\n\
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
# Shabbat times, parshiot, and choice of Israel/diaspora hoidays\n\
#SUNSET_AWARE=TRUE\n\
#LATITUDE=\n\
#LONGITUDE=\n\
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
#SUN=FALSE\n\
#TIMES_OF_DAY=FALSE\n\
#SHORT_FORMAT=FALSE\n\
#SEFIRAT_HAOMER=FALSE\n\
#DIASPORA=FALSE\n\
\n\
\n\
# Shabbat related\n\
# setting SHABBAT_INFO true will output parshiot and Shabbat times\n\
#PARASHA_NAMES=FALSE\n\
#ONLY_IF_PARASHA_IS_READ=FALSE\n\
#SHABBAT_INFO=FALSE\n\n\
#CANDLE_LIGHTING=FALSE\n\
#HAVDALAH=FALSE\n\\n\
# Holiday related\n\
#HOLIDAYS=FALSE\n\
#ONLY_IF_HOLIDAY=FALSE\n\n\
# Tabular output\n\
# This option has hdate output the information you request in a single\n\
# comma-delimited line per day, suitable for piping or import to\n\
# spreadsheet formatting applications, etc. To belabor the obvious,\n\
# try running -   ./hdate 12 2011 -Rt --table |column -s, -t \n\
# The command line option for this feature is, um, --table\n\
#TABULAR=FALSE\n\
# iCal format\n\
# hdate can output its information in iCal-compatable format\n\
# ICAL=FALSE\n\
# Suppress alerts and warnings\n\
# hdate alerts the user via STDERR when it guesses the user's location.\n\
#QUIET_ALERTS=FALSE\n\n\
# Julian day number\n\
# The Julian day number is a .... See ... for more details.\n\
# setting the option JULIAN_DAY will have hdate output that number in\n\
# the format JDN-nnnnnnn at the beginning of its first line of output.\n\
#JULIAN_DAY=FALSE";



/************************************************************
* print version information
************************************************************/
int print_version ()
{
	printf ("hdate - display Hebrew date information\nversion 2\n");
	return 0;
}

/************************************************************
* print usage information
************************************************************/
void print_usage ()
{
	printf ("hdate - display Hebrew date information\n");

	printf ("USAGE: hdate [options] [-l xx[.xxx] -L yy[.yyy] [-z nn] ] [[[day] month] year]\n");
	printf ("       hdate [options] [-l xx[.xxx] -L yy[.yyy] [-z nn] ] [julian_day]\n");
}


/************************************************************
* print help information
************************************************************/
void print_help ()
{
	print_usage();
	
	printf ("OPTIONS:\n\
   -b --bidi          prints hebrew in reverse (visual)\n\
      --visual\n\
   -c                 print Shabbat start/end times.\n\
                      Shabbat starts 20 min before sunset,\n\
                      and ends when three stars are out.\n\
   -d --diaspora      use diaspora reading and holidays.\n\
   -h --holidays      print holidays.\n\
   -H                 print only it is a holiday.\n\
   -i --ical          use iCal formated output.\n\
   -j --julian        print Julian day number.\n\
   -o --omer          print Sefirat Haomer.\n\
   -q --quiet-alerts  suppress warning messages\n\
   -r --parasha       print weekly reading on saturday.\n\
   -R                 print only if there is a weekly reading on Shabbat.\n\
   -s --sun           print sunrise/sunset times.\n\
   -S --short-format  print using short format.\n\
   -t                 print day times: first light, talit, sunrise,\n\
                            midday, sunset, first stars, three stars.\n\
   -T --table         tabular output, suitable for spreadsheets\n\n\
   -z --timezone nn   timezone, +/-UTC\n\
   -l --latitude xx   latitude xx degrees. Negative values are South\n\
   -L --longitude yy  longitude yy degrees. Negative values are West\n\n\
   --hebrew           forces Hebrew to print in Hebrew characters\n\
   --yom              force Hebrew prefix to Hebrew day of week\n\
   --leshabbat        insert parasha between day of week and day\n\
   --leseder          insert parasha between day of week and day\n\
   --not-sunset-aware don't display next day if after sunset\n");
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
void print_astronomical_time( char *description, int timeval, int tz)
{
	if (timeval < 0) printf("%s: --:--\n", description);
	else printf("%s: %02d:%02d\n", description,
		(timeval+(tz*60)) / 60, (timeval+(tz*60)) % 60 );
	return;
}

/************************************************************
* generic print astronomical time for tablular output
************************************************************/
void print_astronomical_time_tabular( int timeval, int tz)
{
	if (timeval < 0) printf(",--:--");
	else printf(",%02d:%02d",
			(timeval+(tz*60)) / 60, (timeval+(tz*60)) % 60 );
	return;
}




/************************************************************
* print iCal header
************************************************************/
int print_ical_header ()
{
	/* Print start of iCal format */
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
	/* Print end of iCal format */
	printf ("END:VCALENDAR\n");

	return 0;
}





/************************************************************
* find Shabbat
*	this function is to become obsolete - I want a feature
*	for this standard in libhdate
************************************************************/
int find_shabbat (hdate_struct * h, int opt_d)
{
	hdate_struct coming_Shabbat;

	hdate_set_jd (&coming_Shabbat, h->hd_jd+(7-h->hd_dw));
	/*	this return value is the reading number,
		used to print parshiot */
	return hdate_get_parasha (&coming_Shabbat, opt_d);
	}



/************************************************************
* print one date - both Gregorian and Hebrew
************************************************************/
int print_date (hdate_struct * h, hdate_struct * tomorrow, option_list opt, double lat, double lon)
{
	char *language;				// system environment variable
	char *locale;				// system environment variable
	char *eve_before    = "";   // prefix if after sunset
	char *for_day_of_g  = "";	// Hebrew prefix for day of week
	char *bet_g 		= "";	// Hebrew prefix for Gregorian month
	char *bet_h         = "";	// Hebrew prefix for Hebrew month
	char *psik_mark     = "";	// --force-yom
	char *for_day_of_h  = "";	// --force-yom
	char *for_week_of   = "";	// --force-leshabbat --force-leseder
	int  is_parasha_read;

	char *hday_int_str, *hyear_int_str;
	hday_int_str = malloc(HEBREW_NUMBER_BUFFER_SIZE);
	hyear_int_str = malloc(HEBREW_NUMBER_BUFFER_SIZE);


#ifdef ENABLE_NLS
	locale = setlocale (LC_MESSAGES, NULL);
	language = getenv ("LANGUAGE");
#else
	locale = NULL;
	language = NULL;
#endif


	/************************************************************
	* preliminary - if it's after sunset, it's tomorrow already
	* but we will only acknowledge this if printing in the long
	* format (which is the default)
	************************************************************/
	if (opt.print_tomorrow)
	{
		if (opt.hebrew)
		{
			eve_before = "אור ל";
			if (!opt.yom) for_day_of_h="-";
		}
		else eve_before = N_("eve of ");
	}



	/************************************************************
	* preliminary - prepare Hebrew prefixes if forcing any Hebrew
	************************************************************/
	language=getenv("LANGUAGE"); // not sure this is really doing anything
	if (opt.hebrew)
	{
		bet_h="ב";
		/* preliminary - prepare Yom prefix */
		if (opt.yom)
		{
			for_day_of_h="יום";
			psik_mark=",";

			if		(opt.leShabbat)
				{
				if (h->hd_dw==7)		for_week_of="פרשת";
				else					for_week_of="לשבת";
				}
			else if (opt.leSeder)	for_week_of="לסדר";
		}
	}

	/************************************************************
	*	preliminary - prepare the Hebrew prefixes
	*	for the Gregorian month and day of week
	************************************************************/
	if (hdate_is_hebrew_locale())
		{ bet_g="ב"; for_day_of_g="יום ";}



	/************************************************************
	* Three major print formats: iCal, short, long
	************************************************************/


	/************************************************************
	* iCal format
	************************************************************/
	if (opt.iCal)
	{
		if (opt.hebrew)
				setenv("LANGUAGE", "he_IL.UTF-8", 1);
		printf ("%s%s %s%s ",
				for_day_of_g,
				hdate_get_int_string_ (hday_int_str, h->hd_day, NO_COMPRESS),
				bet_h,
				hdate_get_hebrew_month_string (h->hd_mon, opt.short_format));
		printf ("%s", hdate_get_int_string_(hyear_int_str, h->hd_year, NO_COMPRESS));
	}


	/************************************************************
	* short format
	************************************************************/
	else if (opt.short_format)
	{
		printf ("%d.%d.%d  ", h->gd_day, h->gd_mon, h->gd_year);


		if (!hdate_is_hebrew_locale() && (!opt.hebrew))
		{
			printf ("%d", h->hd_day);
		}
		else
		{
			setenv("LANGUAGE", "he_IL.UTF-8", 1);
			printf ("%s", hdate_get_int_string_(hday_int_str, h->hd_day, NO_COMPRESS));
		}

		printf (" %s %s\n", hdate_get_hebrew_month_string (h->hd_mon, opt.short_format),
							hdate_get_int_string_(hyear_int_str, h->hd_year, NO_COMPRESS));
	}


	/************************************************************
	* long (normal) format
	************************************************************/
	else
	{
		/************************************************************
		* Gregorian date - the easy part
		************************************************************/
		printf ("%s%s, %d %s%s %d, ",
				for_day_of_g,
				hdate_get_day_string (h->hd_dw, opt.short_format),
				h->gd_day,
				bet_g,
				hdate_get_month_string (h->gd_mon, opt.short_format), h->gd_year);


		/************************************************************
		* Start of the Hebrew date
		************************************************************/
		if (opt.print_tomorrow) *h = *tomorrow;


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
		if (opt.hebrew)
		{
			setenv("LANGUAGE", "he_IL.UTF-8", 1);
			if (opt.yom)
				{
				hebrew_buffer1_len = asprintf(&hebrew_buffer1, "%s%s %s", eve_before,for_day_of_h, hdate_get_day_string (h->hd_dw, opt.short_format));
				if ((opt.leShabbat) || (opt.leSeder))
				{
					is_parasha_read = find_shabbat(h, opt.diaspora);
					if (is_parasha_read)
							hebrew_buffer2_len = asprintf(&hebrew_buffer2, " %s %s", for_week_of, hdate_get_parasha_string (is_parasha_read, opt.diaspora));
				}
				hebrew_buffer3_len = asprintf (&hebrew_buffer3, "%s ", psik_mark);
			}
		}
		else hebrew_buffer1_len = asprintf(&hebrew_buffer1, "%s", eve_before);


		/************************************************************
		* prepare buffers with Hebrew dd mmmm yyyy
		************************************************************/
		hebrew_buffer4_len =
			asprintf (&hebrew_buffer4, "%s %s%s %s",
				hdate_get_int_string_(hday_int_str, h->hd_day, NO_COMPRESS),
				bet_h,
				hdate_get_hebrew_month_string (h->hd_mon, opt.short_format),
				hdate_get_int_string_(hyear_int_str, h->hd_year, NO_COMPRESS));


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
		if (opt.bidi) revstr(hebrew_buffer, hebrew_buffer_len);

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
		free(hebrew_buffer4);
		free(hebrew_buffer);
	}


	/************************************************************
	* CLEANUP - restore default language environment variable *
	************************************************************/
	setenv("LANGUAGE", language, 1); // not sure this is really doing anything

	/************************************************************
	* CLEANUP - free allocated memory
	************************************************************/
	free(hday_int_str);
	free(hyear_int_str);

	return 0;
}



/************************************************************
* option 's' - sunrise/set times
************************************************************/
int
print_sunrise (hdate_struct * h, double lat, double lon, int tz, int opt_i)
{
	int sunrise, sunset;

	/* get times */
	hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
							&sunrise, &sunset);

	print_astronomical_time( sunrise_text, sunrise, tz);
	print_astronomical_time( sunset_text, sunset, tz);

	return DATA_WAS_PRINTED;
}



/************************************************************
* option 't' - day times
************************************************************/
int print_times (hdate_struct * h, double lat, double lon, int tz, int opt_i)
{
	int sun_hour, first_light, talit, sunrise;
	int midday, sunset, first_stars, three_stars;

	/* get times */
	hdate_get_utc_sun_time_full (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
								 &sun_hour, &first_light, &talit, &sunrise,
								 &midday, &sunset, &first_stars, &three_stars);

	print_astronomical_time( first_light_text, first_light, tz);
	print_astronomical_time( talit_text, talit, tz);
	print_astronomical_time( sunrise_text, sunrise, tz);
	print_astronomical_time( midday_text, midday, tz);
	print_astronomical_time( sunset_text, sunset, tz);
	print_astronomical_time( first_stars_text, first_stars, tz);
	print_astronomical_time( three_stars_text, three_stars, tz);
	printf("%s: %02d:%02d\n",N_("sun_hour"), sun_hour/60, sun_hour%60);

	return DATA_WAS_PRINTED;
}



/************************************************************
* option 'h' - holiday identification
************************************************************/
/* int print_holiday (hdate_struct * h, int opt.diaspora, int short_format, int opt_i)
{
	int holiday;

	holiday = hdate_get_holyday (h, opt.diaspora);
	if (holiday)
	{
		printf ("%s\n", hdate_get_holyday_string (holiday, short_format));
		return DATA_WAS_PRINTED;
	}
	else return DATA_WAS_NOT_PRINTED;
}
*/

/************************************************************
* option 'o' - sefirat ha'omer
************************************************************/
int print_omer (hdate_struct * h)
{
	int omer_day;

	omer_day = hdate_get_omer_day(h);
	if (omer_day != 0)
	{
		printf ("%s %s \n", N_("today is day"), hdate_get_omer_string (omer_day));
		return DATA_WAS_PRINTED;
	}
	else return DATA_WAS_NOT_PRINTED;
}



/************************************************************
* option 'r' - parashat hashavua
************************************************************/
/* int print_reading (hdate_struct * h, int opt.diaspora, int short_format, int opt_i)
{
	int reading;

	reading = hdate_get_parasha (h, opt.diaspora);
	if (reading)
	{
		printf ("%s  %s\n", N_("Parashat"), hdate_get_parasha_string (reading, short_format));
		return DATA_WAS_PRINTED;
	}
	else return DATA_WAS_NOT_PRINTED;
}
*/


/************************************************************
* option 'c' - candle-lighting time; tzeit Shabbat
************************************************************/
int print_candles (hdate_struct * h, double lat, double lon, int tz, int opt_i)
{
	int sun_hour, first_light, talit, sunrise;
	int midday, sunset, first_stars, three_stars;

	// check for friday - print knisat shabat
	if (h->hd_dw == 6)
	{
		// get times
		hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
								&sunrise, &sunset);

		// FIXME - allow for minhag variation
		sunset = sunset + tz * 60 - 20;

		// print candlelighting times
		printf ("candle-lighting: %d:%d\n", sunset / 60, sunset % 60);
		return DATA_WAS_PRINTED;
	}

	// check for saturday - print motzay shabat
	else if (h->hd_dw == 7)
	{
		// get times
		hdate_get_utc_sun_time_full (h->gd_day, h->gd_mon, h->gd_year, lat,
									 lon, &sun_hour, &first_light, &talit,
									 &sunrise, &midday, &sunset,
									 &first_stars, &three_stars);

		// FIXME - allow for minhag variation
		three_stars = three_stars + tz * 60;

		// print motzay shabat
		printf ("Shabbat ends: %d:%d\n", three_stars / 60, three_stars % 60);
		return DATA_WAS_PRINTED;
	}
	else return DATA_WAS_NOT_PRINTED;
}

/************************************************************
* print tabular header
************************************************************/
void print_tabular_header( option_list opt)
{
	printf("%s,%s",N_("Gregorian date"), N_("Hebrew Date"));
	if (opt.times) printf(",%s,%s,%s,%s,%s,%s,%s",first_light_text,talit_text,sunrise_text,midday_text,sunset_text,first_stars_text,three_stars_text );
	else if (opt.sun) printf(",%s,%s",sunrise_text,sunset_text);
	if (opt.holidays) printf(",%s",N_("holiday"));
	if (opt.omer) printf(",%s",N_("omer count"));
	if (opt.parasha) printf(",%s",N_("parashat"));
	printf("\n");
return;
}

/************************************************************
* print one day - tabular output *
************************************************************/
int print_tabular_day (hdate_struct * h, option_list opt,
						double lat, double lon, int tz)
{
	int sun_hour, first_light, talit, sunrise;
	int midday, sunset, first_stars, three_stars;

	hdate_struct tomorrow;
	int data_printed = 0;

	int parasha = 0;
	int holiday = 0;
	int omer_day = 0;

	char *h_int_str;
	h_int_str = malloc(HEBREW_NUMBER_BUFFER_SIZE);

	/************************************************************
	* options -R, -H are restrictive filters, so if there is no
	* parasha reading / holiday, print nothing.
	************************************************************/
	parasha = hdate_get_parasha (h, opt.diaspora);
	holiday = hdate_get_holyday (h, opt.diaspora);
	if (opt.only_if_parasha && opt.only_if_holiday && !parasha && !holiday)
		return 0;
	if (opt.only_if_parasha && !opt.only_if_holiday && !parasha)
		return 0;
	if (opt.only_if_holiday && !opt.only_if_parasha && !holiday)
		return 0;



	/************************************************************
	* print Gregorian date
	************************************************************/
	printf ("%d.%d.%d,", h->gd_day, h->gd_mon, h->gd_year);


	/************************************************************
	* begin - print Hebrew date
	************************************************************/
	if (opt.print_tomorrow)	hdate_set_jd (&tomorrow, (h->hd_jd)+1);
	if ((!hdate_is_hebrew_locale()) && (!opt.hebrew))
	{	/* non hebrew numbers */
		printf ("%d", h->hd_day);
	}
	else /* Hebrew */
	{
		setenv("LANGUAGE", "he_IL.UTF-8", 1);
		printf ("%s", hdate_get_int_string_(h_int_str, h->hd_day, NO_COMPRESS));
	}

	printf (" %s ", hdate_get_hebrew_month_string (h->hd_mon, 1));
	printf ("%s", hdate_get_int_string_(h_int_str, h->hd_year, NO_COMPRESS));
	/************************************************************
	* end - print Hebrew date
	************************************************************/


	if (opt.times)
	{
		/* get astronomical times */
		hdate_get_utc_sun_time_full (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
									 &sun_hour, &first_light, &talit, &sunrise,
									 &midday, &sunset, &first_stars, &three_stars);
		/* print astronomical times */
		print_astronomical_time_tabular( first_light, tz);
		print_astronomical_time_tabular( talit, tz);
		print_astronomical_time_tabular( sunrise, tz);
		print_astronomical_time_tabular( midday, tz);
		print_astronomical_time_tabular( sunset, tz);
		print_astronomical_time_tabular( first_stars, tz);
		print_astronomical_time_tabular( three_stars, tz);
		printf(",%02d:%02d",sun_hour/60, sun_hour%60);
	}
	else if (opt.sun)
	{
		hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
								&sunrise, &sunset);
		print_astronomical_time_tabular( sunrise, tz);
		print_astronomical_time_tabular( sunset, tz);
	}
	
	if (opt.holidays && holiday)
		printf (",%s", hdate_get_holyday_string (holiday, opt.short_format));

	if (opt.omer)
	{
		omer_day = hdate_get_omer_day(h);
		if (omer_day != 0) printf (",%s", hdate_get_omer_string (omer_day));
	}

	if (opt.parasha && parasha)
		printf (",%s", hdate_get_parasha_string (parasha, opt.short_format));

	
	printf("\n");

	if ((opt.print_tomorrow) && (data_printed) && (!opt.quiet_alerts)) print_alert_sunset();

	return 0;
}




/************************************************************
* print one day - regular output
************************************************************/
int print_day (hdate_struct * h, option_list opt,
				double lat, double lon, int tz)
{

	time_t t;
	hdate_struct tomorrow;

	int iCal_uid_counter = 0;
	int data_printed = 0;
	int parasha = 0;
	int holiday = 0;
	

	/************************************************************
	* options -R, -H are restrictive filters, so if there is no
	* parasha reading / holiday, print nothing.
	************************************************************/
	parasha = hdate_get_parasha (h, opt.diaspora);
	holiday = hdate_get_holyday (h, opt.diaspora);
	if (opt.only_if_parasha && opt.only_if_holiday && !parasha && !holiday)
		return 0;
	if (opt.only_if_parasha && !opt.only_if_holiday && !parasha)
		return 0;
	if (opt.only_if_holiday && !opt.only_if_parasha && !holiday)
		return 0;


	/************************************************************
	* print the iCal event header
	************************************************************/
	if (opt.iCal)
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
	if (opt.julian) printf ("JDN-%d ", h->hd_jd);


	/************************************************************
	* print the date
	************************************************************/
	if (opt.print_tomorrow)	hdate_set_jd (&tomorrow, (h->hd_jd)+1);
	print_date (h, &tomorrow, opt, lat, lon);



	/************************************************************
	* begin - print additional information for day
	************************************************************/
	if (opt.print_tomorrow) *h = tomorrow;
	if (opt.sun) data_printed = data_printed | print_sunrise (h, lat, lon, tz, opt.iCal);
	if (opt.times) data_printed = data_printed | print_times (h, lat, lon, tz, opt.iCal);
	if (opt.holidays && holiday)
	{
		printf ("%s\n", hdate_get_holyday_string (holiday, opt.short_format));
		data_printed = DATA_WAS_PRINTED;
	}
	if (opt.omer) data_printed = data_printed | print_omer (h);
	if (opt.parasha && parasha)
	{
		printf ("%s  %s\n", N_("Parashat"), hdate_get_parasha_string (parasha, opt.short_format));
		data_printed = DATA_WAS_PRINTED;
	}
	if (opt.candles || opt.havdalah) data_printed = data_printed | print_candles (h, lat, lon, tz, opt.iCal);
	if ((opt.print_tomorrow) && (data_printed) && (!opt.quiet_alerts)) print_alert_sunset();
	/************************************************************
	* end - print additional information for day
	************************************************************/




	/************************************************************
	* print the iCal event footer
	************************************************************/
	if (opt.iCal)
	{
		printf ("\nCLASS:PUBLIC\n");
		printf ("DTEND;VALUE=DATE:%04d%02d%02d\n", h->gd_year,
				h->gd_mon, h->gd_day);
		printf ("CATEGORIES:Holidays\n");
		printf ("END:VEVENT\n");
	}

	return 0;
}



/************************************************************
* print one Gregorian month - tabular output
************************************************************/
int print_tabular_gmonth( option_list opt,
		double lat, double lon, int tz, int month, int year)
{
	hdate_struct h;
	int jd;

	/* get date of month start */
	hdate_set_gdate (&h, 1, month, year);
	jd = h.hd_jd;

	/* print month days */
	while (h.gd_mon == month)
	{
		print_tabular_day (&h, opt, lat, lon, tz);
		jd++;
		hdate_set_jd (&h, jd);
	}

	return 0;
}



/************************************************************
* print one Gregorian month - regular output
************************************************************/
int print_gmonth (option_list opt, double lat, double lon, int tz,
					int month, int year)
{
	hdate_struct h;
	int jd;

	/* get date of month start */
	hdate_set_gdate (&h, 1, month, year);
	jd = h.hd_jd;

	/* print month header */
	if (!opt.iCal && !opt.short_format)
		printf ("\n%s:\n", hdate_get_month_string (h.gd_mon, opt.short_format));

	/* print month days */
	while (h.gd_mon == month)
	{
		print_day (&h, opt, lat, lon, tz);

		jd++;
		hdate_set_jd (&h, jd);
	}

	return 0;
}



/************************************************************
* print one Hebrew month - tabular output *
************************************************************/
int print_tabular_hmonth
	(	option_list opt, double lat, double lon, int tz,
		int month, int year)
{
	hdate_struct h;
	int jd;

	/* get date of month start */
	hdate_set_hdate (&h, 1, month, year);
	jd = h.hd_jd;

	/* print month days */
	while (h.gd_mon == month)
	{
		print_tabular_header( opt );
		print_tabular_day (&h, opt, lat, lon, tz);
		jd++;
		hdate_set_jd (&h, jd);
	}

	return 0;
}


/************************************************************
* print one Hebrew month - regular output
************************************************************/
int print_hmonth (hdate_struct * h, option_list opt,
					double lat, double lon, int tz, int month, int year)
{
	int jd;

	/* get date of month start */
	jd = h->hd_jd;

	/* print month header */
	if (!opt.iCal && !opt.short_format)
		printf ("\n%s:\n", hdate_get_hebrew_month_string (h->hd_mon, opt.short_format));

	/* print month days */
	while (h->hd_mon == month)
	{
		print_day (h, opt, lat, lon, tz);

		jd++;
		hdate_set_jd (h, jd);
	}
	return 0;
}



/************************************************************
* print one Gregorian year - tabular output *
************************************************************/
int print_tabular_gyear
	( option_list opt, double lat, double lon, int tz, int year)
{
	int month = 1;
	while (month < 13)
	{
		print_tabular_gmonth(opt, lat, lon, tz, month, year);
		month++;
	}
	return 0;
}


/************************************************************
* print one Hebrew year - tabular output *
************************************************************/
int print_tabular_hyear
	(	option_list opt, double lat, double lon, int tz, int year)
{
	hdate_struct h;
	int month = 1;

	/* print year months */
	while (month < 13)
	{
		/* get date of month start */
		hdate_set_hdate (&h, 1, month, year);

		/* if leap year, print both Adar months */
		if (h.hd_size_of_year > 365 && month == 6)
		{
			hdate_set_hdate (&h, 1, 13, year);
			print_tabular_hmonth(opt, lat, lon, tz, 13, year);
			hdate_set_hdate (&h, 1, 14, year);
			print_tabular_hmonth(opt, lat, lon, tz, 14, year);
		}
		else
		{
			print_tabular_hmonth(opt, lat, lon, tz, month, year);
		}
		month++;
	}
	return 0;
}



/************************************************************
* print one Gregorian year - regular output
************************************************************/
int print_gyear (option_list opt, double lat, double lon, int tz, int year)
{

	int month = 1;

	/* print year header */
	if (!opt.iCal && !opt.short_format)
		printf ("%d:\n", year);

	/* print year months */
	while (month < 13)
	{
		print_gmonth ( opt, lat, lon, tz, month, year);
		month++;
	}

	return 0;
}



/************************************************************
* print one hebrew year - regular output
************************************************************/
int print_hyear (option_list opt, double lat, double lon, int tz, int year)
{
	hdate_struct h;
	int month = 1;

	char *h_int_str;
	h_int_str = malloc(HEBREW_NUMBER_BUFFER_SIZE);

	/* print year header */
	if (!opt.iCal && !opt.short_format)
		printf ("%s:\n", hdate_get_int_string_(h_int_str, year,NO_COMPRESS));

	/* print year months */
	while (month < 13)
	{
		/* get date of month start */
		hdate_set_hdate (&h, 1, month, year);

		/* if leap year, print both Adar months */
		if (h.hd_size_of_year > 365 && month == 6)
		{
			hdate_set_hdate (&h, 1, 13, year);
			print_hmonth (&h, opt, lat, lon, tz, 13, year);
			hdate_set_hdate (&h, 1, 14, year);
			print_hmonth (&h, opt, lat, lon, tz, 14, year);
		}
		else
		{
			print_hmonth (&h, opt, lat, lon, tz, month, year);
		}
		month++;
	}

	return 0;
}



/****************************************************
* read and parse config file
****************************************************/
void read_config_file( FILE *config_file, option_list *opt )
{
	char	*input_string;
	size_t	input_str_len = 50;
	char	*input_key = "";
	char	*input_value = "";
	int		line_count = 0;
	int		match_count;
	int		end_of_input_file = FALSE;
	int		i;
	const int	num_of_keys = 17;
	const char*	key_list[] = {	"SUNSET_AWARE",
								"LATITUDE",
								"LONGITUDE",
								"TIMEZONE",
								"DIASPORA",
								"FORCE_ISRAEL",
								"PARASHA_NAMES",
								"SHABBAT_INFO",
								"FORCE_HEBREW",
								"OUTPUT_BIDI",
								"QUIET_ALERTS"};
	


	input_string = malloc(input_str_len+1);
	input_key    = malloc(input_str_len+1);
	input_value  = malloc(input_str_len+1);
	while ( end_of_input_file!=TRUE )
	{
		end_of_input_file = getline(&input_string, &input_str_len, config_file);
		if ( end_of_input_file!=TRUE )
		{
			errno = 0;
			match_count = sscanf(input_string,"%[A-Z_]=%sa",input_key,input_value);
			line_count++;
			if (errno != 0) error(0,errno,"scan error at line %d", line_count);
// DEBUG -	printf("line number = %d, matches made = %d, key = %s, value = %s, string = %s",
//					line_count, match_count, input_key, input_value, input_string);
			if (match_count ==2)
			{
				for (i=0; i<num_of_keys; i++)
				{
					if (strcmp(input_key, key_list[i]) == 0)
					{
						printf("match found!, %s = %s, index = %d\n",input_key,key_list[i],i);
						switch(i)
						{

		case  0:if      (strcmp(input_value,"FALSE") == 0) opt->not_sunset_aware = 1;
				else if (strcmp(input_value,"TRUE") == 0) opt->not_sunset_aware = 0;
				break;
		case  1:break;		//LATITUDE
		case  2:break;		//Longitude
		case  3:break;		//time zone
		case  4:if      (strcmp(input_value,"FALSE") == 0) opt->diaspora = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->diaspora = 1;
				break;
//		case  5:if      (strcmp(input_value,"FALSE") == 0) opt->force_israel = 0;
//				else if (strcmp(input_value,"TRUE") == 0) opt->force_israel = 1;
//				break;
		case  6:if      (strcmp(input_value,"FALSE") == 0) opt->parasha = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->parasha = 1;
				break;
//		case  7:if      (strcmp(input_value,"FALSE") == 0) opt->shabbat = 0;
//				else if (strcmp(input_value,"TRUE") == 0)
//				{
//					opt->shabbat = 1;
//					opt->parasha = 1;
//				}
//				break;
//		case  9:if      (strcmp(input_value,"FALSE") == 0) opt->force_hebrew = 0;
//				else if (strcmp(input_value,"TRUE") == 0) opt->force_hebrew = 1;
//				break;
//		case 10:if      (strcmp(input_value,"FALSE") == 0) opt->bidi = 0;
//				else if (strcmp(input_value,"TRUE") == 0)
//						{
//							opt->bidi = 1;
//							opt->force_hebrew = 1;
//						}
//				break;
		case 15:if      (strcmp(input_value,"FALSE") == 0) opt->quiet_alerts = 0;
				else if (strcmp(input_value,"TRUE") == 0) opt->quiet_alerts = 1;
				break;
						}
						break;
					}
				}
			}
		}
	}
	free(input_string);
	free(input_key);
	free(input_value);
	return;	
}



/************************************************************
*************************************************************
*************************************************************
* main function
*************************************************************
*************************************************************
************************************************************/
int main (int argc, char *argv[])
{

	hdate_struct h;				/* The Hebrew date */
	int c;

	int day;					/* The Gregorian date */
	int month;
	int year;

	double lat = BAD_COORDINATE;	/* set to this value for error handling */
	double lon = BAD_COORDINATE;	/* set to this value for error handling */
	int tz = BAD_TIMEZONE;			/* -z option Time Zone, default to system local time */

	int opt_latitude = 0;			/* -l option latitude */
	int opt_Longitude = 0;			/* -L option longitude */
	
	int error_detected = FALSE;		/* exit after reporting ALL bad parms */


	option_list opt;
	opt.hebrew = 0;
	opt.bidi = 0;
	opt.yom = 0;
	opt.leShabbat = 0;
	opt.leSeder = 0;
	opt.tablular_output = 0;
	opt.not_sunset_aware = 0;
	opt.quiet_alerts = 0;
	opt.print_tomorrow = 0; /*	This isn't a command line option;
								It here to restrict sunset_aware to
								single-day outputs */
	opt.sun = 0;			/* -s option sunrise/set times */
	opt.candles = 0;
	opt.havdalah = 0;
	opt.times = 0;			/* -t option print times of day */
	opt.short_format = 0;	/* -S Short format flag */
	opt.holidays = 0;		/* -h option holidays */
	opt.only_if_holiday = 0;				/* -H option just holidays */
	opt.parasha = 0;		/* -r option reading */
	opt.only_if_parasha = 0;				/* -R option just reading */
	opt.julian = 0;				/* -j option Julian day number */
	opt.diaspora = 0;				/* -d option diaspora */
	opt.iCal = 0;				/* -i option iCal */
	opt.omer = 0;				/* -o option Sfirat Haomer */



	// support for getopt short options
	static char * short_options = "bcdhHjioqrRsStTl:L:z:";

	/* support for getopt long options */
	/* note: other (long) options will be defined
	   globally at beginning of this file */
	int option_index = 0;
	static struct option long_options[] = {
		{"version", 0, 0, 0},
		{"help", 0, 0, 0},
		{"hebrew", 0, 0, 0},
		{"yom", 0, 0, 0},
		{"leshabbat", 0, 0, 0},
		{"leseder", 0, 0, 0},
		{"table",0,0,0},
		{"not-sunset-aware",0,0,0},
		{"quiet-alerts",0,0,0},
		{"short_format",0,0,0},
		{"parasha",0,0,0},
		{"holidays",0,0,0},
		{"shabbat-times",0,0,0},
		{"candle-lighting",0,0,0},
		{"sun",0,0,0},
		{"sunset",0,0,0},
		{"sunrise",0,0,0},
		{"candles",0,0,0},
		{"havdalah",0,0,0},
		{"latitude", 0, 0, 0},
		{"longitude", 0, 0, 0},
		{"timezone", 0, 0, 0},
		{"bidi", 0, 0, 0},
		{"visual", 0, 0, 0},
		{"omer", 0, 0, 0},
		{"ical", 0, 0, 0},
		{"julian", 0, 0, 0},
		{"diaspora", 0, 0, 0},
		{0, 0, 0, 0}
		};

	/* init locale */
	/* WHY?? -- reconsider this, or at least test it out */
	setlocale (LC_ALL, "");

	/************************************************************
	* parse config file
	************************************************************/
	FILE *config_file = get_config_file("/hdate", "/hdaterc", hdate_config_file_text);
	if (config_file != NULL)
	{
		read_config_file(config_file, &opt);
		fclose(config_file);
	}


	/************************************************************
	* parse command line
	************************************************************/
	opterr = 0; // we'll do our own error reporting
	while ((c = getopt_long(argc, argv, short_options,
						long_options, &option_index)) != -1)
	{
		switch (c)
		{
		case 0: /* long options */
			switch (option_index)
			{
			case 0:	print_version (); exit (0); break;
			case 1:	print_help (); exit (0); break;
			case 2: opt.hebrew = 1; break;
			case 3:
				opt.yom = 1;
				opt.hebrew = 1;
				break;
			case 4:
				opt.leShabbat = 1;
				opt.yom = 1;
				opt.hebrew = 1;
				break;
			case 5:
				opt.leSeder = 1;
				opt.yom = 1;
				opt.hebrew = 1;
				break;
			case 6:	opt.tablular_output = 1; break;
			case 7:	opt.not_sunset_aware = 1; break;
			case 8:	opt.quiet_alerts = 1; break;
			case 9:	opt.short_format = 1; break;
			case 10:opt.parasha = 1; break;
			case 11:opt.holidays = 1; break;
			case 12:
				opt.candles = 1;
				opt.havdalah = 1;
				break;
			case 13:
			case 14:
			case 15: opt.sun = 1;		break;
			case 16: opt.candles = 1;	break;
			case 17: opt.havdalah = 1;	break;
			case 18: error_detected = parse_latitude(&lat, &opt_latitude);	break;
			case 19: error_detected = parse_longitude(&lon, &opt_Longitude);	break;
			case 20: error_detected = parse_timezone(&tz);	break;
			case 21:
			case 22: opt.bidi = 1; opt.hebrew = 1; break;
			case 23: opt.omer = 1; break;
			case 24: opt.iCal = 1; break;
			case 25: opt.julian = 1; break;
			case 26: opt.diaspora = 1; break;
			}
			break;
		case 'b': opt.bidi = 1; opt.hebrew = 1; break;
		case 'c':	opt.candles = 1;
					opt.havdalah = 1;
					break;
		case 'd': opt.diaspora = 1; break;
		case 'H': opt.only_if_holiday = 1;	/* why no break? */
								/* it seems ok 'H' is used only to abort
									a print, so it always needs 'h' set */
		case 'h': opt.holidays = 1; break;
		case 'i': opt.iCal = 1; break;
		case 'j': opt.julian = 1; break;
		case 'q': opt.quiet_alerts = 1; break;
		case 'o': opt.omer = 1; break;
		case 'R': opt.only_if_parasha = 1;
		case 'r': opt.parasha = 1; break;
		case 'S': opt.short_format = 1; break;
		case 's': opt.sun = 1; break;
		case 't': opt.times = 1; break;
		case 'T': opt.tablular_output = 1; break;
		case 'l': error_detected = parse_latitude(&lat,&opt_latitude);	break;
		case 'L': error_detected = parse_longitude(&lon,&opt_Longitude); break;
		case 'z': parse_timezone(&tz);	break;
		case '?':
			if (strchr(short_options,optopt)==NULL)
				error(0,0,"option %c unknown",optopt);
			else error(0,0,"option %c missing parameter",optopt);
			// variable error_detected is defined in location.include.c
			error_detected = TRUE;
			break;
		default:
			print_usage (); exit (0); break;
		}
	}


	/************************************************************
	* function validate_location is defined in the include file
	* ./location.include.c
	* It issues an exit(EXIT_CODE_BAD_PARMS) [==1]
	* if it discovers, um, bad parameters 
	************************************************************/
	validate_location(opt_latitude, opt_Longitude, &lat, &lon, &tz, opt.quiet_alerts, error_detected);
	
	/************************************************************
	* option "s" is a subset of option "t"
	************************************************************/
	if (opt.sun && opt.times) opt.sun=0;


/************************************************************
* begin parse input date
************************************************************/

	/************************************************************
	* parse input date - no date entered
	************************************************************/
	if (argc == optind)
	{

		/* set date for today */
		hdate_set_gdate (&h, 0, 0, 0);

		if (opt.tablular_output)
		{
			print_tabular_header( opt );
			print_tabular_day(&h, opt, lat, lon, tz);
		}
		else
		{
			if (opt.iCal) print_ical_header ();
			else if (!opt.not_sunset_aware)
				opt.print_tomorrow = check_for_sunset(&h, lat, lon, tz);
			print_day (&h, opt, lat, lon, tz);
			if (opt.iCal) print_ical_footer ();
		}
	}

	/************************************************************
	* parse input date - only year or julian day number entered
	************************************************************/
	else if (argc == (optind + 1))
	{
		/* get year */
		year = atoi (argv[optind]);

		/* handle error */
		if (year <= 0) { print_parm_error(N_("year")); exit (0); }


		/************************************************************
		* process single Julian day
		************************************************************/
		if (year > 348021)
		{
			hdate_set_jd (&h, year);

			if (opt.tablular_output)
			{
				print_tabular_header( opt );
				print_tabular_day(&h, opt, lat, lon, tz);
			}
			else
			{
				if (opt.iCal) print_ical_header ();
				print_day (&h, opt, lat, lon, tz);
				if (opt.iCal) print_ical_footer ();
			}
		}


		/************************************************************
		* process entire Hebrew year
		************************************************************/
		else if (year > 3000)
		{
			if (opt.tablular_output)
			{
				print_tabular_header( opt );
				print_tabular_hyear( opt, lat, lon, tz, year);
			}
			else
			{
				if (opt.iCal) print_ical_header ();
				print_hyear ( opt, lat, lon, tz, year);
				if (opt.iCal) print_ical_footer ();
			}
		}
		/************************************************************
		* process entire Gregorian year
		************************************************************/
		else
		{
			if (opt.tablular_output)
			{
				print_tabular_header( opt );
				print_tabular_gyear( opt, lat, lon, tz, year);
			}
			else
			{
				if (opt.iCal) print_ical_header ();

				print_gyear ( opt, lat, lon, tz, year);

				if (opt.iCal) print_ical_footer ();
			}

		}
	}


	/************************************************************
	* parse input date - only month and year entered
	************************************************************/
	else if (argc == (optind + 2))
	{

		year = atoi (argv[optind + 1]);
		month = atoi (argv[optind]);

		/* handle errors */
		if (year <= 0) { print_parm_error(N_("year")); exit (0); }


		/************************************************************
		* process entire Hebrew month
		************************************************************/
		if (year > 3000)
		{
			/* begin bounds check for month */
			if ((month <= 0) || (month > 14))
				{ print_parm_error(N_("month")); exit (0); }
			hdate_set_hdate (&h, 1, 1, year);
			if ((h.hd_size_of_year <365) && (month >12))
				{ print_parm_error(N_("month")); exit (0); }
			/* end bounds check for month */


			if (opt.tablular_output)
			{
				print_tabular_header( opt );

				/* get date of month start */
				hdate_set_hdate (&h, 1, month, year);

				/* if leap year, print both Adar months */
				if (h.hd_size_of_year > 365 && month == 6)
				{
					hdate_set_hdate (&h, 1, 13, year);
					print_tabular_hmonth ( opt, lat, lon, tz, 13, year);
					hdate_set_hdate (&h, 1, 14, year);
					print_tabular_hmonth ( opt, lat, lon, tz, 14, year);
				}
				else
				{
					print_tabular_hmonth ( opt, lat, lon, tz, month, year);
				}
			}
			else
			{
				if (opt.iCal) print_ical_header ();

				/* get date of month start */
				hdate_set_hdate (&h, 1, month, year);

				/* if leap year, print both Adar months */
				if (h.hd_size_of_year > 365 && month == 6)
				{
					hdate_set_hdate (&h, 1, 13, year);
					print_hmonth (&h, opt, lat, lon, tz, 13, year);
					hdate_set_hdate (&h, 1, 14, year);
					print_hmonth (&h, opt, lat, lon, tz, 14, year);
				}
				else
				{
					print_hmonth (&h, opt, lat, lon, tz, month, year);
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
			if ((month <= 0) || (month > 12))
				{ print_parm_error(N_("month")); exit (0); }

			if (opt.tablular_output)
			{
				print_tabular_header( opt );
				print_tabular_gmonth(opt, lat, lon,  tz, month, year);
			}

			else
			{
				if (opt.iCal)print_ical_header ();

				print_gmonth (opt, lat, lon,  tz, month, year);

				if (opt.iCal) print_ical_footer ();
			}
		}
	}


	/************************************************************
	* parse input date - day, month and year entered
	************************************************************/
	else if (argc == (optind + 3))
	{
		year = atoi (argv[optind + 2]);
		month = atoi (argv[optind + 1]);
		day = atoi (argv[optind]);

		/* handle error */
		if (year <= 0) { print_parm_error(N_("year")); exit (0); }


		/************************************************************
		* get Hebrew date
		************************************************************/
		if (year > 3000)
		{
			/* begin bounds check for month */
			if ((month <= 0) || (month > 14))
				{ print_parm_error(N_("month")); exit (0); }
			hdate_set_hdate (&h, 1, 1, year);
			if ((h.hd_size_of_year <365) && (month >12))
				{ print_parm_error(N_("month")); exit (0); }
			/* end bounds check for month */

			if ((day <= 0) || (day > 30) ||
				((day > 29) && ((month==4) || (month==6) || (month==8) || (month=10) || (month==12) || (month==14))) ||
				(((h.hd_size_of_year==353) || (h.hd_size_of_year==383)) && ((month==2) || (month==3)) && (day >29)) ||
				(((h.hd_size_of_year==354) || (h.hd_size_of_year==384)) && (month==2) && (day>29)) )			
				{ print_parm_error(N_("day")); exit (0); }


			hdate_set_hdate (&h, day, month, year);
		}

		/************************************************************
		* get Gregorian date
		************************************************************/
		else
		{
			if ((month <= 0) || (month > 12))
				{ print_parm_error(N_("month")); exit (0); }
			if ((day <= 0) || 
				((day > 31) && ((month==1) || (month==3) || (month==5) || (month==7) || (month==8) || (month==10) ||(month==12))) ||
				((day > 30) && ((month==4) || (month==6) || (month==9) || (month==11))) ||
				((day > 29) && (month==2) && ((year%4)==0)) ||
				((day > 28) && (month==2) && ((year%4)!=0))
			   )
				{ print_parm_error(N_("day")); exit (0); }

			hdate_set_gdate (&h, day, month, year);
		}

		/************************************************************
		* process a single date
		************************************************************/
		if (opt.tablular_output)
		{
			print_tabular_header( opt );
			print_tabular_day(&h, opt, lat, lon, tz);
		}

		else
		{
			if (opt.iCal) print_ical_header ();
			print_day (&h, opt, lat, lon, tz);

			if (opt.iCal) print_ical_footer ();
		}
	}


	/************************************************************
	* parse input date - too many fields were received
	************************************************************/
	else
	{
		error(0,0,"%s", N_("too many arguments; after options max is dd mm yyyy"));
		exit (0);
	}

	return 0;
}
