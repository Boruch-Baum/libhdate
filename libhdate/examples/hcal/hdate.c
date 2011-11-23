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

#include <stdio.h>				/* For printf */
#include <hdate.h>				/* For hebrew date */
#include <stdlib.h>				/* For atoi, getenv, setenv */
#include <locale.h>				/* For setlocale */
/* #include <unistd.h>		 For getopt */
#include <getopt.h>				/* For getopt_long */
#include <time.h>				/* For time */
#include <error.h>				/* For error */

#define FALSE 0
#define TRUE -1
#define DELTA_LONGITUDE 30
#define BAD_COORDINATE 999
#define DATA_WAS_NOT_PRINTED 0
#define DATA_WAS_PRINTED 1

/* Temporarily define option variables globally */
int opt_hebrew = 0;
int opt_yom = 0;
int opt_leShabbat = 0;
int opt_leSeder = 0;
int opt_tablular_output = 0;
int opt_sunset_aware = 0;
int opt_print_tomorrow = 0; /*	This one isn't a command line option;
								It here to restrict sunset_aware to
								single-day outputs */


/*  I'm guessing I should pad extra room for
	gettext internationalization. I really
	should just learn the subject */
static char sunrise_text[15] = "sunrise";
static char sunset_text[15]  = "sunset";
static char first_light_text[15] = "first_light";
static char talit_text[15] = "talit";
static char midday_text[15] = "midday";
static char first_stars_text[15] = "first_stars";
static char three_stars_text[15] = "three_stars";

/* print version */
int
print_version ()
{
	printf ("hdate - display Hebrew date information\nversion 2\n");
	return 0;
}

/* print help */
int
print_help ()
{
	printf ("hdate - display Hebrew date information\n\n");

	printf ("USAGE: hdate [options] [-l xx[.xxx] -L yy[.yyy] ] [[[day] month] year]\n");
	printf ("       hdate [options] [-l xx[.xxx] -L yy[.yyy] ] [julian_day]\n\n");

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
	printf ("   -z nn  timezone, +/-UTC\n\n");
	printf ("   -l xx  latitude xx degrees. Negative values are South.\n");
	printf ("   -L yy  longitude yy degrees. Negative values are West.\n\n");

	printf ("   --table	        tabular output, suitable for spreadsheets\n");
	printf ("   --hebrew        forces Hebrew to print in Hebrew characters\n");
	printf ("   --yom           force Hebrew prefix to Hebrew day of week\n");
	printf ("   --leshabbat     insert parasha between day of week and day\n");
	printf ("   --leseder       insert parasha between day of week and day\n");
	printf ("   --sunset-aware  display next day if after sunset\n");

	printf ("USEFUL LOCATIONS/TIMEZONES:\n");
	printf ("      Jerusalem    31, 35, 2    Tiberias     32, 35, 2\n");
	printf ("      Tel Aviv     32, 34, 2    London       51,  0, 0\n");
	printf ("      Haifa        32, 34, 2    Paris        48,  2, 1\n");
	printf ("      Beer Sheva   31, 34, 2    New York     40,-74,-5\n");
	printf ("      Ashdod       31, 34, 2    Moscow       55, 37, 3\n");
	printf ("      Eilat        29, 34, 2\n");

	return 0;
}
/* TODO - document these options here and in man page
--hebrew
--yom (implies --hebrew)
--leshabbat (implies --hebrew and --yom)
--leseder (implies --hebrew and --yom)
--table
--sunset-aware
*/


/************************************************************
* begin - error message functions
************************************************************/
void print_parm_error ( char *parm_name )
{
	error(0,0,"error: parameter %s is non-numeric or out of bounds",parm_name);
}

void print_timezone_warning( int tz )
{
	error(0,0,"time zone not entered, using system local time zone: %s, %+d UTC", *tzname, tz);
}

void print_coordinate_warning( char *city_name )
{
	error(0,0,"guessing... will use co-ordinates for %s", city_name);
}

void print_default_location_warning( int tz )
{
	switch (tz)
	{
	case -8:	print_coordinate_warning( "Los Angeles" ); break;
	case -6:	print_coordinate_warning( "Mexico City"); break;
	case -5:	print_coordinate_warning( "New York City"); break;
//	case -5:	print_coordinate_warning( "Toronto"); break;
//	case -3:	print_coordinate_warning( "Sao Paolo"); break;
	case -3:	print_coordinate_warning( "Buenos Aires"); break;
	case  0:	print_coordinate_warning( "London"); break;
	case  1:	print_coordinate_warning( "Paris"); break;
	case  2:	print_coordinate_warning( "Tel-Aviv"); break;
	case  3:	print_coordinate_warning( "Moscow"); break;
	case  5:	print_coordinate_warning( "Tashkent"); break;
	case  8:	print_coordinate_warning( "Beijing"); break;
//	case  8:	print_coordinate_warning( "Hong Kong"); break;
	case 10:	print_coordinate_warning( "Honolulu"); break;
	default:	error(0,0,"Hmmm, ... hate to do this, really ... \nusing co-ordinates for the equator, at the center of time zone"); break;
	}
}

