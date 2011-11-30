
/* hcal.c
 * Example file for libhdate. 
 * 
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` hcal.c -o hcal
 * 
 * Copyright:  2011 (c) Boruch Baum, 2004-2010 (c) Yaacov Zamir 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>		/* For printf */
#include <hdate.h>		/* For hebrew date */
#include <stdlib.h>		/* For atoi */
#include <locale.h>		/* For setlocale */
#include <getopt.h>		/* For getopt_long */
#include <string.h>		/* For strlen */
#include <error.h>		/* For error */

#define FALSE 0
#define TRUE -1
#define Shabbat 7

// copied from support.h in src dir
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif


// for function hdate_validate
#define CHECK_DAY_PARM   1
#define CHECK_MONTH_PARM 2
#define CHECK_YEAR_PARM  3


// for colorization
#define CODE_REVERSE_VIDEO "%c[7m", 27
#define CODE_RESTORE_VIDEO "%c[m", 27
#define CODE_BLACK         "%c[30m", 27
#define CODE_LIGHT_RED     "%c[31m", 27
#define CODE_LIGHT_GREEN   "%c[32m", 27
#define CODE_LIGHT_BROWN   "%c[33m", 27
#define CODE_DARK_BLUE     "%c[34m", 27
#define CODE_LIGHT_PURPLE  "%c[35m", 27
#define CODE_LIGHT_AQUA    "%c[36m", 27
#define CODE_LIGHT_GREY    "%c[37m", 27
#define CODE_BOLD_GREY     "%c[1;30m", 27
#define CODE_BOLD_RED      "%c[1;31m", 27
#define CODE_BOLD_GREEN    "%c[1;32m", 27
#define CODE_BOLD_YELLOW   "%c[1;33m", 27
#define CODE_BOLD_BLUE     "%c[1;34m", 27
#define CODE_BOLD_PURPLE   "%c[1;35m", 27
#define CODE_BOLD_AQUA     "%c[1;36m", 27
#define CODE_BOLD_WHITE    "%c[1;37m", 27
#define ELEMENT_WEEKDAY_G     1
#define ELEMENT_WEEKDAY_H     2
#define ELEMENT_SHABBAT_DAY   3
#define ELEMENT_HOLIDAY_DAY   4
#define ELEMENT_HOLIDAY_FLAG  5
#define ELEMENT_SHABBAT_NAME  6
#define ELEMENT_WEEKDAY_NAMES 7
#define ELEMENT_MONTH_G       8
#define ELEMENT_MONTH_H       9




typedef struct {
			int html;
			int external_css;
			int diaspora;
			int parasha;
			int shabbat;
			int no_reverse;
			int three_month;
			int colorize;
			int jd_today_g;
			int jd_today_h;
				} option_list;

typedef struct {
			int g_month_1;
			int g_year_1;
			int g_month_2;
			int g_year_2;
			int h_month_1;
			int h_year_1;
			int h_month_2;
			int h_year_2;
				} header_info;



/**************************************************
*  print version
*************************************************/
int print_version ()
{
	printf ("hcal - Hebrew calendar\nversion 2\n");
	return 0;
}

/**************************************************
*  print help
*************************************************/
int print_help ()
{
	printf ("hcal - Hebrew calendar\n\n");

	printf ("USAGE: hcal [-dhi] [[month] year]\n");
	printf ("OPTIONS:\n");
	printf ("   -d : Use diaspora reading and holidays.\n");
	printf ("   -h : Print html format.\n");
	printf ("   -i : Use external css file \"./hcal.css\".\n");
/* To be documented:
	--no-reverse	do not highlight the current date in reverse video
*/
	return 0;
}

