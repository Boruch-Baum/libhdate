/* hdate_reading_ical.c
 * Example file for libhdate. 
 * 
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hdate_reading_ical.c -o hdate_reading_ical
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


#include <stdio.h>  /* For printf */
#include <hdate.h>  /* For hebrew date */
#include <stdlib.h> /* For atoi */
#include <locale.h> /* For setlocale */

int
main (int argc, char* argv[])
{
	hdate_struct h; 	/* The Hebrew date */

	int day;	/* The Gregorian date for the event in the iCal */
	int month;
	int year;
	int jd;
	int parasha;
	char* event_summary;

	int  start_jd;	/* The Julian day number to start iCal calendar from */
	int end_jd;	/* The Julian day number to end iCal calendar with */

	int start_d ;
	int start_m;
	int start_y;
	int end_d;
	int end_m;
	int end_y;
	
	/* Get start and end dates of the iCal file from user */
	if (argc == 7) 
		{   
			/* Set start and end dates of the iCal file from user */                         
			start_d = atoi (argv[1]);
			start_m = atoi (argv[2]);
			start_y = atoi (argv[3]);
			end_d = atoi (argv[4]);
			end_m = atoi (argv[5]);
			end_y = atoi (argv[6]);

			start_jd = hdate_gdate_to_jd (start_d, start_m, start_y);
			end_jd = hdate_gdate_to_jd (end_d, end_m, end_y);
		} 
	else 
		{	
			/* Print help for user and exit */
			printf ("USAGE: %s start_day start_month start_year end_day end_month end_year\n", argv[0]);
			exit (0);
		}
	
	/* Set the locale, for libhdate to print locale messages */ 
	setlocale (LC_ALL,"");

	/* Print start of iCal format */
	printf ("BEGIN:VCALENDAR\n");
	printf ("CALSCALE:GREGORIAN\n");
	printf ("METHOD:PUBLISH\n");

	/* Print iCal event */
	for (jd = start_jd; jd < end_jd; jd++)
		{
			hdate_jd_to_gdate (jd, &day, &month, &year);
			hdate_gdate (&h, day, month, year);
			/* set diaspora flag to 0, for reading ba harez */
			parasha =  hdate_get_parasha (&h, 0);
			
			if (parasha != 0)
				{
					event_summary = hdate_get_parasha_string (parasha, 0);

					printf ("BEGIN:VEVENT\n");
					printf ("UID:0\n");
					printf ("DTSTART;VALUE=DATE:%04d%02d%02d\n", year, month, day);
					printf ("SUMMARY:%s\n", event_summary);
					printf ("DTEND;VALUE=DATE:%04d%02d%02d\n", year, month, day);
					printf ("CATEGORIES:Holidays\n");
					printf ("END:VEVENT\n");
				}
		}
	
	/* Print end of iCal format */
	printf ("END:VCALENDAR\n");
	
	return 0;
}
