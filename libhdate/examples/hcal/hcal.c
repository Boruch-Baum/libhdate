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
	int m, y;
	
	h = *hdate_hdate (1, month, year);
	jd1 = hdate_gdate_to_jd (h.gd_day, h.gd_mon, h.gd_year) - h.hd_dw + 1;
	m = h.gd_mon + 1;
	if (m > 12)
	{
		m = 1;
		y = h.gd_year + 1;
	}
	else
	{
		y = h.gd_year;
	}	
	jd2 = hdate_gdate_to_jd (1, m, y);
	
	hdate_jd_to_hdate (jd1, &h_day1, &h_month1, &h_year1);
	hdate_jd_to_hdate (jd2, &h_day2, &h_month2, &h_year2);
	
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
	int jd1, jd, jd_today;
	int i,j;
	int holyday_type;
	int print_flag;
	
	/* Find today date */
	h = *hdate_hdate (0, 0, 0);
	jd_today = hdate_gdate_to_jd (h.gd_day, h.gd_mon, h.gd_year);
	
	/* Find day to start calendar with */
	h = *hdate_hdate (1, month, year);
	jd1 = hdate_gdate_to_jd (1, h.gd_mon, h.gd_year) - h.hd_dw;
		
	/* Loop over all days in this month */
	jd = jd1 + 1;
	for (i=0; i<6; i++)
		{
			for (j=0; j<7; j++)
				{
					/* Get this day hebrew date */
					hdate_jd_to_gdate (jd, &g_day, &g_month, &g_year);
					h = *hdate_hdate (g_day, g_month, g_year);
					
					/* Get this day holyday type */
					holyday_type = hdate_get_holyday(&h)==0?0:1;
					
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
	hdate_struct h;
	
	int day;	/* The Gregorian date */
	int month;
	int year;
	int flag = 0;
	
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
	else if (argc == 2 && (flag=atoi (argv[1])) != 0)
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

	/* Print calendar header */
	h = *hdate_hdate (1, month, year);
		
	print_header (h.gd_mon, h.gd_year);
	print_calendar (h.gd_mon, h.gd_year);
	return 0;		
}