/************************************************************
* check validity of date parameters
************************************************************/
int hdate_validate (int parameter_to_check, int day, int month, int year)
{
	hdate_struct h;
	
	if (year < 0) return FALSE;
	
	switch (parameter_to_check)
	{

	case CHECK_DAY_PARM:

		/************************************************************
		* check day in Hebrew date
		************************************************************/
		if (year > 3000)
		{
			hdate_set_hdate (&h, 1, 1, year);
			if ((day < 1) || (day > 30) ||
				((day > 29) && ((month==4) || (month==6) || (month==8) || (month=10) || (month==12) || (month==14))) ||
				(((h.hd_size_of_year==353) || (h.hd_size_of_year==383)) && ((month==2) || (month==3)) && (day >29)) ||
				(((h.hd_size_of_year==354) || (h.hd_size_of_year==384)) && (month==2) && (day>29)) )			
				return FALSE;
			return TRUE;
		}

		/************************************************************
		* check day in Gregorian date
		************************************************************/
		else
		{
			if ((day < 1) || 
				((day > 31) && ((month==1) || (month==3) || (month==5) || (month==7) || (month==8) || (month==10) ||(month==12))) ||
				((day > 30) && ((month==4) || (month==6) || (month==9) || (month==11))) ||
				((day > 29) && (month==2) && ((year%4)==0)) ||
				((day > 28) && (month==2) && ((year%4)!=0))
			   )
				return FALSE;
			return TRUE;
		}
		break;	


	case CHECK_MONTH_PARM:

		/************************************************************
		* check month in Hebrew date
		************************************************************/
		if (year > 3000)
		{
			if ((month <= 0) || (month > 14)) return FALSE;
			hdate_set_hdate (&h, 1, 1, year);
			if ((h.hd_size_of_year <365) && (month >12)) return FALSE;
			return TRUE;
		}

		/************************************************************
		* check month in Gregorian date
		************************************************************/
		else
		{
			if ((month <= 0) || (month > 12)) return FALSE;
			return TRUE;
		}
		break;
	
	case CHECK_YEAR_PARM:
		return TRUE;
		break;
	}
	return FALSE;
}







/**************************************************
*  HTML - print css section
*************************************************/
int print_css (int external_css)
{
	if (external_css) printf ("\n\t@import \"hcal.css\";\n");
	else
	{
		printf ("\nbody {\n direction: rtl;}\n\
\nimg { \nmargin:0; padding: 0;	vertical-align: middle;	border: 0;}\n\
p {}\n\
table {	width: 80%; table-layout: fixed; font-size: 14pt; border: solid #aaaaaa; }\n\
th { background-color: #aaaaaa;	text-align: center; }\n\
td { text-align: center; }\n\
div.today { }\n\
div.gmonth { font-size: 16pt; }\n\
div.gyear {	font-size: 24pt; font-weight: bold; }\n\
div.gday { }\n\
div.hmonth { font-size: 16pt; }\n\
div.hyear {	font-size: 24pt; font-weight: bold; }\n\
div.hday { }\n\
div.holiday_name { }\n\
td.sat { border: solid #777777; }\n\
td.regular { border: solid #aaaaaa; }\n\
td.holiday { color: #990000; border: solid #888888; }\n\
td.out_of_month { color: #dddddd; border: solid #dddddd; }\n");
	}

	return 0;
}

/************************************************************
* begin - error message functions
************************************************************/
void print_parm_error ( char *parm_name )
{
	error(0,0,"%s: %s %s %s",N_("error"), N_("parameter"), parm_name, N_("is non-numeric or out of bounds"));
}
/************************************************************
* end - error message functions
************************************************************/


/**************************************************
*  HTML - print header
*************************************************/
int print_html_header (int external_css)
{
	printf ("\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\n\
   \"http://www.w3.org/TR/html4/strict.dtd\">\n\
<html>\n\
<head>\n\
<meta name=\"generator\" content=\"hcal (libhdate)\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n\
<style title=\"Normal\" type=\"text/css\" media=\"all\">");

	print_css (external_css);

	printf ("</style>\n");

	/* some alternative css styles */
	printf ("<link rel=\"alternate stylesheet\" title=\"High contrast\" \
type=\"text/css\" media=\"screen\" href=\"high_contrast.css\">");
	printf ("</style>\n\
<link rel=\"alternate stylesheet\" title=\"Colorful\" \
type=\"text/css\" media=\"screen\" href=\"colorful.css\">");
	printf ("</style>\n\
<link rel=\"alternate stylesheet\" title=\"Print\" \
type=\"text/css\" media=\"all\" href=\"print.css\">\n");

	printf ("<title>Hebrew Calendar</title>\n</head>\n<body>\n");

	return 0;
}



/**************************************************
 *  HTML - print footer
 *************************************************/
int print_html_footer ()
{
	printf ("<!-- <p>\n\
<a href=\"http://validator.w3.org/check/referer\">\n<img \
src=\"http://www.w3.org/Icons/valid-html401\"\n\
alt=\"Valid HTML 4.01!\" height=\"31\" width=\"88\" />\n</a>\n");
	printf ("<a href=\"http://jigsaw.w3.org/css-validator\">\n\
<img src=\"http://www.w3.org/Icons/valid-css\" alt=\"Valid CSS!\">\n\
</a>\n</p> -->\n");

	printf ("</body>\n</html>\n");

	return 0;
}


