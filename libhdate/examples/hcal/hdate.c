/* hdate.c
 * Example file for libhdate. 
 * 
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hdate.c -o hdate
 * 
 * Copyright:  2004 (c) Yaacov Zamir 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#include <stdio.h>		/* For printf */
#include <hdate.h>		/* For hebrew date */
#include <stdlib.h>		/* For atoi */
#include <locale.h>		/* For setlocale */
#include <unistd.h>		/* For getopt */

#define FALSE 0
#define TRUE -1

/* some useful time zones
 * Eilat 29, -34, 2
 * Haifa 32, -34, 2
 * Jerusalem 31, -35, 2
 * Tel Aviv 32, -34, 2
 */

/* print help */
int
print_help (char *program)
{
	printf ("hdate - example program for libhdate\n\n");

	printf ("USAGE: %s [-sShdr]\n", program);
	printf ("   [-L longitude -l latitude]\n");
	printf ("   [-z timezone]\n");
	printf ("   [[[day] month] year]\n");

	printf ("OPTIONS:\n");
	printf ("   -s : Print sunrise/sunset times.\n");
	printf ("   -S : Print using short format.\n");
	printf ("   -h : Print holidays.\n");
	printf ("   -r : Print weekly reading on saturday.\n");

	printf ("   -d : Use diaspora reading and holidays.\n");
	printf ("   -l xx : Set the latitude for solar calculations to\n");
	printf ("              xx degrees.  Negative values are south.\n");
	printf ("   -L xx : Set the longitude for solar calculations to\n");
	printf ("              xx degrees.  *Negative values are EAST*.\n");
	printf ("        The -l and -L switches must both be used, or not at all.\n");
	printf ("   -z : Use specified timezone,\n");
	
	printf ("   ( default location for sunrise/set is Tel Aviv winter time ).\n");

	return 0;
}

/* print one day - gregorian and hebrew date */
int
print_date (hdate_struct * h, int opt_S)
{
	if (opt_S)
	{
		printf ("%d.%d.%d %s ",
			h->gd_day, h->gd_mon, h->gd_year,
			hdate_get_int_string (h->hd_day));
		printf ("%s\n", hdate_get_hebrew_month_string (h->hd_mon, opt_S));
	}
	else
	{
		printf ("%s, %d %s %d, ",
			hdate_get_day_string (h->hd_dw, opt_S),
			h->gd_day,
			hdate_get_month_string (h->gd_mon, opt_S),
			h->gd_year);
		printf ("%s %s ",
			hdate_get_int_string (h->hd_day),
			hdate_get_hebrew_month_string (h->hd_mon, opt_S));
		printf ("%s\n",
			hdate_get_int_string (h->hd_year));
	}
	
	return 0;
}

/* print one day - sunrise/set times */
int
print_sunrise (hdate_struct * h, double lat, double lon, int tz)
{
	int sunrise, sunset;

	/* get times */
	hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
				&sunrise, &sunset);

	sunset = sunset + tz * 60;
	sunrise = sunrise + tz * 60;

	/* print sunset/rise times */
	printf ("%d:%d - %d:%d ",
		sunrise / 60, sunrise % 60,
		sunset / 60, sunset % 60);

	return 0;
}

/* print one day - holiday */
int
print_holiday (hdate_struct * h, int opt_d, int opt_S)
{
	int holyday;

	holyday = hdate_get_holyday (h, opt_d);

	if (holyday)
	{
		/* print holyday */
		printf ("%s ", hdate_get_holyday_string (holyday, opt_S));
	}

	return 0;
}

/* print one day - reading */
int
print_reading (hdate_struct * h, int opt_d, int opt_S)
{
	int reading;

	reading = hdate_get_parasha (h, opt_d);

	if (reading)
	{
		/* print parash */
		printf ("%s", hdate_get_parasha_string (reading, opt_S));
	}

	return 0;
}

/* print one day - all */
int
print_day (hdate_struct * h,
	   int opt_d, int opt_S,
	   double lat, double lon, int tz, int opt_s, int opt_h, int opt_r)
{
	print_date (h, opt_S);

	if (opt_s)
	{
		print_sunrise (h, lat, lon, tz);
		
		if ((opt_h  && hdate_get_holyday (h, opt_d)) || 
			(opt_r && hdate_get_parasha (h, opt_d)))
				printf (", ");
	}
	if (opt_h)
	{
		print_holiday (h, opt_d, opt_S);
		
		if (opt_r && hdate_get_parasha (h, opt_d) && hdate_get_holyday (h, opt_d))
				printf (", ");
	}
	if (opt_r)
	{
		print_reading (h, opt_d, opt_S);
	}
	
	if (opt_r || opt_h || opt_s)
	{
		printf ("\n");
	}
	
	return 0;
}

/* print one month - all */
int
print_month (int opt_d, int opt_S,
	     double lat, double lon, int tz,
	     int opt_s, int opt_h, int opt_r, int month, int year)
{
	hdate_struct h;
	int jd;

	/* get date of month start */
	hdate_set_gdate (&h, 1, month, year);
	jd = h.hd_jd;

	/* print month header */
	printf ("\n%s:\n", hdate_get_month_string (h.gd_mon, opt_S));

	/* print month days */
	while (h.gd_mon == month)
	{
		print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h,
			   opt_r);

		jd++;
		hdate_set_jd (&h, jd);
	}

	return 0;
}

/* print one year - all */
int
print_year (int opt_d, int opt_S,
	    double lat, double lon, int tz, int opt_s, int opt_h, int opt_r,
	    int year)
{
	int month = 1;

	/* print year header */
	printf ("%d:\n", year);

	/* print year months */
	while (month < 13)
	{
		print_month (opt_d, opt_S, lat, lon, tz, opt_s, opt_h, opt_r,
			     month, year);
		month++;
	}

	return 0;
}

