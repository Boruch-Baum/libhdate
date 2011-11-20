/* hdate.c
 * Example file for libhdate. 
 * 
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hdate.c -o hdate
 * 
 * Copyright:  2004 (c) Yaacov Zamir 
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

/* TODO:
	By the way, refer to man 5 locale "LC_TIME" for
	internationalizations of time/date formats, and the
	example at the end of man 3 printf for using it
*/


#include <stdio.h>				/* For printf */
#include <hdate.h>				/* For hebrew date */
#include <stdlib.h>				/* For atoi, getenv, setenv */
#include <locale.h>				/* For setlocale */
/* #include <unistd.h>		 For getopt */
#include <getopt.h>		/* For getopt_long */

#define FALSE 0
#define TRUE -1

/* FIXME: global var - ugly ! */
int iCal_uid_counter = 0;

/* Defining option variables globally */
int opt_force_hebrew = 0;
int opt_force_yom = 0;
int opt_force_leShabbat = 0;
int opt_force_leSeder = 0;

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

	printf ("USAGE: hdate [options] [-l xx -L yy] [[[day] month] year]\n");
	printf ("       hdate [options] [-l xx -L yy] [julian_day]\n\n");

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
	printf ("   -z nn  timezone, +/-GMT\n\n");
	printf ("   -l xx  latitude xx degrees. Negative values are South.\n");
	printf ("   -L yy  longitude yy degrees. Negative values are West.\n\n");

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
--force-hebrew
--force-yom (implies --force-hebrew)
--force-leshabbat (implies --force-hebrew and --force-yom)
--force-leseder (implies --force-hebrew and --force-yom)
*/


/* print ical header */
int
print_ical_header ()
{
	/* Print start of iCal format */
	printf ("BEGIN:VCALENDAR\n");
	printf ("VERSION:2.0\n");
	printf ("CALSCALE:GREGORIAN\n");
	printf ("METHOD:PUBLISH\n");

	return 0;
}

/* print ical footer */
int
print_ical_footer ()
{
	/* Print end of iCal format */
	printf ("END:VCALENDAR\n");

	return 0;
}


int find_shabbat (hdate_struct * h, int opt_d)
/*	this function is to become obsolete - I want a feature
	for this standard in libhdate */
{
	hdate_struct coming_Shabbat;
	
	hdate_set_jd (&coming_Shabbat, h->hd_jd+(7-h->hd_dw));
	/*	this return value is the reading number,
		used to print parshiot */
	return hdate_get_parasha (&coming_Shabbat, opt_d);
	}