/**************************************************
*  Print HTML header - month line
*************************************************/
void print_header_month_line_html( header_info header)
{
	
	// TODO - fix bug ellul xxxx - tishrei xxxx+1
	
	
	/**************************************************
	*  Print Gregorian month and year
	*************************************************/
	printf ("<div class=\"gmonth\">\n");
	printf ("%s\n", hdate_get_month_string (header.g_month_1, FALSE));
	printf ("</div>\n");
	printf ("<div class=\"gyear\">\n");
	printf ("%d\n", header.g_year_1);
	printf ("</div>\n");
	/**************************************************
	*  Print Hebrew month and year
	*************************************************/
	printf ("<div class=\"hmonth\">\n");
	if (header.h_month_1 != header.h_month_2)
	{
		printf ("%s-",
			hdate_get_hebrew_month_string (header.h_month_1, FALSE));
	}
	printf ("%s ", hdate_get_hebrew_month_string (header.h_month_2, FALSE));
	printf ("\n</div>\n");
	printf ("<div class=\"hyear\">\n");
	printf ("%s\n", hdate_get_int_string (header.h_year_1));
	printf ("</div>\n");

	printf ("<div class=\"month_table\">\n");
	printf ("<table>\n<tr>");
}


/**************************************************
*  HTML - print column headings for days of weeks
**************************************************/
void print_header_dow_line_html()
{
	int j;
	
	for (j = 1; j < 8; j++)
		printf ("<th>");
		printf ("%3s", hdate_get_day_string (j, FALSE));
		printf ("</th>\n");
}



/**************************************************
 *  end HTML functions
 *************************************************/


/*************************************************
*  Display data in a more pleasing visual manner
*************************************************/
void colorize_element ( int element )
{
	switch (element) {
	case ELEMENT_WEEKDAY_G: printf(CODE_LIGHT_GREY); break;
	case ELEMENT_WEEKDAY_H: printf(CODE_LIGHT_BROWN); break;
	case ELEMENT_MONTH_G: printf(CODE_LIGHT_GREEN); break;
	case ELEMENT_MONTH_H: printf(CODE_LIGHT_BROWN); break;
	case ELEMENT_WEEKDAY_NAMES: printf(CODE_LIGHT_GREY); break;
	case ELEMENT_SHABBAT_NAME: printf(CODE_LIGHT_AQUA); break;
	case ELEMENT_SHABBAT_DAY: printf(CODE_LIGHT_AQUA); break;
	case ELEMENT_HOLIDAY_DAY: printf(CODE_LIGHT_AQUA); break;
	}
}



/**************************************************
 *  print header - month line to stdout
 *************************************************/
void print_header_month_line_stdout( header_info header, int colorize )
{

	char *g_month, *h1_month, *h2_month;
	int j;
	int padding, g_month_len, h1_month_len, h2_month_len, h2_year_len;
	static int h1_year_len = 5;

	/**************************************************
	*  Print Gregorian month and year
	*************************************************/
	g_month = hdate_get_month_string (header.g_month_1, FALSE);
	if (colorize) colorize_element(ELEMENT_MONTH_G);
	printf ("%s-%d", g_month, header.g_year_1);
	if (colorize) printf(CODE_RESTORE_VIDEO);


	/**************************************************
	*  calculate padding
	*************************************************/
	g_month_len = strlen(g_month);
	h1_month = hdate_get_hebrew_month_string (header.h_month_1, FALSE);
	h1_month_len = strlen(h1_month);
	if (header.h_month_1 != header.h_month_2)
	{
		h2_month = hdate_get_hebrew_month_string (header.h_month_2, FALSE);
		h2_month_len = strlen(h2_month);
	}
	else { h2_month_len = 0; }
	
	if (header.h_year_1 != header.h_year_2)	h2_year_len = 5;
	else { h2_year_len = 0; }


	if (!hdate_is_hebrew_locale())
		padding = 42-g_month_len-6-h2_month_len-h1_month_len-h1_year_len-h2_year_len;
	else
	{
		/* This is necessary because strlen interprets each Hebrew unicode character
		 * as two characters
		 */
		if (h2_year_len != 0) h2_year_len = (h2_year_len * 2) +4;
		padding = 42-11+(0-g_month_len-h2_month_len-h1_month_len-h2_year_len-h1_year_len)/2;
	}
	
	
	/**************************************************
	*  print padding
	*************************************************/
	for (j = 1; j < padding; j++) printf(" ");



	/**************************************************
	*  Print Hebrew month and year
	*************************************************/
	if (colorize) colorize_element(ELEMENT_MONTH_H);
	printf ("%s-", h1_month);
	if (header.h_year_1 != header.h_year_2)
	{
		printf ("%s ", hdate_get_int_string (header.h_year_1));
		printf ("%s-%s", 
			h2_month,  hdate_get_int_string (header.h_year_2));
	}
	else if (header.h_month_1 != header.h_month_2)
	{
		printf ("%s-%s", h2_month,
			hdate_get_int_string (header.h_year_1));
	}
	else 
	{
		printf ("%s", hdate_get_int_string (header.h_year_1));
	}
	if (colorize) printf(CODE_RESTORE_VIDEO);
}


