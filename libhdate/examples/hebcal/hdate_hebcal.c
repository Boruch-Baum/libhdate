/* hdate_hebcal.c
 * Example file for libhdate. 
 * 
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hdate_hebcal.c -o hdate_hebcal
 * 
 * Copyright:  2005 (c) Yaacov Zamir 
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

#include <stdio.h>  /* For printf */
#include <hdate.h>  /* For hebrew date */
#include <stdlib.h> /* For atoi */
#include <locale.h> /* For setlocale */
#include <unistd.h> /* For getopt */

/* some useful time zones
 * Eilat 29, -34, 2
 * Haifa 32, -34, 2
 * Jerusalem 31, -35, 2
 * Tel Aviv 32, -34, 2
 */

int
main (int argc, char* argv[])
{
	hdate_struct h; 	/* The Hebrew date */
	int jd;
	int reading;
	int holyday;
	int rosh_hodesh;
	int candle_lighting;
	int havdala;
	int event;
	int year;
	int sunset, sunrise;
	char number_suffix[3];
	char c;
	
	/* command line options */
	char *progname=argv[0];
	int opt_c=0; /* -c option candle lighting times */
	int opt_d=0; /* -d option hebrew date for evrey day */
	int opt_D=0; /* -d option hebrew date for event day */
	int opt_e=0; /* -e option use normal date format */
	int opt_h=0; /* -h option do not show holydays */
	int opt_i=0; /* -i option do not use diaspora reading and holyday */
	int opt_r=0; /* -r option use tab as a delimiter */
	char sep = ' '; /* default separator is space */
	int opt_s=0; /* -s option show parasha */
	int opt_x=0; /* -x option do not show new month day */
	double lon = -34.0; /* default to Tel aviv longitude */
	double lat = 32.0; /* default to Tel aviv latitude */
	int tz = 0; /* default to Tel aviv time zone */
	
	while((c=getopt(argc, argv, "cdDehirsxl:L:z:"))!=EOF){
		switch(c){
		case 'c':
			opt_c=1;
			break;
		case 'd':
			opt_d=1;
			break;
		case 'D':
			opt_D=1;
			break;
		case 'e':
			opt_e=1;
			break;
		case 'h':
			opt_h=1;
			break;
		case 'i':
			opt_i=1;
			break;
		case 'r':
			opt_r=1;
			char sep = '\t'; /* separator is tab */
			break;
		case 's':
			opt_s=1;
			break;
		case 'x':
			opt_x=1;
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
		}
	}
	
	argc -= optind;
	
	/* Get calendar hebrew year */
	if (argc == 0) 
		{
			/* set initial date */
			hdate_set_gdate (&h, 0, 0, 0);
			year = h.gd_year;
			hdate_set_gdate (&h, 1, 1, year);
		}
	else if (argc == 1) 
		{
			/* set initial date */
			year = atoi (argv[optind]);
			hdate_set_gdate (&h, 1, 1, year);
		}
	else 
		{
			/* Print help for user and exit */
			printf ("USAGE: %s [-cdDehirsx] [-l latitude -L longitude -z time zone] [year]\n", argv[0]);
			exit (0);
		}
	
	/* Set the locale to C*/ 
	setlocale (LC_ALL,"C");
	
	/* start the loop */
	jd = h.hd_jd;
	
	/* Print the hebcal output */
	while (h.gd_year == year)
		{
			/* get todays events */
			reading = hdate_get_parasha (&h, !opt_i) * opt_s;
			holyday = hdate_get_holyday (&h, !opt_i) * (1 - opt_h);
			rosh_hodesh = (h.hd_day == 1) && !opt_x;
			candle_lighting = (h.hd_dw == 6) && opt_c;
			havdala = (h.hd_dw == 7) && opt_c;
			event = reading || holyday || rosh_hodesh || candle_lighting || havdala;
			
			/* print hebrew date */
			if (opt_d || (opt_D && event))
			{
				/* print the gregorian date */
				if (opt_e)
					printf ("%d.%d.%d%c", 
						h.gd_day, h.gd_mon, h.gd_year, sep); 
				else
					printf ("%d/%d/%d%c", 
						h.gd_mon, h.gd_day, h.gd_year, sep); 
				
				/* print hebrew date */
				switch (h.hd_day % 10)
				{
					case  1:
						number_suffix[0] = 's';
						number_suffix[1] = 't';
						number_suffix[2] = 0;
						break;
					case  2:
						number_suffix[0] = 'n';
						number_suffix[1] = 'd';
						number_suffix[2] = 0;
						break;
					case  3:
						number_suffix[0] = 'r';
						number_suffix[1] = 'd';
						number_suffix[2] = 0;
						break;
					default:
						number_suffix[0] = 't';
						number_suffix[1] = 'h';
						number_suffix[2] = 0;
						break;
				}
				
				printf ("%d%s of %s, %d\n", 
						h.hd_day,
						number_suffix,
						hdate_get_hebrew_month_string (h.hd_mon, 0),
						h.hd_year); 
			}
			
			/* print parasha */
			if (reading)
			{
				/* print the gregorian date */
				if (opt_e)
					printf ("%d.%d.%d%c", 
						h.gd_day, h.gd_mon, h.gd_year, sep); 
				else
					printf ("%d/%d/%d%c", 
						h.gd_mon, h.gd_day, h.gd_year, sep); 
				
				/* print parash */
				printf ("Parashat %s\n", hdate_get_parasha_string (reading, 0));
			}
			
			/* print rosh hodesh */
			if (rosh_hodesh)
			{
				/* print the gregorian date */
				if (opt_e)
					printf ("%d.%d.%d%c", 
						h.gd_day, h.gd_mon, h.gd_year, sep); 
				else
					printf ("%d/%d/%d%c", 
						h.gd_mon, h.gd_day, h.gd_year, sep); 
				
				/* print rosh hodesh */
				printf ("Rosh Chodesh %s\n", hdate_get_hebrew_month_string (h.hd_mon, 0));
			}
			
			/* print holyday */
			if (holyday)
			{
				/* print the gregorian date */
				if (opt_e)
					printf ("%d.%d.%d%c", 
						h.gd_day, h.gd_mon, h.gd_year, sep); 
				else
					printf ("%d/%d/%d%c", 
						h.gd_mon, h.gd_day, h.gd_year, sep); 
				
				/* print holyday */
				printf ("%s\n", hdate_get_holyday_string (holyday, 0));
			}
			
			/* print times */
			if (candle_lighting)
			{
				/* print the gregorian date */
				if (opt_e)
					printf ("%d.%d.%d%c", 
						h.gd_day, h.gd_mon, h.gd_year, sep); 
				else
					printf ("%d/%d/%d%c", 
						h.gd_mon, h.gd_day, h.gd_year, sep); 
				
				hdate_get_utc_sun_time (h.gd_day, h.gd_mon, h.gd_year, lat, lon, &sunrise, &sunset);
				sunset = sunset + tz * 60 - 20; /* -20 for shabat */
				
				/* print parash */
				printf ("Candle lighting:  %d:%d\n", sunset / 60, sunset % 60);
			}
			
			if (havdala)
			{
				/* print the gregorian date */
				if (opt_e)
					printf ("%d.%d.%d%c", 
						h.gd_day, h.gd_mon, h.gd_year, sep); 
				else
					printf ("%d/%d/%d%c", 
						h.gd_mon, h.gd_day, h.gd_year, sep); 
				
				hdate_get_utc_sun_time (h.gd_day, h.gd_mon, h.gd_year, lat, lon, &sunrise, &sunset);
				sunset = sunset + tz * 60 + 72; /* -72 for havdala */
				
				/* print parash */
				printf ("Havdalah (72 min): %d:%d\n", sunset / 60, sunset % 60);
			}
			
			/* move to next day */
			jd ++;
			hdate_set_jd (&h, jd);
		}
	
	return 0;
}