/* print one day - gregorian and hebrew date */
int
print_date (hdate_struct * h, int opt_S, int opt_i, int opt_d)
{
	char *locale;
	char *language;
	char *for_day_of_g;	/* Hebrew prefix for day of week */
	char *bet_g;		/* Hebrew prefix for Gregorian month */
	char *bet_h;		/* Hebrew prefix for Hebrew month */
	char *psik_mark;	/* --force-yom */
	char *for_day_of_h;	/* --force-yom */
	char *for_week_of;	/* --force-leshabbat --force-leseder */
	int  is_parasha_read;


	
/* TODO - learn about this ENABLE_NLS option */
#ifdef ENABLE_NLS
	locale = setlocale (LC_MESSAGES, NULL);
	language = getenv ("LANGUAGE");
#else
	locale = NULL;
	language = NULL;
#endif

	/* preliminary - prepare Hebrew prefixes if forcing any Hebrew */
	for_week_of="";	for_day_of_h="";	psik_mark="";
	language=getenv("LANGUAGE");
	if (!opt_force_hebrew) bet_h="";
	else
		{
		bet_h="ב";
		/* preliminary - prepare Yom prefix */
		if (opt_force_yom)
			{
			for_day_of_h="יום";
			psik_mark=",";
			if		(opt_force_leShabbat)
				{
				if (h->hd_dw==7)		for_week_of="פרשת";
				else					for_week_of="לשבת";
				}
			else if (opt_force_leSeder)	for_week_of="לסדר";
			}
		}
	
	/*	preliminary - prepare the Hebrew prefixes
		for the Gregorian month and day of week */
	if ((locale && (locale[0] == 'h') &&
		(locale[1] == 'e')) ||
		(language && (language[0] == 'h') && (language[1] == 'e'))
		)
		{ bet_g="ב"; for_day_of_g="יום ";}
	else
		{ bet_g="";  for_day_of_g="";}
	
	/* Three major print formats: iCal, short, long */
	if (opt_i)		/* iCal format */
	{
		if (opt_force_hebrew)
				setenv("LANGUAGE", "he_IL.UTF-8", 1);
		printf ("%s%s %s%s ",
				for_day_of_g,
				hdate_get_int_string (h->hd_day),
				bet_h,
				hdate_get_hebrew_month_string (h->hd_mon, opt_S));
		printf ("%s", hdate_get_int_string (h->hd_year));
	}
	else if (opt_S) /* short format */
	{
		printf ("%d.%d.%d ", h->gd_day, h->gd_mon, h->gd_year);

		/* check for hebrew locale */
		/* we performed this check a few lines up, so all we need to do
			is test for bet="ב"
		if (!((locale && (locale[0] == 'h') && (locale[1] == 'e')) || 
			  (language && (language[0] == 'h') && (language[1] == 'e'))))
		*/
		if ((!bet_g) && (!opt_force_hebrew))
		{	/* non hebrew numbers */

			printf ("%d", h->hd_day);

			/* Begin - print ordinal number of month in short format */
			/* It says correctly below FIXME; however, printing in
				ordinal format isn't short, it's long!. For now, I'm
				commenting out the code entirely; it can be restored
				as a separate option
			*/
			/* By the way, refer to man 5 locale "LC_TIME" for
				internationalizations of time/date formats, and the
				example at the end of man 3 printf for using it
			*/
			/* FIXME: this only works for english :-( */
			/* check for english locale */
			/*
			if ((!locale && !language) ||
				(language && (language[0] == 'C')) ||
				(!language && locale && (locale[0] == 'C')) ||
				(language && (language[0] == 'e') && (language[1] == 'n'))
				|| (!language && locale && (locale[0] == 'e')
					&& (locale[1] == 'n')))
			{
				switch (h->hd_day)
				{
					case 1:
					case 21:
					case 31:
						printf ("st of");
						break;
					case 2:
					case 22:
						printf ("nd of");
						break;
					case 3:
					case 23:
						printf ("rd of");
						break;
					default:
						printf ("th of");
				}
			} */

		}
		else 		/* Hebrew in short format */
		{
			setenv("LANGUAGE", "he_IL.UTF-8", 1);
			printf ("%s", hdate_get_int_string (h->hd_day));
		}

		printf (" %s%s, ", bet_h, hdate_get_hebrew_month_string (h->hd_mon, opt_S));
		printf ("%s\n", hdate_get_int_string (h->hd_year));
	}
	else 			/* normal (long) format) */
	{
		printf ("%s%s, %d %s%s %d, ",
				for_day_of_g,
				hdate_get_day_string (h->hd_dw, opt_S),
				h->gd_day,
				bet_g,
				hdate_get_month_string (h->gd_mon, opt_S), h->gd_year);
		if (opt_force_hebrew)
			{
			setenv("LANGUAGE", "he_IL.UTF-8", 1);
			if (opt_force_yom)
				{
				printf("%s %s", for_day_of_h, hdate_get_day_string (h->hd_dw, opt_S));
				if ((opt_force_leShabbat) || (opt_force_leSeder))
				{
					is_parasha_read = find_shabbat(h, opt_d);
					if (is_parasha_read)
							printf(" %s %s", for_week_of, hdate_get_parasha_string (is_parasha_read, opt_d));
				}
				printf ("%s ", psik_mark);
			}
		}
		printf ("%s %s%s ",
				hdate_get_int_string (h->hd_day),
				bet_h,
				hdate_get_hebrew_month_string (h->hd_mon, opt_S));
		printf ("%s\n", hdate_get_int_string (h->hd_year));
	}
	/* restore default language environment variable */
	setenv("LANGUAGE", language, 1);
	return 0;
}

/* print one day - sunrise/set times */
int
print_sunrise (hdate_struct * h, double lat, double lon, int tz, int opt_i)
{
	int sunrise, sunset;

	/* get times */
	hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
							&sunrise, &sunset);

	sunset = sunset + tz * 60;
	sunrise = sunrise + tz * 60;

	/* print sunset/rise times */
	printf ("sunrise: %02d:%02d\nsunset: %02d:%02d\n",
			sunrise / 60, sunrise % 60, sunset / 60, sunset % 60);

	return 0;
}

