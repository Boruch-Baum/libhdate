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


#include <stdio.h>  /* For printf */
#include <hdate.h>  /* For hebrew date */
#include <stdlib.h> /* For atoi */
#include <locale.h> /* For setlocale */

#define FALSE 0
#define TRUE -1

int
main (int argc, char* argv[])
{
	hdate_struct h;   /* The Hebrew date */

	int day;   /* The Gregorian date */
	int month;
	int year;
		
	/* Get date from user */
	if (argc == 4) 
		{   
			/* Set date from user */                         
			day = atoi (argv[1]);
			month = atoi (argv[2]);
			year = atoi (argv[3]);
			
			/* FIXME: different sanity check for hebrew and gregorian */
			if (year < 1 || day < 1 || day > 31 || month < 1 || month > 14) 
				{
					/* Print help for user and exit */
					printf ("USAGE: %s [day month year]\n", argv[0]);
					exit (0);
				}
		} 
	else if (argc == 1)
		{
			day = 0;
			month = 0;
			year = 0;
		}
	else		
		{	
			/* Print help for user and exit */
			printf ("USAGE: %s [day month year]\n", argv[0]);
			exit (0);
		}
	
	/* Set the locale, for libhdate to print locale messages */ 
	setlocale (LC_ALL,"");

	/* check if input date was hebrew, year > 3000 must be hebrew date */
	if (year > 3000)
		{
			/* Set hebrew date struct */
			hdate_set_hdate (&h, day, month, year);
		}
	else
		{
			/* Set hebrew date struct */
			hdate_set_gdate (&h, day, month, year);
		}
	
	/* Print long erez israeili date */
	printf ("%d,%d,%d, %s\n",  h.gd_day, h.gd_mon, h.gd_year,
		hdate_get_format_date (&h, FALSE, FALSE));	

	return 0;
}
