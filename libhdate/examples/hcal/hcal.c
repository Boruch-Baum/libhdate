
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

#include <stdio.h>		/* For printf */
#include <hdate.h>		/* For hebrew date */
#include <stdlib.h>		/* For atoi */
#include <locale.h>		/* For setlocale */
#include <unistd.h>		/* For getopt */

#define FALSE 0
#define TRUE -1

/* print help */
int
print_help (char *program)
{
	printf ("hcal - example program for libhdate\n\n");

	printf ("USAGE: %s [-h] ", program);
	printf ("   [[month] year]\n");

	printf ("OPTIONS:\n");
	printf ("   -h : Print html format.\n");

	return 0;
}

/* print html css part */
int
print_css ()
{
	printf ("\n\
body {\n\
  direction: rtl;\n\
}\n\
\n\
table {\n\
	width: 80%;\n\
	table-layout: fixed;\n\
	font-size: 14pt;\n\
	border: solid #aaaaaa;\n\
}\n\
\n\
th {\n\
	background-color: #aaaaaa;\n\
	text-align: center;\n\
}\n\
\n\
td {\n\
	text-align: center;\n\
}\n\
\n\
div.today {\n\
\n\
}\n\
\n\
div.gmonth {\n\
	font-size: 16pt;\n\
}\n\
\n\
div.gyear {\n\
	font-size: 24pt;\n\
	font-weight: bold;\n\
}\n\
\n\
div.hmonth {\n\
	font-size: 16pt;\n\
}\n\
\n\
div.hyear {\n\
	font-size: 24pt;\n\
	font-weight: bold;\n\
}\n\
\n\
td.sat {\n\
	border: solid #777777;\n\
}\n\
\n\
td.regular {\n\
	border: solid #aaaaaa;\n\
}\
\
td.holiday {\
	color: #990000;\n\
	border: solid #888888;\
}\n\
\n\
td.out_of_month {\n\
	color: #dddddd;\n\
	border: solid #dddddd;\n\
}\n\n");

	return 0;
}

/* print html header */
int
print_html_header ()
{
	printf ("\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n\
<html>\n\
<head>\n\
<meta name=\"generator\" content=\"hcal (libhdate)\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n\
<style type=\"text/css\" media=\"all\">");

	print_css ();

	printf ("\
</style>\n\
<title>Hebrew Calendat</title>\n\
</head>\n\
<body>\n");

	return 0;
}

int
print_html_footer ()
{
	printf ("\n</body>\n</html>\n");

	return 0;
}

/* print year and month part */
int
print_header (int month, int year, int opt_h, int opt_s)
{
	hdate_struct h1, h2;
	int j;

	/* set dates for begining and end of calendar */
	hdate_set_gdate (&h1, 1, month, year);
	hdate_set_gdate (&h2, 1, month % 12 + 1, year);

	/* Print Gregorian month and year */
	if (opt_h)
	{
		printf ("<div class=\"gmonth\">\n");
		printf ("%s\n", hdate_get_month_string (h1.gd_mon, FALSE));
		printf ("</div>\n");

		printf ("<div class=\"gyear\">\n");
		printf ("%d\n", h1.gd_year);
		printf ("</div>\n");
	}
	else
	{
		printf ("%s %d\n", hdate_get_month_string (h1.gd_mon, FALSE),
			h1.gd_year);
	}

	/* Print Hebrew month and year */
	if (opt_h)
	{
		printf ("<div class=\"hmonth\">\n");
	}

	if (h1.hd_mon != h2.hd_mon)
	{
		printf ("%s-",
			hdate_get_hebrew_month_string (h1.hd_mon, FALSE));
	}
	printf ("%s ", hdate_get_hebrew_month_string (h2.hd_mon, FALSE));

	if (opt_h)
	{
		printf ("</div>\n");

		printf ("<div class=\"hyear\">\n");
	}

	printf ("%s\n", hdate_get_int_string (h1.hd_year));

	if (opt_h)
	{
		printf ("</div>\n");

		printf ("<div class=\"month_table\">\n");
		printf ("<table>\n");
		printf ("<tr>\n");
	}

	for (j = 1; j < 8; j++)
	{
		if (opt_h)
		{
			printf ("<th>");
			printf ("%3s", hdate_get_day_string (j, FALSE));
			printf ("</th>\n");
		}
		else
		{
			printf ("%3s", hdate_get_day_string (j, TRUE));

			if (j != 7)
				printf ("\t");
		}
	}

	if (opt_h)
	{
		printf ("</tr>\n");
	}
	else
	{
		printf ("\n");
	}

	return 0;
}