/* print one day - day times */
int
print_times (hdate_struct * h, double lat, double lon, int tz, int opt_i)
{
	int sun_hour, first_light, talit, sunrise;
	int midday, sunset, first_stars, three_stars;

	/* get times */
	hdate_get_utc_sun_time_full (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
								 &sun_hour, &first_light, &talit, &sunrise,
								 &midday, &sunset, &first_stars, &three_stars);

	first_light = first_light + tz * 60;
	talit = talit + tz * 60;
	sunrise = sunrise + tz * 60;
	midday = midday + tz * 60;
	sunset = sunset + tz * 60;
	first_stars = first_stars + tz * 60;
	three_stars = three_stars + tz * 60;

	/* print sunset/rise times */	
	printf ("first light: %02d:%02d\ntalit: %02d:%02d\nsun rise: %02d:%02d\nmid day: %02d:%02d\n",
			first_light / 60, first_light % 60, talit / 60, talit % 60,
			sunrise / 60, sunrise % 60, midday / 60, midday % 60);
	printf ("sun set: %02d:%02d\nfirst stars: %02d:%02d\nthree stars: %02d:%02d\n",
			sunset / 60, sunset % 60, first_stars / 60, first_stars % 60,
			three_stars / 60, three_stars % 60);

	return 0;
}

/* print one day - holiday */
int
print_holiday (hdate_struct * h, int opt_d, int opt_S, int opt_i)
{
	int holyday;

	holyday = hdate_get_holyday (h, opt_d);

	if (holyday)
	{
		/* print holyday */
		printf ("%s\n", hdate_get_holyday_string (holyday, opt_S));
	}

	return 0;
}

/* print day in the omer */
int
print_omer (hdate_struct * h)
{
	int omer_day;
	
	omer_day = hdate_get_omer_day(h);
	
	if (omer_day != 0)
		printf ("today is day %s \n", hdate_get_omer_string (omer_day));
	
	return 0;
}

/* print one day - reading */
int
print_reading (hdate_struct * h, int opt_d, int opt_S, int opt_i)
{
	int reading;

	reading = hdate_get_parasha (h, opt_d);

	if (reading)
	{
		/* print parasha */
		printf ("Parashat %s\n", hdate_get_parasha_string (reading, opt_S));
	}

	return 0;
}

/* print one day - candles */
int
print_candles (hdate_struct * h, double lat, double lon, int tz, int opt_i)
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
	}

	return 0;
}

/* print one day - all */
int
print_day (hdate_struct * h,
		   int opt_d, int opt_S,
		   double lat, double lon, int tz, int opt_s, int opt_h, int opt_o, int opt_r,
		   int opt_R, int opt_j, int opt_H, int opt_i, int opt_c, int opt_t)
{
	
	/* iCal format require \ before comma */
	/* BUT ... who wants the comma? */
	/*
	char separator[5];
	if (opt_i) 
		snprintf(separator, 5, "\\, ");
	else
		snprintf(separator, 5, ", ");
	*/
	
	/* check for just parasha or holiday flag */
	if (opt_R && opt_H &&
		!hdate_get_parasha (h, opt_d) && !hdate_get_holyday (h, opt_d))
		return 0;
	if (opt_R && !opt_H && !hdate_get_parasha (h, opt_d))
		return 0;
	if (opt_H && !opt_R && !hdate_get_holyday (h, opt_d))
		return 0;

	/* check for iCal format */
	if (opt_i)
	{
		/* FIXME: older versions of mozilla calendar and evulotion 
		 * needed this hack */
		/* print_ical_header (); */

		printf ("BEGIN:VEVENT\n");
		printf ("UID:%d\n", ++iCal_uid_counter);
		printf ("DTSTART;VALUE=DATE:%04d%02d%02d\n", h->gd_year,
				h->gd_mon, h->gd_day);
		printf ("SUMMARY:");
	}

	/* print Julian day number */
	if (opt_j)
	{
		printf ("JDN-%d ", h->hd_jd);
	}

	/* print the day */
	print_date (h, opt_S, opt_i, opt_d);
	
	if (opt_s)
	{
		/* why this pesky leading comma? */
		/* printf (separator); */
		print_sunrise (h, lat, lon, tz, opt_i);
	}
	if (opt_t)
	{
		/* why this pesky leading comma? */
		/* printf (separator); */
		print_times (h, lat, lon, tz, opt_i);
	}
	if (opt_h && hdate_get_holyday (h, opt_d) != 0)
	{
		/* why this pesky leading comma? */
		/* printf (separator); */
		print_holiday (h, opt_d, opt_S, opt_i);
	}
	if (opt_o && hdate_get_omer_day(h) != 0)
	{
		/* Finally, a possible case of needing
		a leading comma - first few days of omer,
		during Pesach ... if you want all that
		information on one line. However, in other
		other cases we're printing discrete info
		on separate lines
		printf (separator);
		*/
		print_omer (h);
	}
	if (opt_r && hdate_get_parasha (h, opt_d) != 0)
	{
		/* why this pesky leading comma? */
		/* printf (separator); */
		print_reading (h, opt_d, opt_S, opt_i);
	}
	if (opt_c)
	{
		/* Another possible case of needing a
		leading comma - to have both parsha and
		candlelighting on same line
		printf (separator);
		*/
		print_candles (h, lat, lon, tz, opt_i);
	}

	if (opt_r || opt_h || opt_s || opt_t || opt_i || opt_c || opt_o)
	{
/*		printf ("\n");*/
	}

	/* check for iCal format */
	if (opt_i)
	{
		printf ("CLASS:PUBLIC\n");
		printf ("DTEND;VALUE=DATE:%04d%02d%02d\n", h->gd_year,
				h->gd_mon, h->gd_day);
		printf ("CATEGORIES:Holidays\n");
		printf ("END:VEVENT\n");
		/* FIXME: older versions of mozilla calendar and evulotion 
		 * needed this hack */
		/* print_ical_footer (); */
	}

	return 0;
}

