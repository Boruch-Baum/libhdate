/* hdate_holydays_html.c
 * Example file for libhdate. 
 * 
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hdate_holydays_html.c -o hdate_holydays_html
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

/* some useful time zones
 * Eilat 29, -34, 2
 * Haifa 32, -34, 2
 * Jerusalem 31, -35, 2
 * Tel Aviv 32, -34, 2
 */
/* define Tel Aviv time zone */
#define tz 2
#define lat 32.0
#define lon -34.0

int
main (int argc, char* argv[])
{
	hdate_struct h; 	/* The Hebrew date */

	int jd;
	int reading;
	int year;
	int sunset, sunrise;
	
	/* Get calendar hebrew year */
	if (argc == 2) 
		{                
			year = atoi (argv[1]);
						
			hdate_set_hdate (&h, 1, 1, year);
			/* get the julian of first shabat in year */
			jd = h.hd_jd + (7 - h.hd_dw);
		} 
	else 
		{	
			/* Print help for user and exit */
			printf ("USAGE: %s year\n", argv[0]);
			exit (0);
		}
	
	/* Set the locale to Hebrew*/ 
	setlocale (LC_ALL,"he_IL");

	/* Print start of html format */
	printf ("<html>\n");
	printf ("<head>\n");
	printf ("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">");
	printf ("<title>לוח שנה</title>\n");
	printf ("</head>\n");
	printf ("<body dir=\"rtl\">\n");
		
	/* Print the html calendar */
	printf ("<h1>לוח שנה, קריאות בתורה לשנת %s</h1>\n", hdate_get_int_string (h.hd_year));
	printf ("<h3>אורכה %d ימים, ראש השנה ביום %s</h3>\n", h.hd_size_of_year, hdate_get_day_string (h.hd_new_year_dw, 0));
	
	while (h.hd_year == year)
		{	
			reading = hdate_get_parasha (&h, 0);
			
			if (reading != 0)
				{
					printf ("%d,%d,%d %s, %s</br>\n", h.gd_day, h.gd_mon, h.gd_year, 
						hdate_get_format_date (&h, 0, 0),
						hdate_get_parasha_string (reading, 0));
				}
			else
				{
					printf ("%d,%d,%d %s</br>\n", h.gd_day, h.gd_mon, h.gd_year, 
						hdate_get_format_date (&h, 0, 0));
				}

			/* print sunrise and sunset times of fri and sat. */
			hdate_set_jd (&h, jd - 1);
			hdate_get_utc_sun_time (h.gd_day, h.gd_mon, h.gd_year, lat, lon, &sunrise, &sunset);
			sunrise = sunrise + tz * 60;
			sunset = sunset + tz * 60;
			
			printf ("יום שישי</br>\n");
			printf ("זריחה %d:%d, שקיעה %d:%d</br>\n", sunrise / 60, sunrise % 60, sunset / 60, sunset % 60);
			hdate_set_jd (&h, jd);
				
			hdate_get_utc_sun_time (h.gd_day, h.gd_mon, h.gd_year, lat, lon, &sunrise, &sunset);
			sunrise = sunrise + tz * 60;
			sunset = sunset + tz * 60;
			
			printf ("יום שבת</br>\n");
			printf ("זריחה %d:%d, שקיעה %d:%d</br>\n", sunrise / 60, sunrise % 60, sunset / 60, sunset % 60);
			
			/* print a seperator */
			printf ("------</br>\n");
			 
			/* get next week shabats hdate */
			hdate_set_jd (&h, jd);
			jd = jd + 7;
		}
	
	/* Print end of html format */
	printf ("</body>\n");
	printf ("</html>\n");
	
	return 0;
}
