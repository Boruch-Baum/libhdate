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

#include <stdio.h>		// For printf
#include <hdate.h>		// For hebrew date
#include <stdlib.h>		// For atoi, getenv, setenv
#include <locale.h>		// For setlocale
#include <getopt.h>		// For getopt_long
#include <time.h>		// For time
#include <error.h>		// For error
#include <string.h>		// For strchr

#define FALSE 0
#define TRUE -1
#define DATA_WAS_NOT_PRINTED 0
#define DATA_WAS_PRINTED 1

// copied from support.h in src dir
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

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
				int yom;
				int hebrew;
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
	
	printf ("OPTIONS:\n");
	printf ("   -i     use iCal formated output.\n");
	printf ("   -S     print using short format.\n");
	printf ("   -s     print sunrise/sunset times. (default: Tel Aviv winter time)\n");
	printf ("   -c     print Shabbat start/end times.\n");
	printf ("            Shabbat starts 20 min before sunset,\n");
	printf ("            and ends when three stars are out.\n");
	printf ("   -t     print day times: first light, talit, sunrise,\n");
	printf ("            midday, sunset, first stars, three stars.\n");
	printf ("   -h     print holidays.\n");
	printf ("   -H     print just holidays.\n");
	printf ("   -r     print weekly reading on saturday.\n");
	printf ("   -R     print just weekly reading on saturday.\n");
	printf ("   -o     print Sefirat Haomer.\n");
	printf ("   -j     print Julian day number.\n");
	printf ("   -d     use diaspora reading and holidays.\n");
	printf ("   -q     quiet. suppress warning messages.]n");
	printf ("   -z nn  timezone, +/-UTC\n\n");
	printf ("   -l xx  latitude xx degrees. Negative values are South.\n");
	printf ("   -L yy  longitude yy degrees. Negative values are West.\n\n");

	printf ("   --table         tabular output, suitable for spreadsheets\n");
	printf ("   --hebrew        forces Hebrew to print in Hebrew characters\n");
	printf ("   --yom           force Hebrew prefix to Hebrew day of week\n");
	printf ("   --leshabbat     insert parasha between day of week and day\n");
	printf ("   --leseder       insert parasha between day of week and day\n");
	printf ("   --not-sunset-aware  don't display next day if after sunset\n");

	printf ("USEFUL LOCATIONS/TIMEZONES:\n");
	printf ("      Jerusalem    31, 35, 2    Tiberias     32, 35, 2\n");
	printf ("      Tel Aviv     32, 34, 2    London       51,  0, 0\n");
	printf ("      Haifa        32, 34, 2    Paris        48,  2, 1\n");
	printf ("      Beer Sheva   31, 34, 2    New York     40,-74,-5\n");
	printf ("      Ashdod       31, 34, 2    Moscow       55, 37, 3\n");
	printf ("      Eilat        29, 34, 2\n");

}
/* TODO - document these options here and in man page
--hebrew
--yom (implies --hebrew)
--leshabbat (implies --hebrew and --yom)
--leseder (implies --hebrew and --yom)
--table
--not-sunset-aware
*/


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
				hdate_get_int_string_ (h->hd_day, NO_COMPRESS),
				bet_h,
				hdate_get_hebrew_month_string (h->hd_mon, opt.short_format));
		printf ("%s", hdate_get_int_string_(h->hd_year, NO_COMPRESS));
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
			printf ("%s", hdate_get_int_string_(h->hd_day, NO_COMPRESS));
		}

		printf (" %s ", hdate_get_hebrew_month_string (h->hd_mon, opt.short_format));
		printf ("%s\n", hdate_get_int_string_(h->hd_year, NO_COMPRESS));
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

		// print Hebrew day of week, including possibly Shabbat name
		if (opt.hebrew)
		{
			setenv("LANGUAGE", "he_IL.UTF-8", 1);
			if (opt.yom)
				{
				printf("%s%s %s", eve_before,for_day_of_h, hdate_get_day_string (h->hd_dw, opt.short_format));
				if ((opt.leShabbat) || (opt.leSeder))
				{
					is_parasha_read = find_shabbat(h, opt.diaspora);
					if (is_parasha_read)
							printf(" %s %s", for_week_of, hdate_get_parasha_string (is_parasha_read, opt.diaspora));
				}
				printf ("%s ", psik_mark);
			}
		}
		else printf("%s",eve_before);

		// print Hebrew dd mmmm yyyy
		printf ("%s %s%s ",
				hdate_get_int_string_(h->hd_day, NO_COMPRESS),
				bet_h,
				hdate_get_hebrew_month_string (h->hd_mon, opt.short_format));
		printf ("%s\n", hdate_get_int_string_(h->hd_year, NO_COMPRESS));
	}


	/************************************************************
	* CLEANUP - restore default language environment variable *
	************************************************************/
	setenv("LANGUAGE", language, 1); // not sure this is really doing anything
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
		printf ("%s", hdate_get_int_string_(h->hd_day, NO_COMPRESS));
	}

	printf (" %s ", hdate_get_hebrew_month_string (h->hd_mon, 1));
	printf ("%s", hdate_get_int_string_(h->hd_year, NO_COMPRESS));
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
		print_tabular_header( opt );
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

	/* print year header */
	if (!opt.iCal && !opt.short_format)
		printf ("%s:\n", hdate_get_int_string_(year,NO_COMPRESS));

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

	option_list opt;
	opt.hebrew = 0;
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
	static char * short_options = "sctShHorRjdiql:L:z:";

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
		{0, 0, 0, 0}
		};

	/* init locale */
	/* WHY?? -- reconsider this, or at least test it out */
	setlocale (LC_ALL, "");


	/************************************************************
	* location-related parsing functions
	*	void parse_latitude()
	*	void parse_longitude()
	*	void parse_timezone()
	*	void validate_location()
	*	double lat, lon
	*	int tz
	*	int error_detected
	************************************************************/
	#include "./location.include.c"


/************************************************************
* begin command line parsing
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
			case 18: parse_latitude();	break;
			case 19: parse_longitude();	break;
			case 20: parse_timezone();	break;
			}
			break;
		case 's': opt.sun = 1; break;
		case 'c':	opt.candles = 1;
					opt.havdalah = 1;
					break;
		case 't': opt.times = 1; break;
		case 'S': opt.short_format = 1; break;
		case 'H': opt.only_if_holiday = 1;	/* why no break? */
								/* it seems ok 'H' is used only to abort
									a print, so it always needs 'h' set */
		case 'h': opt.holidays = 1; break;
		case 'o': opt.omer = 1; break;
		case 'R': opt.only_if_parasha = 1;
		case 'r': opt.parasha = 1; break;
		case 'j': opt.julian = 1; break;
		case 'd': opt.diaspora = 1; break;
		case 'i': opt.iCal = 1; break;
		case 'q': opt.quiet_alerts = 1; break;
		case 'l': parse_latitude();	break;
		case 'L': parse_longitude(); break;
		case 'z': parse_timezone();	break;
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
	validate_location();
	
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