/**************************************************
*  print column headings for days of weeks
**************************************************/
void print_header_dow_line_stdout( int colorize )
{
	int column;

	void print_dow_column( int column )
	{
		if (hdate_is_hebrew_locale())
		{		/* Hebrew heading is a single charcter per day */
			printf ("%s%s%s", "  ", hdate_get_day_string (column, TRUE)," ");
		}
		else
		{		/* presume three character heading */
			printf ("%s%3s", " ", hdate_get_day_string (column, TRUE));
		}
	
		if (column != 7) printf ("  ");
	}
	
	if (colorize) colorize_element(ELEMENT_WEEKDAY_NAMES);
	for (column = 1; column < 7; column++) print_dow_column(column);
	if (colorize) colorize_element(ELEMENT_SHABBAT_NAME);
	print_dow_column(7);
	if (colorize) printf(CODE_RESTORE_VIDEO);
}



/**************************************************
*  calculate start/end month/year for a calendar
**************************************************/
header_info calculate_calendar_start_end (int month, int year)
{
	hdate_struct h;
	header_info header;

	hdate_set_gdate (&h, 1, month, year);
	header.g_month_1 = h.gd_mon;
	header.g_year_1 = h.gd_year;
	header.h_month_1 = h.hd_mon;
	header.h_year_1 = h.hd_year;

	hdate_set_jd ( &h, hdate_gdate_to_jd ( 1, month % 12 + 1, ((month==12) ? year+1 : year) ) - 1  );
	header.g_month_2 = h.gd_mon;
	header.g_year_2 = h.gd_year;
	header.h_month_2 = h.hd_mon;
	header.h_year_2 = h.hd_year;

	return header;
}


/**************************************************
 *  print header - year and month
 *************************************************/
int print_header (int month, int year, option_list opt)
{
	header_info previous_month, current_month, next_month;


	/**********************************************************
	*  Preliminary - set dates for begining and end of calendar
	**********************************************************/
	current_month = calculate_calendar_start_end(month, year);
	if (opt.three_month)
	{
		previous_month = calculate_calendar_start_end(
					(month==1 ? 12 : month-1), (month==1 ? year-1 : year) );
		next_month = calculate_calendar_start_end(
					(month==12 ? 1 : month+1), (month==12 ? year+1: year) );
	}
	
	/**************************************************
	*  HTML output - month header line
	*************************************************/
	if (opt.html) print_header_month_line_html(current_month);


	/**************************************************
	*  STDOUT output - month header line
	*************************************************/
	else
	{
		if (opt.three_month)
		{
			print_header_month_line_stdout(previous_month, opt.colorize);
			printf("  ");
		}

		print_header_month_line_stdout(current_month, opt.colorize);

		if (opt.three_month)
		{
			printf("  ");
			print_header_month_line_stdout(next_month, opt.colorize);
		}
	}

	/**************************************************
	*  terminate line
	**************************************************/
	if (opt.html) printf ("</tr>\n");
	else printf ("\n");


	/**************************************************
	*  print column headings for days of weeks
	**************************************************/
	if (opt.html) print_header_dow_line_html();
	else
	{
		if (opt.three_month)
		{
			print_header_dow_line_stdout(opt.colorize);
			printf("   ");
		}
		print_header_dow_line_stdout(opt.colorize);
		if (opt.three_month)
		{
			printf("   ");
			print_header_dow_line_stdout(opt.colorize);
		}
	}

	/**************************************************
	*  terminate line
	**************************************************/
	if (opt.html) printf ("</tr>\n");
	else printf ("\n");

	return 0;
}


