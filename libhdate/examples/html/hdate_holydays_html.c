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

int
main (int argc, char* argv[])
{
	hdate_struct h; 	/* The Hebrew date */

	int jd;
	int holyday;
	int year;
	
	/* Get calendar hebrew year */
	if (argc == 2) 
		{   
			/* Set start and end dates of the iCal file from user */                         
			year = atoi (argv[1]);
						
			hdate_set_hdate (&h, 1, 1, year);
			jd = h.hd_jd;
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
	printf ("<title>לוח שנה</title>\n");
	printf ("</head>\n");
	printf ("<body dir=\"rtl\">\n");
		
	/* Print the html calendar */
	printf ("<h1>לוח שנה, חגים לשנת %s</h1>\n", hdate_get_int_string (h.hd_year));
	printf ("<h3>אורכה %d ימים, ראש השנה ביום %s</h3>\n", h.hd_size_of_year, hdate_get_day_string (h.hd_new_year_dw, 0));
	
	while (h.hd_year == year)
		{
			/* get today hdate */
			hdate_set_jd (&h, jd);
			
			/* if start of month print header with long month name*/
			if (h.hd_day == 1)
				printf ("<h2>%s</h2>\n", hdate_get_hebrew_month_string (h.hd_mon, 0));
			
			/* set diaspora flag to 0, for holydays ba harez */
			holyday = hdate_get_holyday (&h, 0);
			
			if (holyday != 0)
				{
					printf ("%d,%d,%d %s</br>\n", h.gd_day, h.gd_mon, h.gd_year, hdate_get_format_date (&h, 0, 0));
				}
			
			jd++;
		}
	
	/* Print end of html format */
	printf ("</body>\n");
	printf ("</html>\n");
	
	return 0;
}