void print_sunset_warning ()
{
	error(0,0,"the information displayed is for today's Hebrew date. Because it is now after sunset, that means the data is for the Gregorian day beginning at midnight.");
}
/************************************************************
* end - error message functions
************************************************************/




/************************************************************
* set default location
************************************************************/
void set_default_location( int tz, double *lat, double *lon )
{
	/*	Temporarily, set some default lat/lon coordinates
		for certain timezones */
	switch (tz)
	{
	case -8:	*lat =  34.05;	*lon =-118.25;	break; // Los Angeles
	case -6:	*lat =  19.43;	*lon = -99.13;	break; // Mexico City
	case -5:	*lat =  40.0;	*lon = -74.0;	break; // New York
//	case -5:	*lat = -43.71;	*lon = -79.43;	break; // Toronto
//	case -3:	*lat = -23.55;	*lon = -46.61;	break; // Sao Paolo
	case -3:	*lat = -34.6;	*lon = -58.37;	break; // Buenos Aires
	case  0:	*lat =  51.0;	*lon =   0.0;	break; // London
	case  1:	*lat =  48.0;	*lon =   2.0;	break; // Paris
	case  2:	*lat =  32.0;	*lon =  34.0;	break; // Tel aviv
	case  3:	*lat =  55.0;	*lon =  37.0;	break; // Moscow
	case  5:	*lat =  41.27;	*lon =  69.22;	break; // Tashkent
	case  8:	*lat =  39.90;	*lon = 116.38;	break; // Beijing
//	case  8:	*lat =  22.26;	*lon = 114.15;	break; // Hong Kong
	case 10:	*lat =  21.30;	*lon = 157.82;	break; // Honolulu
	default:	*lat =   0.0;	*lon =  tz * 15; break; /* center of tz */
	}
	return;
}


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
	if (timeval < 0) printf("--:--, ");
	else printf("%02d:%02d, ",
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
* check for sunset
************************************************************/
int
check_for_sunset (hdate_struct * h, double lat, double lon )
{
	time_t now_time;
	struct tm *now_timep;
	int sunrise, sunset;

	hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
							&sunrise, &sunset);

	time(&now_time);
	now_timep = localtime(&now_time);

	if ( (now_timep->tm_hour *60 + now_timep->tm_min) > sunset ) return 1;
	else return 0;
}





/************************************************************
* print one date - both Gregorian and Hebrew
************************************************************/
int
print_date (hdate_struct * h, hdate_struct * tomorrow, int opt_S, int opt_i, int opt_d, double lat, double lon)
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



/* TODO - learn about this ENABLE_NLS option */
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
	if (opt_print_tomorrow)
	{
		if (opt_hebrew)
		{
			eve_before = "אור ל";
			if (!opt_yom) for_day_of_h="-";
		}
		else eve_before = "eve of ";
	}



	/************************************************************
	* preliminary - prepare Hebrew prefixes if forcing any Hebrew
	************************************************************/
	language=getenv("LANGUAGE");
	if (opt_hebrew)
	{
		bet_h="ב";
		/* preliminary - prepare Yom prefix */
		if (opt_yom)
		{
			for_day_of_h="יום";
			psik_mark=",";

			if		(opt_leShabbat)
				{
				if (h->hd_dw==7)		for_week_of="פרשת";
				else					for_week_of="לשבת";
				}
			else if (opt_leSeder)	for_week_of="לסדר";
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
	if (opt_i)
	{
		if (opt_hebrew)
				setenv("LANGUAGE", "he_IL.UTF-8", 1);
		printf ("%s%s %s%s ",
				for_day_of_g,
				hdate_get_int_string (h->hd_day),
				bet_h,
				hdate_get_hebrew_month_string (h->hd_mon, opt_S));
		printf ("%s", hdate_get_int_string (h->hd_year));
	}


	/************************************************************
	* short format
	************************************************************/
	else if (opt_S)
	{
		printf ("%d.%d.%d  ", h->gd_day, h->gd_mon, h->gd_year);


		if (!hdate_is_hebrew_locale() && (!opt_hebrew))
		{
			printf ("%d", h->hd_day);
		}
		else
		{
			setenv("LANGUAGE", "he_IL.UTF-8", 1);
			printf ("%s", hdate_get_int_string (h->hd_day));
		}

		printf (" %s ", hdate_get_hebrew_month_string (h->hd_mon, opt_S));
		printf ("%s\n", hdate_get_int_string (h->hd_year));
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
				hdate_get_day_string (h->hd_dw, opt_S),
				h->gd_day,
				bet_g,
				hdate_get_month_string (h->gd_mon, opt_S), h->gd_year);


		/************************************************************
		* Start of the Hebrew date
		************************************************************/
		if (opt_print_tomorrow) *h = *tomorrow;

		// print Hebrew day of week, including possibly Shabbat name
		if (opt_hebrew)
		{
			setenv("LANGUAGE", "he_IL.UTF-8", 1);
			if (opt_yom)
				{
				printf("%s%s %s", eve_before,for_day_of_h, hdate_get_day_string (h->hd_dw, opt_S));
				if ((opt_leShabbat) || (opt_leSeder))
				{
					is_parasha_read = find_shabbat(h, opt_d);
					if (is_parasha_read)
							printf(" %s %s", for_week_of, hdate_get_parasha_string (is_parasha_read, opt_d));
				}
				printf ("%s ", psik_mark);
			}
		}
		else printf("%s",eve_before);

		// print Hebrew dd mmmm yyyy
		printf ("%s %s%s ",
				hdate_get_int_string (h->hd_day),
				bet_h,
				hdate_get_hebrew_month_string (h->hd_mon, opt_S));
		printf ("%s\n", hdate_get_int_string (h->hd_year));
	}


	/************************************************************
	* CLEANUP - restore default language environment variable *
	************************************************************/
	setenv("LANGUAGE", language, 1);
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
	printf("sun hour: %02d:%02d\n",sun_hour/60, sun_hour%60);

	return DATA_WAS_PRINTED;
}



