/* hcal.c
 * Example file for libhdate. 
 * 
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hcal.c -o hcal
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

#define FALSE 0
#define TRUE -1

int 
print_header(int month, int year)
{
	hdate_struct h;
	int h_day1, h_month1, h_year1; /* The Hebrew date */
	int h_day2, h_month2, h_year2;
	int jd1, jd2;
	int j;
	
	hdate_gdate (&h, 1, month, year);
	jd1 = h.hd_jd;
	jd2 = jd1 + 32;
	
	hdate_jd_to_hdate (jd1, &h_day1, &h_month1, &h_year1, NULL, NULL);
	hdate_jd_to_hdate (jd2, &h_day2, &h_month2, &h_year2, NULL, NULL);
	
	/* Print Gregorian month and year */
	printf ("%s %d\n", hdate_get_month_string (h.gd_mon, FALSE), h.gd_year);
	
	/* Print Hebrew month and year */
	if (h_month1 != h_month2)
		{
			printf ("%s-", hdate_get_hebrew_month_string (h_month1, FALSE));
		}
	printf ("%s %s\n", hdate_get_hebrew_month_string (h_month2, FALSE), hdate_get_int_string(h_year1));
	
	for (j=0; j<7; j++)
		{
			printf ("%3s", hdate_get_day_string (j + 1, TRUE));
						
			if (j != 6)
				printf ("\t");
		}
	
	printf ("\n");
				
	return 0;	
}

int 
print_calendar(int month, int year)
{
	hdate_struct h;
	int g_day, g_month, g_year; /* The Gregorian date */
	int jd;
	int i,j;
	int holyday_type;
	
	/* Find day to start calendar with */
	hdate_gdate (&h, 1, month, year);
	
	/* return print head to sunday */
	jd = h.hd_jd - h.hd_dw + 1;
		
	/* Loop over all days in this month */
	for (i=0; i<6; i++)
		{
			for (j=0; j<7; j++)
				{
					/* Get this day hebrew date */
					hdate_jd_to_gdate (jd, &g_day, &g_month, &g_year);
					hdate_gdate (&h, g_day, g_month, g_year);
					
					/* Get this day holyday type ba harez (diaspora flag = 0) */
					holyday_type = hdate_get_holyday_type (hdate_get_holyday (&h, 0));
					
					if (g_month == month)
					{
						/* Print a day */
						if (holyday_type == 0)
							{
								printf ("%2d/%3s", g_day, hdate_get_int_string(h.hd_day));
							}
						else if (holyday_type == 1)
							{
								printf ("%2d-%3s", g_day, hdate_get_int_string(h.hd_day));
							} 
						else if (holyday_type == 2)
							{
								printf ("%2d+%3s", g_day, hdate_get_int_string(h.hd_day));
							}
						else if (holyday_type == 3)
							{
								printf ("%2d*%3s", g_day, hdate_get_int_string(h.hd_day));
							}
					}
					if (j != 6)
						printf ("\t");
					
					jd++;
				}
			printf ("\n");
		}
		
	return 0;
}

int
main (int argc, char* argv[])
{
	int day;	/* The Gregorian date */
	int month;
	int year;
	hdate_struct h;
	
	/* Get date from user */
	if (argc == 3) 
		{
			month = atoi (argv[1]);
			year = atoi (argv[2]);
		} 
	else if (argc == 1)
		{
			month = 0;
			year = 0;
		}
	else
		{	
			/* Print help for user and exit */
			printf ("USAGE: %s [month year]\n", argv[0]);
			exit (0);
		}
	
	/* Set the locale, for libhdate to print locale messages */ 
	setlocale (LC_ALL,"");
	hdate_gdate (&h, 1, month, year);
	
	/* Print calendar header */
	print_header (h.gd_mon, h.gd_year);
	print_calendar (h.gd_mon, h.gd_year);
	return 0;
}