/* print one hebrew month - all */
int
print_hebrew_month (int opt_d, int opt_S,
		    double lat, double lon, int tz,
		    int opt_s, int opt_h, int opt_r, int month, int year)
{
	hdate_struct h;
	int jd;

	/* get date of month start */
	hdate_set_hdate (&h, 1, month, year);
	jd = h.hd_jd;

	if (h.hd_size_of_year > 365 && month == 6)	/* adar of meoberet */
	{
		/* print Adar I and Adar II month days */
		hdate_set_hdate (&h, 1, 13, year);
		jd = h.hd_jd;

		/* print month header */
		printf ("\n%s:\n",
			hdate_get_hebrew_month_string (h.hd_mon, opt_S));

		/* print adar I days */
		while (h.hd_mon == 13)
		{
			print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s,
				   opt_h, opt_r);

			jd++;
			hdate_set_jd (&h, jd);
		}

		hdate_set_hdate (&h, 1, 14, year);
		jd = h.hd_jd;

		/* print month header */
		printf ("\n%s:\n",
			hdate_get_hebrew_month_string (h.hd_mon, opt_S));

		/* print adar II days */
		while (h.hd_mon == 14)
		{
			print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s,
				   opt_h, opt_r);

			jd++;
			hdate_set_jd (&h, jd);
		}
	}
	else
	{
		/* print month header */
		printf ("\n%s:\n",
			hdate_get_hebrew_month_string (h.hd_mon, opt_S));

		/* print month days */
		while (h.hd_mon == month)
		{
			print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s,
				   opt_h, opt_r);

			jd++;
			hdate_set_jd (&h, jd);
		}
	}

	return 0;
}

/* print one hebrew year - all */
int
print_hebrew_year (int opt_d, int opt_S,
		   double lat, double lon, int tz,
		   int opt_s, int opt_h, int opt_r, int year)
{
	int month = 1;

	/* print year header */
	printf ("%s:\n", hdate_get_int_string (year));

	/* print year months */
	while (month < 13)
	{
		print_hebrew_month (opt_d, opt_S, lat, lon, tz, opt_s, opt_h,
				    opt_r, month, year);
		month++;
	}

	return 0;
}

int
main (int argc, char *argv[])
{
	hdate_struct h;		/* The Hebrew date */
	char c;

	int day;		/* The Gregorian date */
	int month;
	int year;

	/* hebcal style command line options */
	char *progname = argv[0];
	int opt_s = 0;		/* -s option sunrise/set times */
	int opt_S = 0;		/* -S Short format flag */
	int opt_h = 0;		/* -h option holidays */
	int opt_r = 0;		/* -r option reading */
	int opt_d = 0;		/* -d option diaspora */

	double lat = 32.0;	/* -l option default to Tel aviv latitude */
	double lon = -34.0;	/* -L option default to Tel aviv longitude */
	int tz = 2;		/* -z option default to Tel aviv time zone */

	/* init locale */
	setlocale (LC_ALL, "");
	
	/* command line parsing */
	while ((c = getopt (argc, argv, "sShrdl:L:z:")) != EOF)
	{
		switch (c)
		{
		case 's':
			opt_s = 1;
			break;
		case 'S':
			opt_S = 1;
			break;
		case 'h':
			opt_h = 1;
			break;
		case 'r':
			opt_r = 1;
			break;
		case 'd':
			opt_d = 1;
			break;
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
		default:
			print_help (argv[0]);
			exit (0);
			break;
		}
	}

	/* Get calendar */
	if (argc == optind)	/* no date entered */
	{
		/* set todays date */
		hdate_set_gdate (&h, 0, 0, 0);

		print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h,
			   opt_r);
		exit (1);
	}
	else if (argc == (optind + 1))	/*only year */
	{
		/* get year */
		year = atoi (argv[optind]);

		if (year <= 0)	/* error */
		{
			print_help (argv[0]);
			exit (0);
		}
		if (year > 3000)	/* hebrew year */
		{
			print_hebrew_year (opt_d, opt_S, lat, lon, tz, opt_s,
					   opt_h, opt_r, year);
			exit (1);
		}
		else
		{
			print_year (opt_d, opt_S, lat, lon, tz, opt_s, opt_h,
				    opt_r, year);
			exit (1);
		}
	}
	else if (argc == (optind + 2))	/*only month and year */
	{
		/* get year */
		year = atoi (argv[optind + 1]);
		month = atoi (argv[optind]);

		if (year <= 0)	/* error */
		{
			print_help (argv[0]);
			exit (0);
		}
		if (year > 3000)	/* hebrew year */
		{
			print_hebrew_month (opt_d, opt_S, lat, lon, tz, opt_s,
					    opt_h, opt_r, month, year);
			exit (1);
		}
		else
		{
			print_month (opt_d, opt_S, lat, lon, tz, opt_s, opt_h,
				     opt_r, month, year);
			exit (1);
		}
	}
	else if (argc == (optind + 3))	/*day month and year */
	{
		/* get year */
		year = atoi (argv[optind + 2]);
		month = atoi (argv[optind + 1]);
		day = atoi (argv[optind]);

		if (year <= 0)	/* error */
		{
			print_help (argv[0]);
			exit (0);
		}

		/* set date */
		if (year > 3000)	/* hebrew year */
		{
			hdate_set_hdate (&h, day, month, year);
		}
		else
		{
			hdate_set_gdate (&h, day, month, year);
		}

		print_day (&h, opt_d, opt_S, lat, lon, tz, opt_s, opt_h,
			   opt_r);

		exit (1);
	}
	else
	{
		print_help (argv[0]);
		exit (0);
	}

	return 0;
}