/************************************************************
* option 'h' - holiday identification
************************************************************/
int print_holiday (hdate_struct * h, int opt_d, int opt_S, int opt_i)
{
	int holyday;

	holyday = hdate_get_holyday (h, opt_d);
	if (holyday)
	{
		printf ("%s\n", hdate_get_holyday_string (holyday, opt_S));
		return DATA_WAS_PRINTED;
	}
	else return DATA_WAS_NOT_PRINTED;
}


/************************************************************
* option 'o' - sefirat ha'omer
************************************************************/
int print_omer (hdate_struct * h)
{
	int omer_day;

	omer_day = hdate_get_omer_day(h);
	if (omer_day != 0)
	{
		printf ("today is day %s \n", hdate_get_omer_string (omer_day));
		return DATA_WAS_PRINTED;
	}
	else return DATA_WAS_NOT_PRINTED;
}



/************************************************************
* option 'r' - parashat hashavua
************************************************************/
int print_reading (hdate_struct * h, int opt_d, int opt_S, int opt_i)
{
	int reading;

	reading = hdate_get_parasha (h, opt_d);
	if (reading)
	{
		printf ("Parashat %s\n", hdate_get_parasha_string (reading, opt_S));
		return DATA_WAS_PRINTED;
	}
	else return DATA_WAS_NOT_PRINTED;
}



/************************************************************
* option 'c' - candle-lighting time; tzeit Shabbat
************************************************************/
int print_candles (hdate_struct * h, double lat, double lon, int tz, int opt_i)
{
	int sun_hour, first_light, talit, sunrise;
	int midday, sunset, first_stars, three_stars;

	/* check for friday - print knisat shabat */
	if (h->hd_dw == 6)
	{
		/* get times */
		hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
								&sunrise, &sunset);

		/* FIXME - knisat shabat 20 minutes before shkiaa ? */
		sunset = sunset + tz * 60 - 20;

		/* print candlelighting times */
		printf ("candle-lighting: %d:%d\n", sunset / 60, sunset % 60);
		return DATA_WAS_PRINTED;
	}

	/* check for saturday - print motzay shabat */
	else if (h->hd_dw == 7)
	{
		/* get times */
		hdate_get_utc_sun_time_full (h->gd_day, h->gd_mon, h->gd_year, lat,
									 lon, &sun_hour, &first_light, &talit,
									 &sunrise, &midday, &sunset,
									 &first_stars, &three_stars);

		three_stars = three_stars + tz * 60;

		/* print motzay shabat */
		printf ("Shabbat ends: %d:%d\n", three_stars / 60, three_stars % 60);
		return DATA_WAS_PRINTED;
	}
	else return DATA_WAS_NOT_PRINTED;
}