/**************************************************
*  print HTML calendar entry (ie. a single day)
*************************************************/
void print_day_html ( hdate_struct h, int month, int holyday_type, option_list opt)
{
	if (h.gd_mon != month) printf ("<td class=\"out_of_month\">");
	else if (h.hd_dw == Shabbat) printf ("<td class=\"sat\">");
	else if (holyday_type) printf ("<td class=\"holiday\">");
	else printf ("<td class=\"regular\">");

	/* Print a day */
	printf ("<div class=\"gday\">%2d</div>\n<div class=\"hday\">%3s</div>", h.gd_day, hdate_get_int_string (h.hd_day));

	if (holyday_type) printf ("<div class=\"holiday_name\">%s</div>",
			hdate_get_holyday_string (hdate_get_holyday (&h, opt.diaspora), FALSE));

	printf ("</td>\n");
}


/*************************************************
*  print stdout calendar entry (ie. a single day)
*************************************************/
void print_day ( hdate_struct h, int month, int holyday_type, option_list opt)
{

#define compress 1

	char type_char[] = { '/', '+', '*', '~', '!', '@', '#', '$', '%', '^' };
/*  Holiday types: (reference hdate_holyday.c)
	/ - 0 - Regular day
	+ - 1 - Yom tov (plus yom kippor)
	* - 2 - Erev yom kippur
	~ - 3 - Hol hamoed
	! - 4 - Hanuka and purim
	@ - 5 - Tzomot
	# - 6 - Independance day and Yom yerushalaim
	$ - 7 - Lag baomer ,Tu beav, Tu beshvat
	% - 8 - Tzahal and Holocaust memorial days
	^ - 9 - National days
*/

	/**************************************************
	*  out of month - needs padding
	*************************************************/
	if (h.gd_mon != month) printf("     ");


	/**************************************************
	*  in month - print the data
	*************************************************/
	else
	{
		/*************************************************
		*  Gregorian date entry
		*************************************************/
		if ((h.hd_jd == opt.jd_today_g) && (! opt.no_reverse))
				printf (CODE_REVERSE_VIDEO);
		else if (opt.colorize)
		{
			if (h.hd_dw==7) colorize_element(ELEMENT_SHABBAT_DAY);
			else if (holyday_type) colorize_element(ELEMENT_HOLIDAY_DAY);
			else            colorize_element(ELEMENT_WEEKDAY_G);
		}
		
		// this next line is necessary to align numbers
		// correctly with bidi (tested using mlterm)
		if ( (h.gd_day < 10) && (hdate_is_hebrew_locale()) )
			 printf ("%d ", h.gd_day);
		else printf ("%2d", h.gd_day);
		
		printf (CODE_RESTORE_VIDEO);


		/*************************************************
		*  holiday flag
		*************************************************/
		printf ("%c", type_char[holyday_type]);


		/*************************************************
		*  Hebrew date entry
		*************************************************/
		if ((h.hd_jd == opt.jd_today_h) && (! opt.no_reverse))
				printf (CODE_REVERSE_VIDEO);
		else if (opt.colorize)
		{
			if (h.hd_dw==7) colorize_element(ELEMENT_SHABBAT_DAY);
			else if (holyday_type) colorize_element(ELEMENT_HOLIDAY_DAY);
			else            colorize_element(ELEMENT_WEEKDAY_H);
		}
		
		if  (hdate_is_hebrew_locale() &&
			( (h.hd_day < 11) || (h.hd_day == 20) || (h.hd_day == 30) ) )
		{	// need to pad Hebrew dates 1-10, 20, 30
			printf ("%2s%s", hdate_get_int_string_(h.hd_day,compress)," ");
		}
		else printf ("%2s", hdate_get_int_string_(h.hd_day,compress));

		printf (CODE_RESTORE_VIDEO);
	}
}


