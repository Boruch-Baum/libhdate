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

/* Change log:
 *
 * Thu, 02 Sep 2004 - Patched by Moshe Wagner, flag support for all dates 
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
	
	h = *hdate_hdate (1, month, year);
	jd1 = hdate_gdate_to_jd (h.gd_day, h.gd_mon, h.gd_year) - h.hd_dw + 1;
	jd2 = jd1 + 41;
	
	hdate_jd_to_hdate (jd1, &h_day1, &h_month1, &h_year1);
	hdate_jd_to_hdate (jd2, &h_day2, &h_month2, &h_year2);
	
	/* Print Gregorian month and year */
	printf ("%s %d\n", hdate_get_month_string (h.gd_mon-1, FALSE), h.gd_year);
	
	/* Print Hebrew month and year */
	if (h_month1 != h_month2)
		{
			printf ("%s-", hdate_get_hebrew_month_string (h_month1, FALSE));
		}
	printf ("%s %s\n", hdate_get_hebrew_month_string (h_month2, FALSE), hdate_get_int_string(h_year1));
	
	for (j=0; j<7; j++)
		{
			printf ("%3s", hdate_get_day_string (j, TRUE));
						
			if (j != 6)
				printf ("\t");
		}
	
	printf ("\n");
				
	return 0;	
}

int 
print_calendar(int month, int year, int flag)
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
	jd=jd1;
	for (i=0; i<6; i++)
		{
			for (j=0; j<7; j++)
				{
					/* Get this day hebrew date */
					hdate_jd_to_gdate (jd, &g_day, &g_month, &g_year);
					h = *hdate_hdate (g_day, g_month, g_year);
					
					/* Get this day holyday type */
					holyday_type = 0; /* hdate_get_holyday_type(&h); */
					
					print_flag = g_month == month;
					
					/* flag = -1 print only saterdays, -2 only holydays, -3 only today */
					print_flag = print_flag && ((flag != -1) || (j == 6));
					print_flag = print_flag && ((flag != -2) || (holyday_type != 0));
					print_flag = print_flag && ((flag != -3) || (jd == jd_today));
					
					/* Print a day */
					if (print_flag)
						{
							if (holyday_type == 0)
								{
									printf ("%2d/%3s", g_day, hdate_get_int_string(h.hd_day+1));
								}
							else if (holyday_type == 1)
								{
									printf ("%2d-%3s", g_day, hdate_get_int_string(h.hd_day+1));
								} 
							else if (hdate_get_holyday_type(&h) == 2)
								{
									printf ("%2d+%3s", g_day, hdate_get_int_string(h.hd_day+1));
								}
							else if (holyday_type == 3)
								{
									printf ("%2d*%3s", g_day, hdate_get_int_string(h.hd_day+1));
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
	if (argc == 4)
		/* Start of patch by Moshe Wagner */
		{
			/* Set date from user */  
			month = atoi (argv[1]);
			year = atoi (argv[2]);
			flag = atoi (argv[3]);
		}
		/* End of patch by Moshe Wagner */
	else if (argc == 3) 
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
			printf ("USAGE: %s [month year] [flag: -1/-2/-3]\n", argv[0]);
			exit (0);
		}
	
	/* Set the locale, for libhdate to print locale messages */ 
	setlocale (LC_ALL,"");

	/* Print calendar header */
	h = *hdate_hdate (1, month, year);
		
	print_header (h.gd_mon, h.gd_year);
	print_calendar (h.gd_mon, h.gd_year, flag);
	return 0;		
}