/************************************************************
* print one day - tabular output *
************************************************************/
int
print_tabular_day (hdate_struct * h,
		   int opt_d, double lat, double lon, int tz, int opt_s, int opt_h, int opt_o, int opt_r,
		   int opt_R, int opt_H, int opt_c, int opt_t)
{
	int sun_hour, first_light, talit, sunrise;
	int midday, sunset, first_stars, three_stars;

	/* print short Gregorian date */
	printf ("%d.%d.%d, ", h->gd_day, h->gd_mon, h->gd_year);


	/************************************************************
	* begin - print Hebrew date
	************************************************************/
	if ((!hdate_is_hebrew_locale()) && (!opt_hebrew))
	{	/* non hebrew numbers */
		printf ("%d", h->hd_day);
	}
	else /* Hebrew */
	{
		setenv("LANGUAGE", "he_IL.UTF-8", 1);
		printf ("%s", hdate_get_int_string (h->hd_day));
	}

	printf (" %s ", hdate_get_hebrew_month_string (h->hd_mon, 1));
	printf ("%s, ", hdate_get_int_string (h->hd_year));
	/* end - print Hebrew date */

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
	printf("%02d:%02d\n",sun_hour/60, sun_hour%60);

	return 0;
}




/************************************************************
* print one day - regular output
************************************************************/
int print_day (hdate_struct * h, int opt_d, int opt_S,
		   double lat, double lon, int tz, int opt_s, int opt_h, int opt_o, int opt_r,
		   int opt_R, int opt_j, int opt_H, int opt_i, int opt_c, int opt_t)
{

	time_t t;
	hdate_struct tomorrow;

	int iCal_uid_counter = 0;
	int data_printed = 0;

	/************************************************************
	* options -R, -H are restrictive filters, so if there is no
	* parasha reading / holiday, print nothing.
	************************************************************/
	/* check for just parasha or holiday flag */
	if (opt_R && opt_H &&
		!hdate_get_parasha (h, opt_d) && !hdate_get_holyday (h, opt_d))
		return 0;
	if (opt_R && !opt_H && !hdate_get_parasha (h, opt_d))
		return 0;
	if (opt_H && !opt_R && !hdate_get_holyday (h, opt_d))
		return 0;


	/************************************************************
	* print the iCal event header
	************************************************************/
	if (opt_i)
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
	if (opt_j) printf ("JDN-%d ", h->hd_jd);


	/************************************************************
	* print the date
	************************************************************/
	if (opt_print_tomorrow)	hdate_set_jd (&tomorrow, (h->hd_jd)+1);
	print_date (h, &tomorrow, opt_S, opt_i, opt_d, lat, lon);



	/************************************************************
	* begin - print additional information for day
	************************************************************/
	if (opt_print_tomorrow) *h = tomorrow;
	if (opt_s) data_printed = data_printed | print_sunrise (h, lat, lon, tz, opt_i);
	if (opt_t) data_printed = data_printed | print_times (h, lat, lon, tz, opt_i);
	if (opt_h) data_printed = data_printed | print_holiday (h, opt_d, opt_S, opt_i);
	if (opt_o) data_printed = data_printed | print_omer (h);
	if (opt_r) data_printed = data_printed | print_reading (h, opt_d, opt_S, opt_i);
	if (opt_c) data_printed = data_printed | print_candles (h, lat, lon, tz, opt_i);
	if ((opt_print_tomorrow) && (data_printed)) print_sunset_warning();
	/************************************************************
	* end - print additional information for day
	************************************************************/




	/************************************************************
	* print the iCal event footer
	************************************************************/
	if (opt_i)
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
int print_tabular_gmonth
	(	int opt_d, double lat, double lon, int tz,
		int opt_s, int opt_h, int opt_o, int opt_r, int opt_R,
		int opt_H, int opt_c, int opt_t, int month, int year)
{
	hdate_struct h;
	int jd;

	/* get date of month start */
	hdate_set_gdate (&h, 1, month, year);
	jd = h.hd_jd;

	/* print month days */
	while (h.gd_mon == month)
	{
		print_tabular_day (&h, opt_d, lat, lon, tz, opt_s, opt_h, opt_o,
				   opt_r, opt_R, opt_H, opt_c, opt_t);
		jd++;
		hdate_set_jd (&h, jd);
	}

	return 0;
}



/************************************************************
* print one Gregorian month - regular output
************************************************************/
int
print_gmonth (int opt_d, int opt_S,
			 double lat, double lon, int tz,
			 int opt_s, int opt_h, int opt_o, int opt_r, int opt_R, int opt_j,
			 int opt_H, int opt_i, int opt_c, int opt_t, int month, int year)
{
	hdate_struct h;
	int jd;

	/* get date of month start */
	hdate_set_gdate (&h, 1, month, year);
	jd = h.hd_jd;

	/* print month header */
	if (!opt_i && !opt_S)
		printf ("\n%s:\n", hdate_get_month_string (h.gd_mon, opt_S));

	/* print month days */
	while (h.gd_mon == month)
	{
		print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
				   opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t);

		jd++;
		hdate_set_jd (&h, jd);
	}

	return 0;
}