/* print one month - all */
int
print_month (int opt_d, int opt_S,
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

/* print one hebrew month - all */
int
print_hebrew_month (int opt_d, int opt_S,
					double lat, double lon, int tz,
					int opt_s, int opt_h, int opt_o, int opt_r, int opt_R, int opt_j,
					int opt_H, int opt_i, int opt_c, int opt_t, int month,
					int year)
{
	hdate_struct h;
	int jd;

	/* get date of month start */
	hdate_set_hdate (&h, 1, month, year);
	jd = h.hd_jd;

	if (h.hd_size_of_year > 365 && month == 6)
	{							/* adar of meoberet */
		/* print Adar I and Adar II month days */
		hdate_set_hdate (&h, 1, 13, year);
		jd = h.hd_jd;

		/* print month header */
		if (!opt_i && !opt_S)
			printf ("\n%s:\n", hdate_get_hebrew_month_string (h.hd_mon, opt_S));

		/* print adar I days */
		while (h.hd_mon == 13)
		{
			print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s,
					   opt_h, opt_o, opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t);

			jd++;
			hdate_set_jd (&h, jd);
		}

		hdate_set_hdate (&h, 1, 14, year);
		jd = h.hd_jd;

		/* print month header */
		if (!opt_i && !opt_S)
			printf ("\n%s:\n", hdate_get_hebrew_month_string (h.hd_mon, opt_S));

		/* print adar II days */
		while (h.hd_mon == 14)
		{
			print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s,
					   opt_h, opt_o, opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t);

			jd++;
			hdate_set_jd (&h, jd);
		}
	}
	else
	{
		/* print month header */
		if (!opt_i && !opt_S)
			printf ("\n%s:\n", hdate_get_hebrew_month_string (h.hd_mon, opt_S));

		/* print month days */
		while (h.hd_mon == month)
		{
			print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s,
					   opt_h, opt_o, opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t);

			jd++;
			hdate_set_jd (&h, jd);
		}
	}

	return 0;
}

/* print one gregorian year - all */
int
print_year (int opt_d, int opt_S,
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
		print_month (opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o, opt_r,
					 opt_R, opt_j, opt_H, opt_i, opt_c, opt_t, month, year);
		month++;
	}

	return 0;
}