/*************************************************
*  print a calendar week's entry (ie. seven columns)
*************************************************/
void print_week( int jd, int month, option_list opt)
{
#define long_parasha_name 0

	hdate_struct h;
	int holyday_type;
	int parasha;
	int calendar_column;
	
	/**************************************************
	*  for each column of calendar
	*************************************************/
	for (calendar_column = 0; calendar_column < 7; calendar_column++)
	{
		/* Get this day's information */
		hdate_set_jd (&h, jd);

		/* Get this day holyday type ba harez
		 *  (diaspora flag = 0) */
		holyday_type =
			hdate_get_holyday_type (hdate_get_holyday
						(&h, opt.diaspora));



		/**************************************************
		*  HTML calendar option
		*************************************************/
		if (opt.html) print_day_html ( h, month, holyday_type, opt );



		/**************************************************
		*  non-HTML calendar option
		*************************************************/
		else print_day ( h, month, holyday_type, opt);


		/**************************************************
		*  space between days of week
		*************************************************/
		if (calendar_column != 6)	printf (" ");
		
		/**************************************************
		*  advance to next day of week
		*************************************************/
		jd++;
	}

	/**************************************************
	*  print end of calendar line
	*************************************************/
	
	if ( (opt.parasha) && !((h.gd_mon != month) && (h.gd_day > 7)) )
	{
		parasha = hdate_get_parasha (&h, opt.diaspora);
		if (parasha) printf("  %s", hdate_get_parasha_string (parasha, long_parasha_name));
	}
	
	//TODO -- requires lat, lon, tz logic
	// if (opt.shabbat) print candle-lighting and tzeit Shabbat
}




/**************************************************
*  print month table
*************************************************/
int print_calendar (int current_month, int current_year, option_list opt)
{
	hdate_struct h;
	int calendar_line;
	int previous_month, next_month;
	int previous_year, next_year;
	int jd_current_month, jd_previous_month, jd_next_month;
	

	/* Determine today's Julian day in order to know what to highlight  */
	if ((!opt.no_reverse) && (!opt.html))
	{
		hdate_set_gdate (&h, 0, 0, 0);
		opt.jd_today_g = h.hd_jd;
		// TODO - guess or determine if it's after sunset
		opt.jd_today_h = h.hd_jd;
	}
	else
	{
		opt.jd_today_g = 0;
		opt.jd_today_h = 0;
	}

	/**************************************************
	*  Find the sunday(s) on which to start calendar with
	**************************************************/
	hdate_set_gdate (&h, 1, current_month, current_year);
	jd_current_month = h.hd_jd - h.hd_dw + 1;

	if (opt.three_month)
	{
		if (current_month == 1)
		{
			previous_month = 12;
			previous_year = current_year - 1;
		}
		else
		{
			previous_month = current_month - 1;
			previous_year = current_year;
		}
		hdate_set_gdate (&h, 1, previous_month, previous_year);
		jd_previous_month = h.hd_jd - h.hd_dw + 1;
		
		if (current_month == 12)
		{
			next_month = 1;
			next_year = current_year + 1;
		}
		else
		{
			next_month = current_month + 1;
			next_year = current_year;
		}
		hdate_set_gdate (&h, 1, next_month, next_year);
		jd_next_month = h.hd_jd - h.hd_dw + 1;
	}
		
	

	/**************************************************
	*  maximum six lines of calendar
	**************************************************/
	for (calendar_line = 0; calendar_line < 6; calendar_line++)
	{
		if (opt.html) printf ("<tr>\n");

		if (opt.three_month)
		{
			print_week(jd_previous_month, previous_month, opt);
			jd_previous_month = jd_previous_month + 7;
			printf("  ");
		}

		print_week(jd_current_month, current_month, opt);
		jd_current_month = jd_current_month + 7;
		
		
		if (opt.three_month)
		{
			printf("  ");
			print_week(jd_next_month, next_month, opt);
			jd_next_month = jd_next_month + 7;
		}
		
		if (opt.html) printf ("</tr>\n");
		else printf ("\n");
	}


	/**************************************************
	*  print end of calendar
	*************************************************/
	if (opt.html) printf ("</table>\n</div>");

	return 0;
}






/**************************************************
* print month header and month table
*************************************************/
int print_month (int month, int year, option_list opt)
{
	hdate_struct h;

	/* check if hebrew year (year > 3000) */
	if (year > 3000)
		hdate_set_hdate (&h, 1, month, year);
	else
		hdate_set_gdate (&h, 1, month, year);

	print_header (h.gd_mon, h.gd_year, opt);
	print_calendar (h.gd_mon, h.gd_year, opt);

	printf ("\n");

	return 0;
}