/* print month table */
int
print_calendar (int month, int year, int opt_h, int opt_s)
{
	hdate_struct h;
	int jd;
	int i, j;
	char type_char[] = { '/', '+', '*', '-' };
	int holyday_type;

	/* Find day to start calendar with */
	hdate_set_gdate (&h, 1, month, year);

	/* return print head to sunday */
	jd = h.hd_jd - h.hd_dw + 1;

	/* Loop over all days in this month */
	for (i = 0; i < 6; i++)
	{
		if (opt_h)
		{
			printf ("<tr>\n");
		}

		for (j = 0; j < 7; j++)
		{
			/* Get this day hebrew date */
			hdate_set_jd (&h, jd);

			/* Get this day holyday type ba harez (diaspora flag =
			 * 0) */
			holyday_type =
				hdate_get_holyday_type (hdate_get_holyday
							(&h, FALSE));

			if (opt_h)
			{
				if (h.gd_mon != month)
				{
					printf ("<td class=\"out_of_month\">");
				}
				else if (j == 6)
				{
					printf ("<td class=\"sat\">");
				}
				else if (holyday_type)
				{
					printf ("<td class=\"holiday\">");
				}
				else
				{
					printf ("<td class=\"regular\">");
				}

				/* Print a day */
				printf ("%2d<br\>\n%3s", h.gd_day,
					hdate_get_int_string (h.hd_day));

				if (holyday_type)
				{
					printf ("<br\>\n%s",
						hdate_get_holyday_string
						(hdate_get_holyday (&h, FALSE),
						 FALSE));
				}

				printf ("</td>\n");
			}
			else
			{
				if (h.gd_mon == month)
				{
					/* Print a day */
					printf ("%2d%c%3s", h.gd_day,
						type_char[holyday_type],
						hdate_get_int_string (h.
								      hd_day));
				}
				if (j != 6)
					printf ("\t");
			}
			jd++;
		}

		if (opt_h)
		{
			printf ("\n</tr>\n");
		}
		else
		{
			printf ("\n");
		}
	}

	if (opt_h)
	{
		printf ("</table>\n</div>\n");
	}

	return 0;
}

/* print month header and month table */
int
print_month (int month, int year, int opt_h, int opt_s)
{
	hdate_struct h;

	/* check if hebrew year (year > 3000) */
	if (year > 3000)
		hdate_set_hdate (&h, 1, month, year);
	else
		hdate_set_gdate (&h, 1, month, year);

	/* Print calendar header */
	print_header (h.gd_mon, h.gd_year, opt_h, opt_s);
	print_calendar (h.gd_mon, h.gd_year, opt_h, opt_s);

	printf ("\n");

	return 0;
}

int
main (int argc, char *argv[])
{
	/* date */
	int month;
	int year;

	/* user opts */
	char c;
	int opt_h = 0;		/* -h html format flag */
	int opt_s = 0;		/* -s Short format flag */

	/* hdate struct */
	hdate_struct h;

	/* init locale */
	setlocale (LC_ALL, "");

	/* command line parsing */
	while ((c = getopt (argc, argv, "sh")) != EOF)
	{
		switch (c)
		{
		case 's':
			opt_s = 1;
			break;
		case 'h':
			opt_h = 1;
			break;
		default:
			print_help (argv[0]);
			exit (0);
			break;
		}
	}

	/* Get date from user */
	if (argc == (optind + 2))
	{
		month = atoi (argv[optind]);
		year = atoi (argv[optind + 1]);
	}
	else if (argc == (optind + 1))
	{
		month = 0;
		year = atoi (argv[optind]);

		/* Check for valid years */
		if (year < 1)
		{
			/* Print help for user and exit */
			print_help (argv[0]);
			exit (0);
		}
	}
	else if (argc == (optind))
	{
		hdate_set_gdate (&h, 0, 0, 0);	/* get today's year */
		month = h.gd_mon;
		year = h.gd_year;
	}
	else
	{
		/* Print help for user and exit */
		print_help (argv[0]);
		exit (0);
	}

	/* if html print html header */
	if (opt_h)
		print_html_header ();

	/* print all year */
	if (month == 0)
	{
		for (month = 1; month < 13; month++)
		{
			print_month (month, year, opt_h, opt_s);
		}
	}
	else			/* print only this month */
	{
		print_month (month, year, opt_h, opt_s);
	}

	/* if html print html header */
	if (opt_h)
		print_html_footer ();

	return 0;
}