/************************************************************
* print one Hebrew month - tabular output *
************************************************************/
int print_tabular_hmonth
	(	int opt_d, double lat, double lon, int tz,
		int opt_s, int opt_h, int opt_o, int opt_r, int opt_R,
		int opt_H, int opt_c, int opt_t, int month, int year)
{
	hdate_struct h;
	int jd;

	/* get date of month start */
	hdate_set_hdate (&h, 1, month, year);
	jd = h.hd_jd;

	/* print month days */
	while (h.gd_mon == month)
	{
		print_tabular_day (&h, opt_d, lat, lon, tz, opt_s, opt_h, opt_o,
				   opt_r, opt_R, opt_H, opt_c, opt_t);
		jd++;
		hdate_set_jd (&h, jd);
	}

	return 0;
}


/************************************************************
* print one Hebrew month - regular output
************************************************************/
int
print_hmonth (hdate_struct * h, int opt_d, int opt_S,
					double lat, double lon, int tz,
					int opt_s, int opt_h, int opt_o, int opt_r, int opt_R, int opt_j,
					int opt_H, int opt_i, int opt_c, int opt_t, int month,
					int year)
{
	int jd;

	/* get date of month start */
	jd = h->hd_jd;

	/* print month header */
	if (!opt_i && !opt_S)
		printf ("\n%s:\n", hdate_get_hebrew_month_string (h->hd_mon, opt_S));

	/* print month days */
	while (h->hd_mon == month)
	{
		print_day (h, opt_d, opt_S, lat, lon, tz, opt_s,
				   opt_h, opt_o, opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t);

		jd++;
		hdate_set_jd (h, jd);
	}
	return 0;
}



/************************************************************
* print one Gregorian year - tabular output *
************************************************************/
int print_tabular_gyear
	(	int opt_d, double lat, double lon, int tz,
		int opt_s, int opt_h, int opt_o, int opt_r, int opt_R,
		int opt_H, int opt_c, int opt_t, int year)
{
	int month = 1;
	while (month < 13)
	{
		print_tabular_gmonth
			(opt_d, lat, lon, tz, opt_s, opt_h, opt_o, opt_r,
					 opt_R, opt_H, opt_c, opt_t, month, year);
		month++;
	}
	return 0;
}


/************************************************************
* print one Hebrew year - tabular output *
************************************************************/
int print_tabular_hyear
	(	int opt_d, double lat, double lon, int tz,
		int opt_s, int opt_h, int opt_o, int opt_r, int opt_R,
		int opt_H, int opt_c, int opt_t, int year)
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
			print_tabular_hmonth(opt_d, lat, lon, tz, opt_s, opt_h, opt_o, opt_r,
					 opt_R, opt_H, opt_c, opt_t, 13, year);
			hdate_set_hdate (&h, 1, 14, year);
			print_tabular_hmonth(opt_d, lat, lon, tz, opt_s, opt_h, opt_o, opt_r,
					 opt_R, opt_H, opt_c, opt_t, 14, year);
		}
		else
		{
			print_tabular_hmonth(opt_d, lat, lon, tz, opt_s, opt_h, opt_o, opt_r,
					 opt_R, opt_H, opt_c, opt_t, month, year);
		}
		month++;
	}
	return 0;
}



/************************************************************
* print one Gregorian year - regular output
************************************************************/
int
print_gyear (int opt_d, int opt_S,
			double lat, double lon, int tz, int opt_s, int opt_h, int opt_o, int opt_r,
			int opt_R, int opt_j, int opt_H, int opt_i, int opt_c,
			int opt_t, int year)
{

	int month = 1;

	/* print year header */
	if (!opt_i && !opt_S)
		printf ("%d:\n", year);

	/* print year months */
	while (month < 13)
	{
		print_gmonth (opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o, opt_r,
					 opt_R, opt_j, opt_H, opt_i, opt_c, opt_t, month, year);
		month++;
	}

	return 0;
}