/* print one hebrew year - all */
int
print_hebrew_year (int opt_d, int opt_S,
				   double lat, double lon, int tz,
				   int opt_s, int opt_h, int opt_o, int opt_r, int opt_R, int opt_j,
				   int opt_H, int opt_i, int opt_c, int opt_t, int year)
{
	int month = 1;

	/* print year header */
	if (!opt_i && !opt_S)
		printf ("%s:\n", hdate_get_int_string (year));

	/* print year months */
	while (month < 13)
	{
		print_hebrew_month (opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
							opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t,
							month, year);
		month++;
	}

	return 0;
}

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

	double lat = 32.0;			/* -l option default to Tel aviv latitude */
	double lon = 34.0;			/* -L option default to Tel aviv longitude */
	int tz = 2;					/* -z option default to Tel aviv time zone */
	/* note: other (long) options will be defined
	   globally at beginning of this file */

	/* support for long options */
	int option_index = 0;
	static struct option long_options[] = {
		{"version", 0, 0, 0},
		{"help", 0, 0, 0},
		{"force-hebrew", 0, 0, 0},
		{"force-yom", 0, 0, 0},
		{"force-leshabbat", 0, 0, 0},
		{"force-leseder", 0, 0, 0},
		{0, 0, 0, 0}
		};

	/* init locale */
	setlocale (LC_ALL, "");

	/* command line parsing */
	/*while ((c = getopt (argc, argv, "sctShHorRjdil:L:z:")) != -1)*/
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
			case 2: opt_force_hebrew = 1; break;
			case 3:
				opt_force_yom = 1;
				opt_force_hebrew = 1;
				break;
			case 4:
				opt_force_leShabbat = 1;
				opt_force_yom = 1;
				opt_force_hebrew = 1;
				break;
			case 5:
				opt_force_leSeder = 1;
				opt_force_yom = 1;
				opt_force_hebrew = 1;
				break;
			}
			break;
		case 's': opt_s = 1; break;
		case 'c': opt_c = 1; break;
		case 't': opt_t = 1; break;
		case 'S': opt_S = 1; break;
		case 'H': opt_H = 1;	/* why no break? */
		case 'h': opt_h = 1; break;
		case 'o': opt_o = 1; break;	
		case 'R': opt_R = 1;	/* why no break? */
		case 'j': opt_j = 1;	/* why no break? */
		case 'r': opt_r = 1; break;
		case 'd': opt_d = 1; break;
		case 'i': opt_i = 1; break;
		case 'l':
			if (optarg)
				lat = (double) atof (optarg);
			break;
		case 'L':
			if (optarg)
				lon = (double) atof (optarg);
			break;
		case 'z':
			if (optarg)
				tz = atoi (optarg);
			break;
		case '?':
		default:
			print_help (); exit (0); break;
		}
	}

	/* disregard opt_s because it is a subset of opt_t */
	if (opt_s && opt_t) opt_s=0;
	
	/* Get calendar */
	if (argc == optind)
	{							/* no date entered */
		/* set date for today */
		hdate_set_gdate (&h, 0, 0, 0);

		if (opt_i)
			print_ical_header ();

		print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
				   opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t);

		if (opt_i)
			print_ical_footer ();

		exit (0);
	}
	else if (argc == (optind + 1))
	{	/* only year or julian day number */
		/* get year */
		year = atoi (argv[optind]);

		if (year <= 0)
		{						/* error */
			print_help ();
			exit (0);
		}
		if (year > 100000)
		{						/* julian day number */
			/* year is julian day number */
			hdate_set_jd (&h, year);

			if (opt_i)
				print_ical_header ();

			print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
					   opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t);

			if (opt_i)
				print_ical_footer ();

			exit (0);
		}
		else if (year > 3000)
		{						/* hebrew year */
			if (opt_i)
				print_ical_header ();

			print_hebrew_year (opt_d, opt_S, lat, lon, tz, opt_s,
							   opt_h, opt_o, opt_r, opt_R, opt_j, opt_H, opt_i,
							   opt_c, opt_t, year);

			if (opt_i)
				print_ical_footer ();

			exit (0);
		}
		else
		{
			if (opt_i)
				print_ical_header ();

			print_year (opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
						opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t, year);

			if (opt_i)
				print_ical_footer ();

			exit (0);
		}
	}
	else if (argc == (optind + 2))
	{							/*only month and year */
		/* get year */
		year = atoi (argv[optind + 1]);
		month = atoi (argv[optind]);

		if (year <= 0)
		{						/* error */
			print_help ();
			exit (0);
		}
		if (year > 3000)
		{						/* hebrew year */
			if (opt_i)
				print_ical_header ();

			print_hebrew_month (opt_d, opt_S, lat, lon, tz, opt_s,
								opt_h, opt_o, opt_r, opt_R, opt_j, opt_H, opt_i,
								opt_c, opt_t, month, year);

			if (opt_i)
				print_ical_footer ();

			exit (0);
		}
		else
		{
			if (opt_i)
				print_ical_header ();

			print_month (opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
						 opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t,
						 month, year);

			if (opt_i)
				print_ical_footer ();

			exit (0);
		}
	}
	else if (argc == (optind + 3))
	{							/*day month and year */
		/* get year */
		year = atoi (argv[optind + 2]);
		month = atoi (argv[optind + 1]);
		day = atoi (argv[optind]);

		if (year <= 0)
		{						/* error */
			print_help ();
			exit (0);
		}

		/* set date */
		if (year > 3000)
		{						/* hebrew year */
			hdate_set_hdate (&h, day, month, year);
		}
		else
		{
			hdate_set_gdate (&h, day, month, year);
		}

		if (opt_i)
			print_ical_header ();

		print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_o,
				   opt_r, opt_R, opt_j, opt_H, opt_i, opt_c, opt_t);

		if (opt_i)
			print_ical_footer ();

		exit (0);
	}
	else
	{
		print_help ();
		exit (0);
	}

	return 0;
}