/**************************************************
***************************************************
***************************************************
* main
***************************************************
***************************************************
**************************************************/
int main (int argc, char *argv[])
{
	/* date */
	int month;
	int year;

	option_list opt;
	opt.html = 0;			// -h html format flag
	opt.diaspora = 0;		// -d Diaspora holidays
	opt.external_css = 0;	// -i External css file
	opt.parasha = 0;		// -p print parasha alongside calendar
	opt.shabbat = 0;		// -c print candle-lighting alongside calendar
	opt.no_reverse = 0;		// don't highlight today in reverse video
	opt.three_month = 0;	// print previous and next months also
	opt.colorize = 0;		// display calendar in muted, more pleasing tones
	
	/* support for long options */
	int option_index = 0;
	int c;
	static struct option long_options[] = {
		{"version", 0, 0, 0},
		{"help", 0, 0, 0},
		{"no-reverse", 0, 0, 0},
		{"html", 0, 0, 0},
		{"parasha", 0, 0, 0},
		{"shabbat", 0, 0, 0},
		{"three-month", 0, 0, 0},
		{"colorize", 0, 0, 0},
		{0, 0, 0, 0}
		};

	/* hdate struct */
	hdate_struct h;

	/* init locale */
	/* WHY ??*/
	setlocale (LC_ALL, "");

	/* command line parsing */
 	while ((c = getopt_long(argc, argv, "hdi?p3c",
						long_options, &option_index)) != -1)

	{
		switch (c)
		{
		case 0: /* long options */
			switch (option_index)
			{
			case 0:	print_version (); exit (0); break;
			case 1:	print_help (); exit (0); break;
			case 2:	opt.no_reverse = 1; break;
			case 3: opt.html = 1; break;
			case 4: opt.parasha = 1; break;
			case 5: opt.shabbat = 1; break;
			case 6: opt.three_month = 1; break;
			case 7: opt.colorize = 1; break;
			}
			break;
		case 'h': opt.html = 1; break;
		case 'd': opt.diaspora = 1; break;
		case 'i': opt.external_css = 1; break;
		case 'p': opt.parasha = 1; break;
		case 'c': opt.colorize = 1; break;
		case '?': print_help (); exit (0); break;
		case '3': opt.three_month = 1; break;
		default: print_help (); exit (0); break;
		}
	}


	/**************************************************
	* sanity check - options compatability
	*************************************************/
	if (opt.three_month && (opt.parasha || opt.shabbat) )
	{
		error(0,0,"%s: %s", N_("error"), N_("options 'parasha' and 'shabbat' not supported in 'three-month' mode"));
		exit (0);
	}

	/**************************************************
	* parse and vaildate date parameters
	**************************************************/


	/**************************************************
	* no date parameter provided - use current mm yyyy
	* and no need to validate parameters
	**************************************************/
	if (argc == (optind))
	{
		hdate_set_gdate (&h, 0, 0, 0);	// current year
		month = h.gd_mon;
		year = h.gd_year;
	}
	else
	{
		/********************************************************
		* additional parameters provided - get them and validate
		********************************************************/
		if (argc == (optind + 2))
		{
			month = atoi (argv[optind]);
			year = atoi (argv[optind + 1]);
		}
		else if (argc == (optind + 1))
		{
			month = 0;
			year = atoi (argv[optind]);
		}
		else
		{
			error(0,0,"%s: %s", N_("error"), N_("too many parameters received. expected  [[mm] [yyyy]"));
			exit (0);
		}

		/********************************************************
		* parameter validation - year
		********************************************************/
		if (!hdate_validate(CHECK_YEAR_PARM, 0, month, year))
		{
			print_parm_error(N_("year"));
			exit (0);
		}
		/********************************************************
		* parameter validation - month
		********************************************************/
		if ((month!=0) && (!hdate_validate(CHECK_MONTH_PARM, 0, month, year)))
		{
			print_parm_error(N_("month"));
			exit (0);
		}
	}


/************************************************************
* begin processing the user request
************************************************************/


	/************************************************************
	* print HTML header
	************************************************************/
	if (opt.html) print_html_header (opt.external_css);


	/************************************************************
	* print one year
	************************************************************/
	if (month == 0)
	{
		if (opt.three_month)
			for (month=2; month<12; month=month+3) print_month (month, year, opt);
		else for (month=1; month<12; month++) print_month (month, year, opt);
	}

	
	/************************************************************
	* print only this month 
	************************************************************/
	else print_month (month, year, opt);


	/************************************************************
	* print HTML footer
	************************************************************/
	if (opt.html) print_html_footer ();

	return 0;
}