/************************************************************
* print one hebrew year - regular output
************************************************************/
int
print_hyear (int opt_d, int opt_S,
				   double lat, double lon, int tz,
				   int opt_s, int opt_h, int opt_o, int opt_r, int opt_R, int opt_j,
				   int opt_H, int opt_i, int opt_c, int opt_t, int year)
{
	hdate_struct h;
	int month = 1;

	/* print year header */
	if (!opt_i && !opt_S)
		printf ("%s:\n", hdate_get_int_string (year));

	/* print year months */
	while (month < 13)
	{
		/* get date of month start */
		hdate_set_hdate (&h, 1, month, year);

		/* if leap year, print both Adar months */
		if (h.hd_size_of_year > 365 && month == 6)
		{
			hdate_set_hdate (&h, 1, 13, year);
			print_hmonth (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
							opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t,
							13, year);
			hdate_set_hdate (&h, 1, 14, year);
			print_hmonth (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
							opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t,
							14, year);
		}
		else
		{
			print_hmonth (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
							opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t,
							month, year);
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
int
main (int argc, char *argv[])
{
	hdate_struct h;				/* The Hebrew date */
	int c;

	int day;					/* The Gregorian date */
	int month;
	int year;

	/* hebcal style command line options */
	/* char *progname = argv[0]; */
	int opt_s = 0;				/* -s option sunrise/set times */
	int opt_c = 0;				/* -c option shabat enter/leave times */
	int opt_t = 0;				/* -t option print times of day */
	int opt_S = 0;				/* -S Short format flag */
	int opt_h = 0;				/* -h option holidays */
	int opt_H = 0;				/* -H option just holidays */
	int opt_r = 0;				/* -r option reading */
	int opt_R = 0;				/* -R option just reading */
	int opt_j = 0;				/* -j option Julian day number */
	int opt_d = 0;				/* -d option diaspora */
	int opt_i = 0;				/* -i option iCal */
	int opt_o = 0;				/* -o option Sfirat Haomer */

	int opt_l = 0;				/* -l option latitude */
	double lat = BAD_COORDINATE;	/* set to this value for error handling */
	int opt_L = 0;				/* -L option longitude */
	double lon = BAD_COORDINATE;	/* set to this value for error handling */

	int opt_z = 0;				/* -z option Time Zone, default to system local time */
	int tz;
	/* note: other (long) options will be defined
	   globally at beginning of this file */

	/* support for long options */
	int option_index = 0;
	static struct option long_options[] = {
		{"version", 0, 0, 0},
		{"help", 0, 0, 0},
		{"hebrew", 0, 0, 0},
		{"yom", 0, 0, 0},
		{"leshabbat", 0, 0, 0},
		{"leseder", 0, 0, 0},
		{"table",0,0,0},
		{"sunset-aware",0,0,0},
		{0, 0, 0, 0}
		};

	int error_detected = 0;		/* exit after reporting ALL bad parms */

	/* init locale */
	/* WHY?? -- reconsider this, or at least test it out */
	setlocale (LC_ALL, "");



/************************************************************
* begin command line parsing
************************************************************/
	while ((c = getopt_long(argc, argv, "sctShHorRjdil:L:z:?",
						long_options, &option_index)) != -1)
	{
		switch (c)
		{
		case 0: /* long options */
			switch (option_index)
			{
			case 0:	print_version (); exit (0); break;
			case 1:	print_help (); exit (0); break;
			case 2: opt_hebrew = 1; break;
			case 3:
				opt_yom = 1;
				opt_hebrew = 1;
				break;
			case 4:
				opt_leShabbat = 1;
				opt_yom = 1;
				opt_hebrew = 1;
				break;
			case 5:
				opt_leSeder = 1;
				opt_yom = 1;
				opt_hebrew = 1;
				break;
			case 6:
				opt_tablular_output = 1;
				break;
			case 7:
				opt_sunset_aware = 1;
				break;
			}
			break;
		case 's': opt_s = 1; break;
		case 'c': opt_c = 1; break;
		case 't': opt_t = 1; break;
		case 'S': opt_S = 1; break;
		case 'H': opt_H = 1;	/* why no break? */
								/* it seems ok 'H' is used only to abort
									a print, so it always needs 'h' set */
		case 'h': opt_h = 1; break;
		case 'o': opt_o = 1; break;
		case 'R': opt_R = 1;
		case 'r': opt_r = 1; break;
		case 'j': opt_j = 1; break;
		case 'd': opt_d = 1; break;
		case 'i': opt_i = 1; break;
		case 'l':
			if (optarg)
			{
				lat = (double) atof (optarg);
				if ((lat < -180) || (lat > 180))
				{
					print_parm_error("lattitude");
					error_detected = 1;
				}
				else opt_l = 1;
			}
			break;
		case 'L':
			if (optarg)
			{
				lon = (double) atof (optarg);
				if ((lon < -90) || (lon > 90))
				{
					print_parm_error("longitude");
					error_detected = 1;
				}
				else opt_L = 1;
			}
			break;
		case 'z':
			if (optarg)
			{
				tz = atoi (optarg);
				if ((tz < -11) || (tz > 11))
				{
					print_parm_error("time zone");
					error_detected = 1;
				}
				else opt_z = 1;
			}
			break;
		case '?':
		default:
			print_help (); exit (0); break;
		}
	}
	/************************************************************
	* begin validating location parameters - lat, lon, tz
	************************************************************/
	/* only if we need the information 	*/
	if ( (opt_s) || (opt_t) || (opt_c) )
	{

		/* lattitude and longitude must be paired */
		if ( (opt_l) && (!opt_L) )
		{
			error(0,0,"error: valid longitude parameter missing for given lattitude");
			error_detected = 1;
		}
		else if ( (opt_L) && (!opt_l) )
		{
			error(0,0,"error: valid lattitude parameter missing for given longitude");
			error_detected = 1;
		}

		/* if no timezone entered, choose guess method */
		if (!opt_z)
		{
			tzset();
			tz = timezone /-3600;
			if ( (opt_l) && (opt_L) && ( ((lon/15)-tz) > DELTA_LONGITUDE ) )
			{
				/*  reject system local timezone, because it's
					too far from the longitude explicitly provided
					by the user; guess based on longitude entered */
				tz = lon /15;
			}
			print_timezone_warning( tz );
			if ( (lat==BAD_COORDINATE) && (lon==BAD_COORDINATE) )
			{
				print_default_location_warning( tz );
				set_default_location( tz, &lat, &lon );
			}
			printf("\n");
		}
		else
		{	/* we have timezone, what about lattitude and longitude? */
			if ( (opt_L) && (opt_l) )
			{
				/* sanity-check longitude versus timezone */
				if ( abs( ( tz * 15 ) - lon ) > DELTA_LONGITUDE )
				{
					error(0,0,"time zone value of %d is incompatible with a longitude of %.3f degrees", tz, lon);
					error_detected = 1;
				}
			}
			else
			{
				print_default_location_warning( tz );
				set_default_location( tz, &lat, &lon );
				printf("\n");
			}
		}
	}

	/* exit after reporting all bad parameters found */
	if (error_detected) exit(0);
	/************************************************************
	* end validating location parameters - lat, lon, tz
	************************************************************/

	/************************************************************
	* option "s" is a subset of option "t"
	************************************************************/
	if (opt_s && opt_t) opt_s=0;


/************************************************************
* begin parse input date
************************************************************/
	/* parse input date - no date entered */
	if (argc == optind)
	{

		/* set date for today */
		hdate_set_gdate (&h, 0, 0, 0);

		if (opt_tablular_output)
		{
			print_tabular_day(&h, opt_d, lat, lon, tz,
						opt_s, opt_h, opt_o, opt_r, opt_R,
						opt_H, opt_c, opt_t);
		}
		else
		{
			if (opt_i) print_ical_header ();
			else if (opt_sunset_aware)
				opt_print_tomorrow = check_for_sunset(&h, lat, lon);
			print_day (&h, opt_d, opt_S, lat, lon, tz,
						opt_s, opt_h, opt_o, opt_r, opt_R, opt_j,
						opt_H, opt_i, opt_c, opt_t);
			if (opt_i) print_ical_footer ();
		}
		exit (0);
	}

/************************************************************
* parse input date - only year or julian day number entered
************************************************************/
	else if (argc == (optind + 1))
	{
		/* get year */
		year = atoi (argv[optind]);

		/* handle error */
		if (year <= 0) { print_parm_error("year"); exit (0); }


	/************************************************************
	* process single Julian day
	************************************************************/
		if (year > 348021)
		{
			hdate_set_jd (&h, year);

			if (opt_tablular_output)
			{
				print_tabular_day(&h, opt_d, lat, lon, tz,
							opt_s, opt_h, opt_o, opt_r, opt_R,
							opt_H, opt_c, opt_t);
			}
			else
			{
				if (opt_i) print_ical_header ();
				print_day (&h, opt_d, opt_S, lat, lon, tz,
							opt_s, opt_h, opt_o, opt_r, opt_R, opt_j,
							opt_H, opt_i, opt_c, opt_t);
				if (opt_i) print_ical_footer ();
			}
			exit (0);

		}


	/************************************************************
	* process entire Hebrew year
	************************************************************/
		else if (year > 3000)
		{
			if (opt_tablular_output)
			{
				print_tabular_hyear(opt_d, lat, lon, tz,
							opt_s, opt_h, opt_o, opt_r, opt_R,
							opt_H, opt_c, opt_t, year);
			}
			else
			{
				if (opt_i) print_ical_header ();
				print_hyear (opt_d, opt_S, lat, lon, tz, opt_s,
						opt_h, opt_o, opt_r, opt_R, opt_j, opt_H, opt_i,
						opt_c, opt_t, year);
				if (opt_i) print_ical_footer ();
			}
			exit (0);
		}
	/************************************************************
	* process entire Gregorian year
	************************************************************/
		else
		{
			if (opt_tablular_output)
			{
				print_tabular_gyear(opt_d, lat, lon, tz,
							opt_s, opt_h, opt_o, opt_r, opt_R,
							opt_H, opt_c, opt_t, year);
			}
			else
			{
				if (opt_i) print_ical_header ();

				print_gyear (opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
							opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t, year);

				if (opt_i) print_ical_footer ();
			}
			exit (0);
		}
	}


/************************************************************
* parse input date - only month and year entered
************************************************************/
	else if (argc == (optind + 2))
	{

		/* get year */
		year = atoi (argv[optind + 1]);
		month = atoi (argv[optind]);

		/* handle errors */
		if (year <= 0) { print_parm_error("year"); exit (0); }


	/************************************************************
	* process entire Hebrew month
	************************************************************/
		if (year > 3000)
		{
			/* begin bounds check for month */
			if ((month <= 0) || (month > 14))
				{ print_parm_error("month"); exit (0); }
			hdate_set_hdate (&h, 1, 1, year);
			if ((h.hd_size_of_year <365) && (month >12))
				{ print_parm_error("month"); exit (0); }
			/* end bounds check for month */


			if (opt_tablular_output)
			{
				/* get date of month start */
				hdate_set_hdate (&h, 1, month, year);

				/* if leap year, print both Adar months */
				if (h.hd_size_of_year > 365 && month == 6)
				{
					hdate_set_hdate (&h, 1, 13, year);
					print_tabular_hmonth (opt_d, lat, lon, tz, opt_s, opt_h, opt_o,
									opt_r, opt_R, opt_H, opt_c, opt_t,
									13, year);
					hdate_set_hdate (&h, 1, 14, year);
					print_tabular_hmonth (opt_d, lat, lon, tz, opt_s, opt_h, opt_o,
									opt_r, opt_R, opt_H, opt_c, opt_t,
									14, year);
				}
				else
				{
					print_tabular_hmonth (opt_d, lat, lon, tz, opt_s, opt_h, opt_o,
									opt_r, opt_R, opt_H, opt_c, opt_t,
									month, year);
				}
			}
			else
			{
				if (opt_i) print_ical_header ();

				/* get date of month start */
				hdate_set_hdate (&h, 1, month, year);

				/* if leap year, print both Adar months */
				if (h.hd_size_of_year > 365 && month == 6)
				{
					hdate_set_hdate (&h, 1, 13, year);
					print_hmonth (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
									opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t,
									13, year);
					hdate_set_hdate (&h, 1, 14, year);
					print_hmonth (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
									opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t,
									14, year);
				}
				else
				{
					print_hmonth (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
									opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t,
									month, year);
				}
				if (opt_i)
					print_ical_footer ();
			exit (0);
			}
		}

	/************************************************************
	* process entire Gregorian month
	************************************************************/
		else
		{
			if ((month <= 0) || (month > 12))
				{ print_parm_error("month"); exit (0); }

			if (opt_tablular_output)
			{
				print_tabular_gmonth(opt_d, lat, lon, tz,
						opt_s, opt_h, opt_o, opt_r, opt_R,
						opt_H, opt_c, opt_t, month, year);
			}

			else
			{
				if (opt_i)print_ical_header ();

				print_gmonth (opt_d, opt_S, lat, lon,  tz,
						opt_s, opt_h, opt_o, opt_r, opt_R, opt_j,
						opt_H, opt_i, opt_c, opt_t,	month, year);

				if (opt_i) print_ical_footer ();
			}
			exit (0);
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
		if (year <= 0) { print_parm_error("year"); exit (0); }


	/************************************************************
	* get Hebrew date
	************************************************************/
		if (year > 3000)
		{
			/* begin bounds check for month */
			if ((month <= 0) || (month > 14))
				{ print_parm_error("month"); exit (0); }
			hdate_set_hdate (&h, 1, 1, year);
			if ((h.hd_size_of_year <365) && (month >12))
				{ print_parm_error("month"); exit (0); }
			/* end bounds check for month */

			if ((day <= 0) || (day > 30))
				{ print_parm_error("day"); exit (0); }
				/* do better bounds checking.. */

			hdate_set_hdate (&h, day, month, year);
		}

	/************************************************************
	* get Gregorian date
	************************************************************/
		else
		{
			if ((month <= 0) || (month > 12))
				{ print_parm_error("month"); exit (0); }
			if ((day <= 0) || (day > 31))
				{ print_parm_error("day"); exit (0); }
				/* do better bounds checking.. */

			hdate_set_gdate (&h, day, month, year);
		}

	/************************************************************
	* process a single date
	************************************************************/
		if (opt_tablular_output)
		{
			print_tabular_day(&h, opt_d, lat, lon, tz,
						opt_s, opt_h, opt_o, opt_r, opt_R,
						opt_H, opt_c, opt_t);
		}

		else
		{
			if (opt_i) print_ical_header ();
			print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
					   opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t);

			if (opt_i) print_ical_footer ();
		}
		exit (0);
	}


/************************************************************
* parse input date - too many fields were received
************************************************************/
	else
	{
		error(0,0,"too many arguments; after options max is dd mm yyyy");
		exit (0);
	}

	return 0;
}
