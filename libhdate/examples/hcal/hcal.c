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
	hdate_struct h1, h2;
	int j;
	
	/* set dates for begining and end of calendar */
	hdate_set_gdate (&h1, 1, month, year);
	hdate_set_gdate (&h2, 1, month % 12 + 1, year);
	
	/* Print Gregorian month and year */
	printf ("%s %d\n", hdate_get_month_string (h1.gd_mon, FALSE), h1.gd_year);
	
	/* Print Hebrew month and year */
	if (h1.hd_mon != h2.hd_mon)
		{
			printf ("%s-", hdate_get_hebrew_month_string (h1.hd_mon, FALSE));
		}
	printf ("%s %s\n", hdate_get_hebrew_month_string (h2.hd_mon, FALSE), hdate_get_int_string(h1.hd_year));
	
	for (j=1; j<8; j++)
		{
			printf ("%3s", hdate_get_day_string (j, TRUE));
			
			if (j != 7)
				printf ("\t");
		}
	
	printf ("\n");
				
	return 0;	
}

int 
print_calendar(int month, int year)
{
	hdate_struct h;
	int jd;
	int i,j;
	char type_char[] = {'/', '-', '+', '*'};
	int holyday_type;
	
	/* Find day to start calendar with */
	hdate_set_gdate (&h, 1, month, year);
	
	/* return print head to sunday */
	jd = h.hd_jd - h.hd_dw + 1;
		
	/* Loop over all days in this month */
	for (i=0; i<6; i++)
		{
			for (j=0; j<7; j++)
				{
					/* Get this day hebrew date */
					hdate_set_jd (&h, jd);
					
					/* Get this day holyday type ba harez (diaspora flag = 0) */
					holyday_type = hdate_get_holyday_type (hdate_get_holyday (&h, 0));
					
					if (h.gd_mon == month)
					{
						/* Print a day */
						printf ("%2d%c%3s", h.gd_day, type_char[holyday_type], hdate_get_int_string(h.hd_day));
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
print_month (int month, int year)
{
	hdate_struct h;
	
	/* check if hebrew year (year > 3000) */
	if (year > 3000)
		hdate_set_hdate (&h, 1, month, year);
	else
		hdate_set_gdate (&h, 1, month, year);
	
	/* Print calendar header */
	print_header (h.gd_mon, h.gd_year);
	print_calendar (h.gd_mon, h.gd_year);
	
	printf ("\n");
	
	return 0;	
}

int
main (int argc, char* argv[])
{
	int month;
	int year;
	hdate_struct h;
	
	/* Get date from user */
	if (argc == 3) 
		{
			month = atoi (argv[1]);
			year = atoi (argv[2]);
		}
	else if (argc == 2)
		{
			month = 0;
			year = atoi (argv[1]);
		}
	else if (argc == 1)
		{
			hdate_set_gdate (&h, 0, 0, 0); /* get today's year */
			month = 0;
			year = h.gd_year;
		}
	else
		{
			/* Print help for user and exit */
			printf ("USAGE: %s [[month] year]\n", argv[0]);
			exit (0);
		}
	
	/* Set the locale, for libhdate to print locale messages */ 
	setlocale (LC_ALL,"");
		
	/* print all year */
	if (month == 0)
	{
		for (month = 1; month < 13; month++)
		{
			print_month (month, year);
		}
	}
	else /* print only this month */
	{
		print_month (month, year);
	}
	return 0;
}
