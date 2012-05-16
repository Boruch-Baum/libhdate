/** local_functions.c            http://libhdate.sourceforge.net
 * a collection of functions in support of both hcal.c and hdate.c
 * hcal.c  Hebrew calendar              (part of package libhdate)
 * hdate.c Hebrew date/times information(part of package libhdate)
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` local_functions.c -o local_functions
 *
 * Copyright:  2011-2012 (c) Boruch Baum
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
//gcc -Wall -c -g -I "../../src" "%f"
//gcc -Wall -g -I "../../src" -L"../../src/.libs" -lhdate -efence -o "%e" "%f"

/**************************************************
*   functions to support hcal and hdate
**************************************************/
#define _GNU_SOURCE		/// For mempcpy, asprintf

#include <hdate.h>		/// For hebrew date (gcc -I ../../src)
#include <stdlib.h>		/// For atoi, malloc
#include <error.h>		/// For error
#include <errno.h>		/// For errno
#include <time.h>		/// For time
#include <ctype.h>		/// for isprint
#include <fnmatch.h>	/// For fnmatch
#include <string.h>		/// For mempcpy
#include <getopt.h>		/// For optarg, optind
#include <stdio.h>		/// For printf, fopen, fclose, fprintf, snprintf. FILE

#include "./timezone_functions.c"

/// support for fnmatch
#define FNM_EXTMATCH	(1 << 5)
#define FNM_NOFLAG		0

#define BAD_DATE_VALUE -1
#define BAD_COORDINATE	999
#define BAD_TIMEZONE	999
#define DELTA_LONGITUDE 30

#define EXIT_CODE_BAD_PARMS	1

#define USING_SYSTEM_TZ    -1
#define NOT_USING_SYSTEM_TZ 0


// WARNING - I had here copied a set of NLS-related
// compiler directives that I had originally copied
// from support.h in src dir. Subsequently, I needed
// that snippet for timezone_functions.c, so I
// copied the snippet there. Thus, because (and as
// long as) the file timezone_functions.c is
// #included above, the following snippet should be
// commented out
/** // copied from support.h in src dir
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
**/


#define FALSE 0
#define TRUE -1

#define HEB_YR_UPPER_BOUND 10999
#define HEB_YR_LOWER_BOUND 3000
#define JUL_DY_LOWER_BOUND 348021

/// support for options parsing
#define MIN_CANDLES_MINUTES 18
#define DEFAULT_CANDLES_MINUTES 20
#define MAX_CANDLES_MINUTES 90
#define MIN_MOTZASH_MINUTES 20
#define MAX_MOTZASH_MINUTES 90


/// for custom days text string
#define MAX_STRING_SIZE_LONG  40
#define MAX_STRING_SIZE_SHORT 15


const char* custom_days_file_text = N_("\
# custom_days configuration file for package libhdate.\n\n\
# Separate copies of this file are used by hcal (Hebrew calendar) and\n\
# hdate (Hebrew date and times of day).\n\n\
# Should you mangle this file and wish to restore its default content,\n\
# rename or delete this file and run hcal or hdate; each will automatically\n\
# regenerate the default content.\n\n\
# Your system administrator can set system-wide defaults for this file by\n\
# modifying file <not yet implemented>.\n\
# You may override all defaults by changing the contents of this file.\n\n\
# How to handle events scheduled for dates that don't occur in all\n\
# calendar years:\n\
#    0 - Do not mark the event in such a calendar year\n\
#    1 - Mark the event the following day\n\
#   -1 - Mark the event the prior day\n\
#CHESHVAN_30 = 1\n\
CHESHVAN_30 = 1\n\
#KISLEV_30   = 1\n\
KISLEV_30   = 1\n\
#FEBRUARY_29 =-1\n\
FEBRUARY_29 =-1\n\n\
# How to handle events scheduled for Adar II in a year without Adar II\n\
#    0 - Do not mark the event in such a calendar year\n\
#    1 - Mark the event in Nissan\n\
#   -1 - Mark the event the Adar\n\
#ADAR_II = -1\n\
ADAR_II = -1\n\n\
# How to handle events scheduled for Adar in a year with two Adars\n\
#    1 - Mark the event in Adar I\n\
#    2 - Mark the event in Adar II\n\
#ADAR_IN_LEAP_YEAR = 2\n\
ADAR_IN_LEAP_YEAR = 2\n\n\
# Format of Custom Day entries\n\
# Each entry consists of 17 fields, comma-delimited, on a single\n\
# 'logical' line, meaning that if you insist on splitting an entry over\n\
# more than one physical line, use a '\\' to mark the continuation. All\n\
# fields are mandatory. The fields are:\n\
#  1) day_type - H for Hebrew calendar dates, G for gregorian calendar dates.\n\
#                Additionally, types h and g are available for marking an\n\
#                event to occur on the \'nth\' \'day_of_week\' of \'month\'.\n\
#  2) start_year - the first year for this commemoration. If the day_type\n\
#                field is G, this must be a gregorian year number;\n\
#                otherwise, a Hebrew year number is expected.\n\
#  3) month -    numbered 1 - 12 for Tishrei - Ellul. Adar I/II are 13, 14\n\
#                I didn't really have to mention that 1 - 12 is Jan - Dec,\n\
#                did I? Good.\n\
#  4) day_of_month - must be zero for day_type h or g\n#\n\
# The following two fields are only for day_type h or g, and must be zero\n\
# for day_type H and G.\n\
#  5) \'nth\'     - eg. nth Shabbat in Adar, or nth Tuesday in April.\n\
#  6) day_of_week - 7=Shabbat\n#\n\
# The following four fields are NOT to be enclosed in quotes. Each is comma\n\
# delimited, like all the other fields. Fields exceeding the maximum allowed\n\
# length will be truncated. All four fields are mandatory and must have at\n\
# least one printable character. Commas and semi-colons are prohibited in order\n\
# not to confuse hdate's tabular output (option -T --tabular), which is output\n\
# in comma-delimited format. Because it is possible to have more than one\n\
# holiday and custom_day on the same calendar date, hdate outputs tabular data\n\
# between holiday and custom_day fields using a semi-colon delimiter.\n\
#  7) Hebrew_language_description     - max 40 chars\n\
#  8) Hebrew_abbreviated_description  - max 15 chars\n\
#  9) Loazi_language_description      - max 40 chars\n\
# 10) Loazi_abbreviated_description   - max 15 chars\n#\n\
# The next three fields allow the custom day to be advanced or postponed\n\
# should that day occur on a friday, Shabbat, or sunday - in the cases of\n\
# day_types H or G. Values are the number of days to advance or postpone the\n\
# commemoration, in the range -9 to +9. For day_types h or g, the plan is that,\n\
# in a future release, these fields will allow the custom day to be advanced\n\
# or postponed should that day occur on an erev khag, khag, or motzei khag.\n\
# 11) if_Shishi    (if_erev_khag)\n\
# 12) if_Shabbat   (if_khag)\n\
# 13) if_Rishon    (if_motzei_khag)\n\
# The final four fields are advancement/postponement values for the remaining\n\
# days of the week  - for the cases of day_types H or G. Values are the number\n\
# of days to advance or postpone the commemoration, in the range -9 to +9. For\n\
# day_types h or g, the plan is that, in a future release, these fields will\n\
# allow the custom day to be advanced or postponed should that day occur on the\n\
# two days prior to erev khag or after motzei khag.\n\
# 14) if_day_2     (if_2_days_after_khag)\n\
# 15) if_day_3     (if_3_days_after_khag)\n\
# 16) if_day_4     (if_3_days_prior_khag)\n\
# 17) if_day_5     (if_2_days_prior_khag)\n\
#\n\
# Examples\n\
# ========\n\
# 1] One following minhag ashkenaz for selichot can mark the first night of\n\
#    selichot as follows:\n\
H, 3001,  1,  1, 0, 0, יום ראשון של סליחות, סליחות יום א', First night of selichot, Selichot I,-5,-6,-7,-8,-9,-3,-4\n\
# 2] To mark the Shabbat prior to a yahrtzeit (for various customs)\n\
H, 5741,  6, 27, 0, 0, שבת זכרון של פלוני, זכרון פלוני, Shabbat yahrtzeit for xxxx, Shabbat yahrtzeit,-6,-7,-1,-2,-3,-4,-5\n\
#\n\
# DEBUG TESTS - REMOVE PRIOR TO RELEASE\n\
# =====================================\n\
H, 5700,  2,  6, 0, 0, יום הוקרת הפינגיונים עברי,   _, Penguin Appreciation Day Heb test,  _,  0,  0, 0, 0, 0, 0, 0\n\
H, 5700,  2,  6, 0, 0, יום הוקרת הפינגיונים עברי,   _, Duplicate day Heb test,             _,  0,  0, 0, 0, 0, 0, 0\n\
G, 1960,  1,  5, 0, 0, יום הוקרת הפינגיונים לועז,   _, Penguin Appreciation Day Greg test, _,  0,  0, 0, 0, 0, 0, 0\n\
G, 1960,  1,  5, 0, 0, יום הוקרת הפינגיונים לועז,   _, Duplicate day Greg test,            _,  0,  0, 0, 0, 0, 0, 0\n\
h, 5730,  4,  0, 2, 5, יום הוקרת הפינגיונים עברי ב, _, Penguin Appreciation Day heb test,  _,  0,  0, 0, 0, 0, 0, 0\n\
g, 1980,  5,  0, 1, 2, יום הוקרת הפינגיונים לועז ב, _, Penguin Appreciation Day greg test, _,  0,  0, 0, 0, 0, 0, 0\n\
H, 5750,  6,  8, 0, 0, פינגיונים מעדיפים יום ד,     _, Penguins Appreciate Wednesday test, _, -2, -3, 3, 0, 0, 0, 0\n\
G, 2000,  7,  1, 0, 0, פינגיונים בעד סופש ארוך,     _, Penguins want long weekends test,   _,  0, -1, 1, 0, 0, 0, 0\n\
");



/************************************************************
* begin - error message functions
************************************************************/
void print_parm_error ( char *parm_name )
{
	error(0,0,"%s: %s %s %s",N_("error"),
			N_("parameter"), parm_name,
			N_("is non-numeric or out of bounds"));
}

void print_parm_missing_error ( char *parm_name )
{
	error(0,0,"%s: %s %s %s",N_("error"),
			N_("option"), parm_name, N_("missing parameter"));
}

void print_alert_timezone( int tz )
{
	error(0,0,"%s: %s, %+d:%d UTC",
			N_("ALERT: time zone not entered, using system local time zone"),
			*tzname, tz/60, tz%60);
}

void print_parm_invalid_error( char *parm_name )
{
	error(0,0,"%s: %s %s %s",N_("error"),
			N_("option"), parm_name, N_("is not a valid option"));
}

/************************************************************
* end - error message functions
************************************************************/




/************************************************************
* set default location
************************************************************/
void set_default_location( const int tz, double *lat, double *lon,
						   const int using_system_tz, const int quiet_alerts )
{

	char* location;
	
	if (using_system_tz)
	{
		char *sys_tz_string_ptr;
		/// attempt to read name of system time zone
		sys_tz_string_ptr = read_sys_tz_string_from_file();
		if (sys_tz_string_ptr != NULL)
		{
			/// attempt to get coorinates from zone.tab file
			if (get_lat_lon_from_zonetab_file( sys_tz_string_ptr, lat, lon, quiet_alerts ) ) return;
		}
	}


	/*	Temporarily, set some default lat/lon coordinates
		for certain timezones */

/**
{ 00 }  ( Latitude : 31.78;  Longitude : 35.22;  Name : 'Jerusalem'     ),
{ 01 }  ( Latitude : 32.07;  Longitude : 34.77;  Name : 'Tel Aviv-Jafa' ),
{ 02 }  ( Latitude : 32.82;  Longitude : 34.99;  Name : 'Haifa'          ),
{ 03 }  ( Latitude : 31.96;  Longitude : 34.80;  Name : 'Rishon Lezion' ),
{ 04 }  ( Latitude : 31.80;  Longitude : 34.64;  Name : 'Ashdod'        ),
{ 05 }  ( Latitude : 31.25;  Longitude : 34.80;  Name : 'Be''er Sheva'  ),
{ 06 }  ( Latitude : 32.09;  Longitude : 34.88;  Name : 'Petach Tiqva'  ),
{ 07 }  ( Latitude : 32.33;  Longitude : 34.86;  Name : 'Netanya'       ),
{ 08 }  ( Latitude : 32.02;  Longitude : 34.76;  Name : 'Holon'         ),
{ 09 }  ( Latitude : 32.09;  Longitude : 34.85;  Name : 'B''ene Beraq'  ),
{ 10 }  ( Latitude : 32.02;  Longitude : 34.75;  Name : 'Bat Yam'       ),
{ 11 }  ( Latitude : 32.08;  Longitude : 34.80;  Name : 'Ramat Gan'     ),
{ 12 }  ( Latitude : 31.67;  Longitude : 34.56;  Name : 'Ashqelon'      ),
{ 13 }  ( Latitude : 31.89;  Longitude : 34.80;  Name : 'Rehovot'       ),
{ 14 }  ( Latitude : 32.17;  Longitude : 34.84;  Name : 'Herzeliyya'    ),
{ 15 }  ( Latitude : 32.19;  Longitude : 34.91;  Name : 'Kfar Saba'     ),
{ 16 }  ( Latitude : 32.45;  Longitude : 34.92;  Name : 'Hadera'        ),
{ 17 }  ( Latitude : 32.19;  Longitude : 34.88;  Name : 'Ra''anana'     ),
{ 18 }  ( Latitude : 31.96;  Longitude : 34.90;  Name : 'Lod'           ),
{ 19 }  ( Latitude : 31.93;  Longitude : 34.86;  Name : 'Ramla'         )
**/

	switch (tz)
	{
/** hours       minutes */
/** -8 **/	case -480:	*lat =  34.05;	*lon =-118.25;	location = N_("Los Angeles"); break;
/** -6 **/	case -360:	*lat =  19.43;	*lon = -99.13;	location = N_("Mexico City"); break;
/** -5 **/	case -300:	*lat =  40.75;	*lon = -74.0;	location = N_("New York City"); break;
/** -5 **///case -300:	*lat =  43.71;	*lon = -79.43;	location = N_("Toronto"); break;
/** -5 **///case -300:	*lat = -23.55;	*lon = -46.61;	location = N_("Sao Paolo"); break;
/** -4.5 **/case -270:	*lat =  10.54;	*lon = -66.93;	location = N_("Caracas"); break;
/** -3 **/	case -180:	*lat = -34.61;	*lon = -58.37;	location = N_("Buenos Aires"); break;
/**  0 **/	case    0:	*lat =  51.5;	*lon =   0.0;	location = N_("London"); break;
/**  1 **/	case   60:	*lat =  48.86;	*lon =   2.39;	location = N_("Paris"); break;
/**  2 **/	case  120:	*lat =  32.0;	*lon =  34.75;	location = N_("Tel-Aviv"); break;
/**  3.5 **/case  210:	*lat =  35,67;	*lon =  51.43;	location = N_("Tehran"); break;
/**  4 **/	case  240:	*lat =  55.75;	*lon =  37.62;	location = N_("Moscow"); break;
/**  5 **/	case  300:	*lat =  41.27;	*lon =  69.22;	location = N_("Tashkent"); break;
/**  5.5 **/case  330:	*lat =  22.57;	*lon =  88.36;	location = N_("Calcutta"); break;
/**  8 **/	case  480:	*lat =  39.90;	*lon = 116.38;	location = N_("Beijing"); break;
/**  8 **///case  480:	*lat =  22.26;	*lon = 114.15;	location = N_("Hong Kong");break;
/** 10 **///case  600:	*lat =  21.30;	*lon = 157.82;	location = N_("Honolulu");break;
/** 10 **/	case  600:	*lat = -33.87;	*lon = 151.21;	location = N_("Sydney");break;
	default:	/// .25 = (15 degrees) / 60 [because tz is in minutes, not hours]
				*lat =   0.0;	*lon =  tz * .25;
				if (!quiet_alerts) error(0,0,"%s \n%s", N_("Hmmm, ... hate to do this, really ..."),
				N_("using co-ordinates for the equator, at the center of time zone")); return; break;
	}
	if (!quiet_alerts) print_alert_coordinate(location);
	return;
}



/************************************************************
* check for sunset
************************************************************/
int check_for_sunset (hdate_struct * h, double lat, double lon, int timezone )
{
	time_t now_time;
	struct tm *now_timep;
	int sunrise, sunset;

	hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
							&sunrise, &sunset);

	time(&now_time);
	now_timep = localtime(&now_time);

	if ( ((now_timep->tm_hour) *60 + now_timep->tm_min) > (sunset + timezone) ) return 1;
	else return 0;
}




/***********************************************************
 *  reverse a hebrew string (for visual bidi)
 *  parameters:
 *    source is a pointer to a writeable buffer.
 *    source_len is the length of a string in source. This 
 *    length must be LESS than the size of the source buffer
 *  returns:
 *    the number of printable characters in the string.
***********************************************************/
int revstr( char *source, const size_t source_len)
{
	#define H_CHAR_WIDTH 2
	int retval = 0;

#define DEBUG 0
#if DEBUG
printf("\nrevstr: entry: sourcelen = %d, source = %s\n",source_len, source);
#endif

	if (source == NULL) {error(0,0,"revstr: source buffer pointer is NULL"); exit(0);};
	if (source_len <= 0) {error(0,0,"revstr: source_len parameter invalid, %ld",source_len); exit(0);};

	size_t i,j;
	char *temp_buff;

	temp_buff = malloc(source_len+2);
	if (temp_buff == NULL) {error(0,0,"revstr: malloc failure"); exit(0);};

	for (i=(source_len-1), j=0; j < source_len; i--, j++)
	{
		retval++;
		if (isprint(source[i])) temp_buff[j] = source[i];
		else
		{
			// FIXME - presumes H_CHAR_WIDTH = 2
			temp_buff[j++] = source[i-1];
			temp_buff[j]   = source[i--];
		}
	}
	memcpy(source, temp_buff, source_len);
	source[source_len] = '\0';
#if DEBUG
printf("\nrevstr: before free(tempbuff): sourcelen = %d, source = %s\n",source_len, source);
#endif
	free(temp_buff);
	return retval;
}


/************************************************************
* parse coordinate
* 
* presumes commandline options were parsed using getopt.
* returns 1 to *opt_found on success
* returns the floating point value of the coordinate in *coordinate
* 
* type_flag (1-latitude, 2=longitude)
************************************************************/
int parse_coordinate( const int type_flag, char *input_string,
						double *coordinate, int *opt_found)
{
//#define NEGATIVE_NUMBER_GLOB	"-[[:digit:]]?([[:digit:]]?([[:digit:]]))?(.+([[:digit:]]))"
//#define NEGATIVE_DMS_GLOB		"-[[:digit:]]?([[:digit:]]?([[:digit:]]))?([:\"]?([012345])[[:digit:]]?([:'\"]?([012345])[[:digit:]]))"
//#define COORDINATE_GLOB_DECIMAL	"?([+-])[[:digit:]]?([[:digit:]]?([[:digit:]]))?(.+([[:digit:]]))"
//#define COORDINATE_GLOB_DMS		"?([+-])[[:digit:]]?([[:digit:]]?([[:digit:]]))?([':\"]?([012345])[[:digit:]]?([:'\"]?([012345])[[:digit:]]))"

#define LATITUDE_GLOB_DECIMAL	"?([nNsS+-])[[:digit:]]?([[:digit:]]?([[:digit:]]))?(.+([[:digit:]]))?([nNsS])"
#define LONGITUDE_GLOB_DECIMAL	"?([eEwW+-])[[:digit:]]?([[:digit:]]?([[:digit:]]))?(.+([[:digit:]]))?([eEwW])"

#define LATITUDE_GLOB_DMS		"?([nNsS+-])[[:digit:]]?([[:digit:]]?([[:digit:]]))?([':\"]?([012345])[[:digit:]]?([:'\"]?([012345])[[:digit:]]))?([nNsS])"
#define LONGITUDE_GLOB_DMS		"?([eEwW+-])[[:digit:]]?([[:digit:]]?([[:digit:]]))?([':\"]?([012345])[[:digit:]]?([:'\"]?([012345])[[:digit:]]))?([eEwW])"

	char* seconds;

	double fractional_part = 0;
	int direction_modifier = 1;

	/************************************************************
	* input_string should hold the value to parse
	* It must exist and be numeric
	* we don't want confuse it with the next option of for "-x"
	************************************************************/
	if	(!input_string)
	{
		if		(type_flag == 1) print_parm_missing_error(N_("l (latitude)"));
		else if (type_flag == 2) print_parm_missing_error(N_("L (Longitue)"));
		return TRUE; /// error_detected = TRUE; ie failure
	}

	/************************************************************
	* parse latitude
	************************************************************/
	if	(type_flag == 1)
	{
		if(	(fnmatch( LATITUDE_GLOB_DECIMAL, input_string, FNM_EXTMATCH)!=0)
		&&	(fnmatch( LATITUDE_GLOB_DMS, input_string, FNM_EXTMATCH) !=0) )
		{
			if (fnmatch("-*", input_string, FNM_NOFLAG)==0)
				 print_parm_missing_error(N_("l (latitude)"));
			else print_parm_error(N_("l (latitude)"));
			return TRUE; /// error_detected = TRUE; ie failure
		}


		if ( strpbrk(input_string, "sS") != NULL ) direction_modifier = -1;


		if (fnmatch(LATITUDE_GLOB_DECIMAL, input_string, FNM_EXTMATCH)==0)
		{
			*coordinate = (double) atof( strtok( input_string, "nNsS"));
		}
		else if (fnmatch(LATITUDE_GLOB_DMS, input_string, FNM_EXTMATCH)==0)
		{
			*coordinate = (double) atof( strtok( input_string, "nNsS:'\""));
			fractional_part = (double) atof( strtok( NULL, "nNsS:'\""))/60;
			seconds = strtok( NULL, "nNsS:'\"");
			if (seconds != NULL) fractional_part = fractional_part + (double) atof( seconds)/3600;
		}


		if (*coordinate > 0)
			*coordinate = (*coordinate + fractional_part) * direction_modifier;
		else
			*coordinate = *coordinate - fractional_part;


		if(	(*coordinate > -90) && (*coordinate < 90) )
		{
			*opt_found = 1;
			return FALSE; /// error_detected = FALSE; ie. success
		}
		print_parm_error(N_("L (Longitude)"));
		return TRUE; /// error_detected = TRUE; ie failure
	}


	/************************************************************
	* parse longitude
	************************************************************/
	if	(type_flag == 2)
	{
		if(	(fnmatch( LONGITUDE_GLOB_DECIMAL, input_string, FNM_EXTMATCH)!=0)
		&&	(fnmatch( LONGITUDE_GLOB_DMS, input_string, FNM_EXTMATCH) !=0) )
		{
			if (fnmatch("-*", input_string, FNM_NOFLAG)==0)
				 print_parm_missing_error(N_("L (Longitude)"));
			else print_parm_error(N_("L (Longitude)"));
			return TRUE; /// error_detected = TRUE; ie failure
		}


		if ( strpbrk(input_string, "wW") != NULL ) direction_modifier = -1;


		if (fnmatch(LONGITUDE_GLOB_DECIMAL, input_string, FNM_EXTMATCH)==0)
		{
			*coordinate = (double) atof( strtok( input_string, "eEwW"));
		}
		else if (fnmatch(LONGITUDE_GLOB_DMS, input_string, FNM_EXTMATCH)==0)
		{
			*coordinate = (double) atof( strtok( input_string, "eEwW:'\""));
			fractional_part = (double) atof( strtok( NULL, "eEwW:'\""))/60;
			seconds = strtok( NULL, "eEwW:'\"");
			if (seconds != NULL) fractional_part = fractional_part + (double) atof( seconds)/3600;
		}

		if (*coordinate > 0)
			*coordinate = (*coordinate + fractional_part) * direction_modifier;
		else
			*coordinate = *coordinate - fractional_part;


		if(	(*coordinate > -180) && (*coordinate < 180) )
		{
			*opt_found = 1;
			return FALSE; /// error_detected = FALSE; ie. success
		}
		print_parm_error(N_("L (Longitude)"));
		return TRUE; /// error_detected = TRUE; ie failure
	}

printf("INTERNAL ERROR: coordinate parse\n");
return TRUE; /// error_detected = TRUE; ie failure
}

/************************************************************
* parse timezone
* 
* presumes commandline options were parsed using getopt.
* timezone is returned in minutes, not hours
************************************************************/
int parse_timezone( char *input_string, int *tz)
{
#define TIMEZONE_GLOB_DECIMAL	"?([+-])?(1)[[:digit:]]?(.+([[:digit:]]))"
#define TIMEZONE_GLOB_DM		"?([+-])?(1)[[:digit:]]?([:'\"]+([[:digit:]]))"

	int fractional_part;

	/************************************************************
	* input_string should hold the value to parse
	* It must exist and be numeric
	* we don't want confuse it with the next option of for "-x"
	************************************************************/
	if	((!input_string) ||
		 (	(fnmatch("-*",input_string,FNM_NOFLAG)==0) &&
			(fnmatch(TIMEZONE_GLOB_DECIMAL, input_string, FNM_EXTMATCH)!=0) &&
			(fnmatch(TIMEZONE_GLOB_DM, input_string, FNM_EXTMATCH)!=0) ) )
	{
		print_parm_missing_error(N_("z (time zone)"));
	}
	else
	{
		if (fnmatch(TIMEZONE_GLOB_DECIMAL, input_string, FNM_EXTMATCH)==0)
		{
			*tz = (int) ( atof (input_string) * 60 );
		}
		else if (fnmatch(TIMEZONE_GLOB_DM, input_string, FNM_EXTMATCH)==0)
		{
				*tz = atoi( strtok( input_string, ":'\"")) * 60;
				fractional_part = atoi( strtok( NULL, ":'\""));
				if (*tz < 0)	*tz = *tz - fractional_part;
				else			*tz = *tz + fractional_part;
		}

		if( (*tz > -720) && (*tz < 720) ) /// 720 minutes in 12 hours
		{
			return FALSE; /// error_detected = FALSE; ie. success
		}
		else print_parm_error(N_("z (timezone)"));
	}
	return TRUE; /// error_detected = TRUE; ie failure
}




/************************************************************
* validate location parameters - lat, lon, tz
*  originally, this was only for hdate, now also for hcal
*  originally, we did this only if we needed the information
*  now, because of sunset-awareness, we always perform these checks
*  if ( (opt.sun) || (opt.times) || (opt.candles) || (opt.havdalah) )
************************************************************/
void validate_location( const int opt_latitude, const int opt_Longitude,
						double *lat, double *lon,
						int *tz, const int quiet_alerts, int error_detected,
						void (*print_usage)(), void (*print_try_help)() )
{

	/// latitude and longitude must be paired
	if ( (opt_latitude) && (!opt_Longitude) )
	{
		error(0,0,"%s: %s", N_("error"), N_("valid longitude parameter missing for given latitude"));
		error_detected = TRUE;
	}
	else if ( (opt_Longitude) && (!opt_latitude) )
	{
		error(0,0,"%s: %s", N_("error"), N_("valid latitude parameter missing for given longitude"));
		error_detected = TRUE;
	}


	if (!error_detected)
	{
		/// if no timezone entered, choose guess method
		if (*tz==BAD_TIMEZONE)
		{
			/// system timezone is in seconds, but we deal in minutes
			tzset();
			*tz = timezone /-60;

			if ( (opt_latitude) && (opt_Longitude) && ( ((*lon/15)-(*tz/60)) > DELTA_LONGITUDE ) )
			{
				/**  reject system local timezone, because it's
				 *   too far from the longitude explicitly provided
				 *   by the user; guess based on longitude entered */
				*tz = (*lon /15) * 60;
			}
			if (!quiet_alerts) print_alert_timezone( *tz );
			if ( (*lat==BAD_COORDINATE) && (*lon==BAD_COORDINATE) )
			{
				// FIXME - print_alert_default_location only looks at my case-switch list
				// and ignores a successful query of /etc/timezone & zone.tab
				// now performed by set_default_location
				set_default_location( *tz, lat, lon, USING_SYSTEM_TZ, quiet_alerts );
			}
			if (!quiet_alerts) printf("\n");
		}
		else
		{	/**	we have timezone explicitly declared, either by the
			 ** user or in the config file. Now, what about latitude
			 ** and longitude? */
			if ( (opt_Longitude) && (opt_latitude) )
			{
				/// sanity-check longitude versus timezone
				/// timezone is in minutes
				if ( abs( ( (*tz/60) * 15 ) - *lon ) > DELTA_LONGITUDE )
				{
					error(0,0,"%s %d:%d %s %.3f %s",
							N_("time zone value of"), *tz/60, *tz%60,
							N_("is incompatible with a longitude of"), *lon, N_("degrees"));
					error_detected = TRUE;
				}
			}
			else
			{
				set_default_location( *tz, lat, lon, NOT_USING_SYSTEM_TZ, quiet_alerts );
				if (!quiet_alerts) printf("\n");
			}
		}
	}
	/// exit after reporting all bad parameters found */
	if (error_detected)
	{
		print_usage();
		print_try_help();
		exit(EXIT_CODE_BAD_PARMS);
	}
}


/*****************************************************************
* get custom day pointer
* 
* returns the pointer to the nth string in string_list
* or NULL on error.
*****************************************************************/

char* get_custom_day_ptr(const int index, char* string_list_ptr)
{
	return string_list_ptr + sizeof(size_t) + (sizeof(char) * ((int) *string_list_ptr) * index);
}


/************************************************************
* check validity of date parameters
* 
* parameters:
*     h_set - TRUE/FALSE indicating whether to trust that
*          parameter 'h' is properly populated. If it is
*          set false, this function can be expected to 
*          populate it with values for the beginning of
*          the year_type being validated.
*     h - hdate_struct with information for the requested
*          year's 1 Tishrei. It is used only when validating
*          Hebrew days or months, and if it is not set, this
*          function will set it, and subsequent calls can
*          depend on it.
* 
* returns FALSE ( 0) on error
* returns TRUE  (!0) on success; In the case of
*     CHECK_MONTH_PARM, the return value will be the length
*     of the Hebrew year if a Hebrew validation was requested
*     or 365 if a gregorian validation was requested.
* 
*     Also, in the case of a TRUE return from a check of a 
*     Hebrew day or month, the hdate_struct pointed to in the
*     final parameter will be set to the values for the first
*     day of that Hebrew year.
* 
* SEE ALSO parse_date for identification of these input
*     parameters.
************************************************************/
#define CHECK_DAY_PARM   1
#define CHECK_MONTH_PARM 2
#define CHECK_YEAR_PARM  3
int validate_hdate (const int parameter_to_check,
					const int day, const int month, const int year,
					int h_set, hdate_struct* h)
{

	if (year < 0) return FALSE;
	
	switch (parameter_to_check)
	{

	case CHECK_DAY_PARM:

		/************************************************************
		* check day in Hebrew date
		************************************************************/
		if (year > HEB_YR_LOWER_BOUND)
		{
			if (year > HEB_YR_UPPER_BOUND) return FALSE;
			if (!h_set) hdate_set_hdate (h, 1, 1, year);
			if ((day < 1) || (day > 30) ||
				((day > 29) && ((month==4) || (month==6) || (month==8) || (month==10) || (month==12) || (month==14))) ||
				(((h->hd_size_of_year==353) || (h->hd_size_of_year==383)) && ((month==2) || (month==3)) && (day >29)) ||
				(((h->hd_size_of_year==354) || (h->hd_size_of_year==384)) && (month==2) && (day>29)) )			
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
				((month==2) && (   (day > 29)                    ||
								 ( (day > 28) &&
								   (     ((year%4)  !=0) ||
									  (  ((year%100)==0) && ((year%400)!=0)   )   )   )  )  )
			   )
				return FALSE;
			return TRUE;
		}
		break;	


	case CHECK_MONTH_PARM:

		/************************************************************
		* check month in Hebrew date
		************************************************************/
		if (year > HEB_YR_LOWER_BOUND)
		{
			if (year > HEB_YR_UPPER_BOUND) return FALSE;
			if ((month <= 0) || (month > 14)) return FALSE;
			if (!h_set) hdate_set_hdate (h, 1, 1, year);
			if ((h->hd_size_of_year <365) && (month >12)) return FALSE;
			return h->hd_size_of_year;
		}

		/************************************************************
		* check month in Gregorian date
		************************************************************/
		else
		{
			if ((month <= 0) || (month > 12)) return FALSE;
			return 365;
		}
		break;
	
	case CHECK_YEAR_PARM:
		if (year > HEB_YR_UPPER_BOUND) return FALSE;
		return TRUE;
		break;
	}
	return FALSE;
}


/************************************************************
* parse_date
* 
* be flexible:
*	Allow yyyy mm OR mm yyyy - check strlen
*	Allow yyyy MMM OR MMM yyyy even with a 2-digit year
* 
* currently returns TRUE always, but if it encounters an
* obvious error, it reports the error and calls exit()
* 
* The logic here is fuzzy and imprecise, and does not cover
* all cases (at least yet). For example, day parameters are
* always evaluated for the gregorian 31-day possibility, and
* the month parameters are always evaluated for the Hebrew
* 14-month possiblity (yes, 13 months, but libhdate internally
* treats ADAR I/II as months 13/14).
* 
* SEE ALSO: validate_hdate for bounds checking of these values
************************************************************/
int parse_date( const char* parm_a, const char* parm_b, const char* parm_c,
					 int* ret_year, int* ret_month, int* ret_day, const int parm_cnt )
{
	int a_span, b_span, c_span, a_len, b_len, c_len, b_val, c_val;
	const char* digits = "0123456789";

	/****************************************************
	 * The Constant BASE_YEAR is for parsing a two-digit
	 * year value. Because this is a Hebrew library with
	 * Hebrew utilities, we default to a Hebrew year and
	 * presume that the user input and desire is for a
	 * Hebrew context.
	 ***************************************************/
	#define BASE_YEAR 5700

	switch (parm_cnt)
	{
	case 3:	c_span = strspn(parm_c, digits);
			c_len  = strlen(parm_c);
			if (c_len == c_span)
			{
				c_val = atoi(parm_c);
				if (c_val > HEB_YR_UPPER_BOUND)
				{
					print_parm_error(N_("year"));
					return FALSE;
				}
			} /// and fall through ...
	case 2:	b_span = strspn(parm_b, digits); b_len  = strlen(parm_b);
			if (b_len == b_span)
			{
				b_val = atoi(parm_b);
				if (b_val > HEB_YR_UPPER_BOUND)
				{
					print_parm_error(N_("year"));
					return FALSE;
				}
			} /// and fall through ...
	case 1: a_span = strspn(parm_a, digits); a_len  = strlen(parm_a); break;
	default: return FALSE;
	}


	if (a_len == a_span) /// parm 'a' is numeric
	{
		*ret_year = atoi(parm_a);
		if (*ret_year > HEB_YR_UPPER_BOUND) { print_parm_error(N_("year")); return FALSE;}
		if (parm_cnt == 1) return TRUE;

		if (*ret_year > 999)  /// parm 'a' is a year
		// TODO - verify 999 in terms of bounds checking elsewhere !!
		{
			if (b_len == b_span)	/// parm 'b' is numeric
			{
				*ret_month = b_val;
				if ( (*ret_month > 31 ) ||
				     ((parm_cnt == 2) && (*ret_month > 14 )) )
				     { print_parm_error(N_("month")); return FALSE; }
				if (parm_cnt == 2) return TRUE;
				if (c_len != c_span)	/// parm 'c' is not numeric
				{
					*ret_day = *ret_month;
					*ret_month = hdate_parse_month_text_string(parm_c);
				}
				else
				{
					if (*ret_month > 14)
					{
						*ret_day = *ret_month;
						*ret_month = c_val;
					}
					*ret_day = c_val;
				}
			}
			else					/// parm 'b' is not numeric
			{
				*ret_month = hdate_parse_month_text_string(parm_b);
				if ((parm_cnt == 2) || (ret_month == 0)) return *ret_month;
				if (c_len != c_span) return FALSE; /// parm 'c' is not numeric
				*ret_day = c_val;
				// relying on later bounds checking for day of month
			}
		}
		else if (b_len == b_span)	/// parm 'a' isn't a four-digit year AND parm 'b' is numeric
		{   
			b_val = b_val;
			if (b_val > HEB_YR_UPPER_BOUND) { print_parm_error(N_("year")); return FALSE;}

			if (parm_cnt == 2)
			{
				/// if parm 'a' can't be a month, then let's see if it's a two-digit year
				if (*ret_year > 14 )
				{
					*ret_month = b_val;
					if ( (*ret_month > 14) || (*ret_year > 99))	{print_parm_error(N_("month")); return FALSE; }
					*ret_year = *ret_year + BASE_YEAR;
					return TRUE;
				}
				// relying on later bounds checking for year
				*ret_month = *ret_year;
				*ret_year = b_val;
			}
			else
			{
				/// if parm 'a' can't be a month or day ...
				if (*ret_year > 31 )
				{
					/// maybe it's a two digit year
					if (*ret_year > 99 ) { print_parm_error(N_("month")); return FALSE; }
				  	*ret_year = *ret_year + BASE_YEAR;
				  	/// continue parsing month and day
					*ret_month = b_val;
					if (*ret_month > 31 ) 
					     { print_parm_error(N_("month")); return FALSE; }
					if (c_len != c_span)	/// parm 'c' is not numeric
					{
						*ret_day = *ret_month;
						*ret_month = hdate_parse_month_text_string(parm_c);
					}
					else
					{
						if (*ret_month > 14)
						{
							*ret_day = *ret_month;
							*ret_month = c_val;
						}
					*ret_day = c_val;
					}
				  	return TRUE;
				}
				/// parm 'a' could only be a day
				if (*ret_year > 14 )
				{
					*ret_day = *ret_year;
					*ret_month = b_val;
					if (*ret_month > 31 )
					{
						// TODO - verify 1000 in terms of bounds checking elsewhere !!
						if (*ret_month < 1000 ) *ret_month = *ret_month + BASE_YEAR;
						*ret_year = *ret_month;
						///parm c must be month
						if (c_len == c_span) *ret_month = c_val;
						else *ret_month = hdate_parse_month_text_string(parm_c);
					}
					else if (*ret_month < 15 )
					{
						///parm_c must be year
						if (c_len != c_span)  { print_parm_error(N_("year")); return FALSE; }
						*ret_year = c_val;
					}
					else { print_parm_error(N_("month")); return FALSE; }
				}
				else /// parm 'a' may be a day or month; parm 'b' is numeric
				{
					*ret_day = *ret_year;
					*ret_month = b_val;
					if (*ret_month > 31 )
					{
						// TODO - verify 1000 in terms of bounds checking elsewhere !!
						if (*ret_month < 1000 ) *ret_month = *ret_month + BASE_YEAR;
						*ret_year = *ret_month;
						if (c_len == c_span) *ret_month = c_val;
						else *ret_month = hdate_parse_month_text_string(parm_c);
					}
					else
					{
						if ( ( (*ret_month > 14) && (*ret_day > 14) ) )
						   { print_parm_error(N_("month")); return FALSE; }
						/// parm 'c' must be a year
						if (c_len != c_span) { print_parm_error(N_("year")); return FALSE; }
						*ret_year = c_val;
					}
				}
				
			}
		}
	}
	else /// parm 'a' is not numeric - might be month, in text
	{
		if (parm_cnt == 1)  { print_parm_error(N_("year")); return FALSE; }
		*ret_month = hdate_parse_month_text_string(parm_a);
		if (!ret_month) { print_parm_error(N_("month")); return FALSE; }
		/// parm 'a' successful return from parse_month_text_string
		if (b_len != b_span)  { print_parm_error(N_("year")); return FALSE; }	/// parm 'b' is not numeric
		*ret_year = b_val;
		if (parm_cnt == 2)
		{
			/// if only 2 parms, parm 'b' must be year
			if (*ret_year < 100) *ret_year = *ret_year + BASE_YEAR;
			// check sanity of 1000 as bounds for year
			else if (*ret_year < 1000 )
				{print_parm_error(N_("year")); return FALSE;}
			return TRUE;
		}
		else /// parm_cnt == 3
		{
			if (c_len != c_span)
				 /// both parms 'b' and 'c' are not numeric
				 { print_parm_error(N_("year")); return FALSE; };
			if (*ret_year > 31) /// parm 'b' can't be a day
			{
				/// parm 'b' can't be a year either = error
				if (*ret_year < 100) *ret_year = *ret_year + BASE_YEAR;
				// check sanity of 1000 as bounds for year
				else if (*ret_year < 1000 )
					{print_parm_error(N_("year")); return FALSE;}
				if (c_val > 31) return FALSE;
				*ret_day = c_val;
				return TRUE;
			}
			*ret_day = *ret_year;
			*ret_year = c_val;
			if (*ret_year > 999) return TRUE;
			else if (*ret_year < 100) *ret_year = *ret_year + BASE_YEAR;
			else return FALSE;
		}
	}
return TRUE;
}



/************************************************************
* Greeting message to new version
************************************************************/
void greetings_to_version_16()
{
	printf("%s\n",N_("\
This seems to be to be your first time using this version.\n\
Please read the new documentation in the man page and config file.\n\
Press <enter> to continue."));
	getchar();
}



/************************************************************
* test_print_custom_days
* 
* This is a debug routine
************************************************************/
void test_print_custom_days(int custom_days_cnt, int* jdn_list_ptr, char* string_list_ptr)
{
	printf("       entered test print routine\n       number of custom_days = %d\n",custom_days_cnt);
	size_t print_len;
	int i, day, month, year;


	if (!custom_days_cnt) return;
	print_len = (size_t) *string_list_ptr;
	string_list_ptr = string_list_ptr + sizeof(size_t);
	for (i=0; i<custom_days_cnt; i++)
	{
		hdate_jd_to_gdate ( *jdn_list_ptr, &day, &month, &year);
		printf("       %d: %d %4d-%02d-%02d %s\n",i,*jdn_list_ptr,year,month,day,string_list_ptr);
		jdn_list_ptr = jdn_list_ptr + 1;
		string_list_ptr = string_list_ptr + (sizeof(char)*print_len);
	}
}



/************************************************************
* read_custom_days_file() - parse 'custom days' file
*      scans the custom_days file once.
*      For every legitimate parse, determines if that custom day
*      is in the requested date range.
*      If so,
*      1   gets the julian_day_number and store it in a malloc'ed array
*      2   use asprintf to append/realloc one of the four description
*          strings for that entry to a second buffer (which will also
*          need to be freed)
*      whether to use a line delimiter or a NULL between strings?
*      Put the pointer to the final jdn_list (or NULL) in jdn_list_ptr
*      Put the pointer to the final string_list (or NULL) in string_list_ptr
*      return the number of items found
************************************************************/
//	TODO - handle CHESHVAN_30, KISLEV_30, FEBRUARY_29, ADAR_II, ADAR_IN_LEAP_YEAR
//  TODO - advancements/postponements for day_types h/g when erev khag, khag, or motzei khag (in the
int read_custom_days_file(
			FILE* config_file,
			int** jdn_list_ptr, char** string_list_ptr,
			const int d_todo, const int m_todo, const int y_todo,
			const char calendar_type,
			hdate_struct h1,
			const int text_short_form, const int text_hebrew_form)
{
///Values defined in libhdate (hdate_strings.c)
/// #define HDATE STRING_SHORT   1
/// #define HDATE_STRING_LONG    0
/// #define HDATE_STRING_HEBREW  1
/// #define HDATE_STRING_LOCAL   0
#define NUMBER_OF_CUSTOM_DAYS_FIELDS 17

	char*	input_string = NULL;
	size_t	input_str_len;

	int		line_count = 0; // remove. for debugging only.
	int		match_count;
	int		end_of_input_file = FALSE;
	int		number_of_items = 0;
	char	custom_day_type = '\0'; 		/// H, G, h, g
	int		custom_start_year = 0;
	int		custom_month = 0;
	int		custom_day_of_month = 0;
	int		custom_nth = 0 ;				/// 1 <= n <=  5
	int		custom_day_of_week = 0;
	char*	heb_long_desc = NULL;
	char*	heb_abbr_desc = NULL;
	char*	eng_long_desc = NULL;
	char*	eng_abbr_desc = NULL;

	/// adj[] - array defining weekend adjustments
	/// valid values are -9 <= n <= 9
	#define WHEN_SHISHI  5
	#define WHEN_SHABBAT 6
	#define WHEN_RISHON  0
	#define WHEN_DAY_2   1
	#define WHEN_DAY_3   2
	#define WHEN_DAY_4   3
	#define WHEN_DAY_5   4
	int adj[7] = {0,0,0,0,0,0,0};

	int jd;
	hdate_struct temp_h;

	char* print_ptr;
	size_t print_len;
	int* new_jdn_list_ptr = NULL;
	int* jdn_entry = NULL;
	char* new_string_ptr = NULL;
	size_t string_list_buffer_size = sizeof(size_t); /// The first atom of this buffer is the array element size
	size_t string_list_index = sizeof(size_t);
	#define LIST_INCREMENT        10


	/************************************************************
	* sub-procedure for read_custom_days_file()
	* bounds check the fields for advancing/postponing
	* a custom_day
	************************************************************/
	int validate_adjustments()
	{
		int i;
		for (i=0;i<7;i++)
		{
			if ( (i<-9) || (i>9) ) return FALSE;
		}
		return TRUE;
	}

	/************************************************************
	* sub-procedure for read_custom_days_file()
	************************************************************/
	void free_my_mallocs()
	{
		if (heb_long_desc != NULL) {free(heb_long_desc);heb_long_desc = NULL;}
		if (heb_abbr_desc != NULL) {free(heb_abbr_desc);heb_abbr_desc = NULL;}
		if (eng_long_desc != NULL) {free(eng_long_desc);eng_long_desc = NULL;}
		if (eng_abbr_desc != NULL) {free(eng_abbr_desc);eng_abbr_desc = NULL;}
	}


	/************************************************************
	* begin main section of procedure read_custom_days_file()
	************************************************************/
	*jdn_list_ptr = NULL;
	*string_list_ptr = NULL;

	/// set the size of each element in the text buffer array
	if   (text_short_form)	print_len = MAX_STRING_SIZE_SHORT;
	else 					print_len = MAX_STRING_SIZE_LONG;



	while ( end_of_input_file != TRUE )
	{
		free_my_mallocs();
		end_of_input_file = getline(&input_string, &input_str_len, config_file);
		if ( end_of_input_file != TRUE)
		{
			errno = 0;
			match_count = sscanf(input_string, "%1[gGhHY], %u, %u, %u, %u, %u, %m[^,] , %m[^,] , %m[^,] , %m[^,] , %i, %i, %i, %i, %i, %i, %i",
				&custom_day_type, &custom_start_year, &custom_month, &custom_day_of_month, &custom_nth, &custom_day_of_week,
				&heb_long_desc, &heb_abbr_desc, &eng_long_desc, &eng_abbr_desc,
				&adj[WHEN_SHISHI], &adj[WHEN_SHABBAT], &adj[WHEN_RISHON],
				&adj[WHEN_DAY_2], &adj[WHEN_DAY_3], &adj[WHEN_DAY_4], &adj[WHEN_DAY_5]);
			line_count++;
			if (errno)
			{
				error(0,errno,"scan error at line %d of custom days file", line_count);
				continue; // exit(0); // FIXME - why so fatal?
			}
			/* DEBUG  	if (match_count)
				printf("line number = %d, matches made = %d\n\tday_type = %c start_year = %d month = %d, day_of_month = %d\n\tnth = %d day_of_week = %d\n\t%s\n\t%s\n\t%s\n\t%s\n\tif_fri = %d if_sbt = %d if_sun = %d\n\n",
					line_count, match_count, custom_day_type, custom_start_year, custom_month, custom_day_of_month, custom_nth, custom_day_of_week,
					heb_long_desc, heb_abbr_desc, eng_long_desc, eng_abbr_desc,
					adj[WHEN_SHISHI], adj[WHEN_SHABBAT], adj[WHEN_RISHON]); */

			if (match_count != NUMBER_OF_CUSTOM_DAYS_FIELDS) continue;

			/************************************************************
			* At this point, we have successfully scanned/parsed a line
			* and are ready to begin sanity and bounds checking. The
			* switch() validates all of the parameters scanned/parsed,
			* converts the custom_day to the julian_day_number of the
			* relevant year, and applies any advancements/postponements.
			************************************************************/
			switch ( custom_day_type )
			{
			case 'G':
				if ((!validate_hdate(CHECK_YEAR_PARM, custom_day_of_month, custom_month, custom_start_year, TRUE, &h1))  ||
					(!validate_hdate(CHECK_MONTH_PARM, custom_day_of_month, custom_month, h1.gd_year, TRUE, &h1)       ) ||
					(!validate_hdate(CHECK_DAY_PARM, custom_day_of_month, custom_month, h1.gd_year, TRUE, &h1)         ) ||
					(!validate_adjustments())      )
					continue;
				hdate_set_gdate( &temp_h, custom_day_of_month, custom_month, h1.gd_year);
				jd = temp_h.hd_jd + adj[temp_h.hd_dw-1];
				if (adj[temp_h.hd_dw-1]) hdate_set_jd(&temp_h, jd);
				if ( (!d_todo) && (y_todo > HEB_YR_LOWER_BOUND) && (y_todo != temp_h.hd_year) )
				{
					hdate_set_gdate( &temp_h, custom_day_of_month, custom_month, h1.gd_year+1);
					jd = temp_h.hd_jd + adj[temp_h.hd_dw-1];
					if (adj[temp_h.hd_dw-1]) hdate_set_jd(&temp_h, jd);
				}
				break;
			case 'H':
				if ((!validate_hdate(CHECK_YEAR_PARM, custom_day_of_month, custom_month, custom_start_year, TRUE, &h1))  ||
					(!validate_hdate(CHECK_MONTH_PARM, custom_day_of_month, custom_month, h1.hd_year, TRUE, &h1)       ) ||
					(!validate_hdate(CHECK_DAY_PARM, custom_day_of_month, custom_month, h1.hd_year, TRUE, &h1)         ) ||
					(!validate_adjustments())     )
					continue;
				hdate_set_hdate( &temp_h, custom_day_of_month, custom_month, h1.hd_year);
				jd = temp_h.hd_jd + adj[temp_h.hd_dw-1];
				if (adj[temp_h.hd_dw-1]) hdate_set_jd(&temp_h, jd);
				if ( (!d_todo) && (y_todo <= HEB_YR_LOWER_BOUND) && (y_todo != temp_h.gd_year) )
				{
					hdate_set_hdate( &temp_h, custom_day_of_month, custom_month, h1.hd_year+1); //?
					jd = temp_h.hd_jd + adj[temp_h.hd_dw-1];
					if (adj[temp_h.hd_dw-1]) hdate_set_jd(&temp_h, jd);
				}
				break;
			case 'g':
				if ((!validate_hdate(CHECK_YEAR_PARM, custom_day_of_month, custom_month, custom_start_year, TRUE, &h1))  ||
					(!validate_hdate(CHECK_MONTH_PARM, 1, custom_month, h1.gd_year, TRUE, &h1)                         ) ||
					(custom_nth < 1) || (custom_nth > 5) ||
					(custom_day_of_week < 1) || (custom_day_of_week > 7) ||
					(!validate_adjustments())      )
					continue;
				hdate_set_gdate( &temp_h, 1, custom_month, h1.gd_year);
				jd = temp_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - temp_h.hd_dw);
				hdate_set_jd(&temp_h, jd);
				if ((custom_nth==5) && ( custom_month != temp_h.gd_mon))
				{
					if ( (!d_todo) && (y_todo > HEB_YR_LOWER_BOUND) && (y_todo != temp_h.hd_year) )
					{
						hdate_set_gdate( &temp_h, 1, custom_month, h1.gd_year+1);
						jd = temp_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - temp_h.hd_dw);
						hdate_set_jd(&temp_h, jd);
						if ((custom_nth==5) && ( custom_month != temp_h.gd_mon)) continue;
					}
					else continue;
				}
				else if ( (!d_todo) && (y_todo > HEB_YR_LOWER_BOUND) && (y_todo != temp_h.hd_year) )
				{
					hdate_set_gdate( &temp_h, 1, custom_month, h1.gd_year+1);
					jd = temp_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - temp_h.hd_dw);
					hdate_set_jd(&temp_h, jd);
					if ((custom_nth==5) && ( custom_month != temp_h.gd_mon)) continue;
				}
				// perform adjustments and verify (again!?!)
				break;
			case 'h':
				if ((!validate_hdate(CHECK_YEAR_PARM, custom_day_of_month, custom_month, custom_start_year, TRUE, &h1) ) ||
					(!validate_hdate(CHECK_MONTH_PARM, 1, custom_month, h1.hd_year, TRUE, &h1)                          )||
					(custom_nth < 1) || (custom_nth > 5) ||
					(custom_day_of_week < 1) || (custom_day_of_week > 7) ||
					(!validate_adjustments())                )
					continue;
				hdate_set_hdate( &temp_h, 1, custom_month, h1.hd_year);
				jd = temp_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - temp_h.hd_dw);
				hdate_set_jd(&temp_h, jd);
				if ((custom_nth==5) && ( custom_month != temp_h.hd_mon))
				{
					if ( (!d_todo) && (y_todo <= HEB_YR_LOWER_BOUND) && (y_todo != temp_h.gd_year) )
					{
						hdate_set_hdate( &temp_h, 1, custom_month, h1.hd_year+1);
						jd = temp_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - temp_h.hd_dw);
						hdate_set_jd(&temp_h, jd);
						if ((custom_nth==5) && ( custom_month != temp_h.hd_mon)) continue;
					}
					else continue;
				}
				else if ( (!d_todo) && (y_todo <= HEB_YR_LOWER_BOUND) && (y_todo != temp_h.gd_year) )
				{
					hdate_set_hdate( &temp_h, 1, custom_month, h1.hd_year+1);
					jd = temp_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - temp_h.hd_dw);
					hdate_set_jd(&temp_h, jd);
					if ((custom_nth==5) && ( custom_month != temp_h.hd_mon)) continue;
				}
				// perform adjustments and verify (again!?!)
				break;
			default:
				// report system error; possibly intentionally crash
				break;
			}
//printf("parsed. d=%d, m=%d, y=%d, temp_h.hd_year=%d temp_h.gd_year=%d\n",d_todo,m_todo,y_todo,temp_h.hd_year,temp_h.gd_year);

			/************************************************************
			* At this point, we have computed the jd of the occurrence
			* of the parsed line's custom_day for the year in question.
			* Now we range check it against the interval being processed
			* - either a single day, single month, or single year. This
			* is non-trivial because the interval and the parse line may
			* have been denominated in different calendars.
			* We are depending upon h1, a parameter passed to us, being
			* the correct hdate_struct for the first day of the interval.
			* We compute the hdate_struct for our jd (if hadn't already
			* been done in the switch for sanity and bounds checking) and
			* compare its values to the interval parameters passed to us
			************************************************************/
			if (d_todo)
			{
				/// interval is a single day
				if (jd != h1.hd_jd) continue;
			}
			else
			{
				/// our custom_day is too early
				if (jd < h1.hd_jd) continue;

				if (m_todo)
				{
					/// interval is for an entire month
					if (y_todo > HEB_YR_LOWER_BOUND)
					{
						if ( m_todo != temp_h.hd_mon) continue;
					}
					else
					{
						if ( m_todo != temp_h.gd_mon) continue;
					}
				}
				else
				{
					/// (!m) interval is for an entire year
					if (y_todo > HEB_YR_LOWER_BOUND)
					{
						if (y_todo != temp_h.hd_year) continue;
					}
					else if ( y_todo != temp_h.gd_year) continue;
				}
			}
//printf("this item appropriate to store\n");

			/************************************************************
			* If we get this far, we can add this custom day to the list
			* to be returned, and be confident that it will be needed and
			* used.
			* 1] increment number of items in list
			* 2] add the day's jdn to our malloc'ed array
			* 3] determine which of the four text strings are desired
			* 		(Heb/Local Long/Short)
			* 4] realloc/append the text string to string_list
			* 
			* Values defined in libhdate (hdate_strings.c)
			*	HDATE STRING_SHORT   1
			*	HDATE_STRING_LONG    0
			*	HDATE_STRING_HEBREW  1
			*	HDATE_STRING_LOCAL   0
			************************************************************/
			/// manage jdn target buffer size
			if (!(number_of_items%LIST_INCREMENT))
			{
//printf("about to alloc for jdn. current pointer address: %ld\n", (long)*jdn_list_ptr);
				new_jdn_list_ptr = realloc(*jdn_list_ptr, sizeof(int)*(number_of_items + LIST_INCREMENT));
				if (new_jdn_list_ptr == NULL)
				{
					/// realloc has failed. However, the original pointer and
					/// all its data are supposed to be fine. For now, let's
					/// silently abort reading the file because, in practice
					/// if we've really exhausted memory, we're about to
					/// seriouly crash anyway
					// TODO - consider issuing a warning / aborting
					free_my_mallocs(); // the four string pointers
					return number_of_items;
				}
				else
				{
					*jdn_list_ptr = new_jdn_list_ptr;
					jdn_entry = ((int*) new_jdn_list_ptr) + number_of_items;
					new_jdn_list_ptr = NULL;
//printf("alloc for jdn succeeded. current pointer address: %ld\n", (long)*jdn_list_ptr);
				}
			}

//printf("about to store jdn (number of items = %d)\n",number_of_items);
			/// store the custom_day's jdn
			*jdn_entry = jd;
			jdn_entry = jdn_entry + 1;

			/// decide which text string to store
			if (text_short_form)
			{
				if (text_hebrew_form) print_ptr = heb_abbr_desc;
				else print_ptr = eng_abbr_desc;
			}
			else
			{
				if (text_hebrew_form) print_ptr = heb_long_desc;
				else print_ptr = eng_long_desc;
			}


			/// manage 'text string' target buffer size
			if (!(number_of_items%LIST_INCREMENT))
			{
				string_list_buffer_size = string_list_buffer_size +
						sizeof(char) * ( (print_len+1) * LIST_INCREMENT);
//printf("about to alloc %ld bytes for text. current pointer address: %ld\n", string_list_buffer_size, (long) *string_list_ptr);
				new_string_ptr = realloc(*string_list_ptr, string_list_buffer_size);
//printf("realloc exited\n");
				if (new_string_ptr == NULL)
				{
					/// realloc has failed. However, the original pointer and
					/// all its data are supposed to be fine. For now, let's
					/// silently abort reading the file because, in practice
					/// if we've really exhausted memory, we're about to
					/// seriouly crash anyway
					// TODO - consider issuing a warning / aborting
					free_my_mallocs(); // the four string pointers
					return number_of_items;
				}
				else
				{
					*string_list_ptr = new_string_ptr;
//printf("realloc succeeded. current pointer address: %ld\n", (long) *string_list_ptr);
					// FIXME - should only have to do this once,
					//         upon the initial m(re)alloc.
					size_t* print_len_ptr = (size_t*) new_string_ptr;
					*print_len_ptr = print_len + 1;
					new_string_ptr = NULL;
				}
			}
//printf("about to store text at index %ld, print_len=%ld\n",string_list_index,print_len);
			/// store the custom_day's text_string
			memcpy(*string_list_ptr + string_list_index, print_ptr, print_len);
			string_list_index = string_list_index + print_len;
// printf("index is now %ld\n",string_list_index);
			memset(*string_list_ptr + string_list_index, '\0', 1);
			string_list_index = string_list_index + 1;
			number_of_items++;
//test_print_custom_days(number_of_items, *jdn_list_ptr, *string_list_ptr);
		}
	}
	return number_of_items;
}


/************************************************************
* Generic read config key file
* can be used in common for hcal and hdate config file by
* passing a function argument with the line parsing
* at line marked // parse_custom_line_here( input_key, input_value )
************************************************************/
void generic_read_config_key_file( FILE* config_file )
{ ;
	char	*input_string = NULL;
	size_t	input_str_len;
	char	*input_key;
	char	*input_value;

	int		line_count = 0;
	int		match_count;
	int		end_of_input_file = FALSE;

	while ( end_of_input_file!=TRUE )
	{
		end_of_input_file = getline(&input_string, &input_str_len, config_file);
		if ( end_of_input_file!=TRUE )
		{
			errno = 0;
			match_count = sscanf(input_string,"%m[A-Z_]=%m[^\n]",&input_key,&input_value);
			line_count++;
			if (errno != 0) error(0,errno,"scan error at line %d", line_count);
// DEBUG - 	printf("line number = %d, matches made = %d, key = %s, value = %s, string = %s",
//					line_count, match_count, input_key, input_value, input_string);
			if (match_count == 2)
			{
				// parse_custom_line_here( input_key, input_value )
				free(input_value);
			}
			if (match_count > 0 ) free(input_key);
		}
	}
	if (input_string != NULL ) free(input_string);
	return;
}



/************************************************************
* Open config file, or create one
*  - returns filepointer or NULL
*  - if file does not exist, attempt to create it
************************************************************/
FILE* get_config_file(	const char* config_dir_name,
						const char* config_file_name,
						const char* default_config_file_text,
						const int quiet_alerts )
{
#include <pwd.h>			// for get pwuid
#include <unistd.h>			// for getuid
#include <sys/stat.h>
#include <sys/types.h>		// for mkdir,

	size_t path_len;

	char* config_home_path_name = "";
	char* config_sub_path_name = "";

	char* config_dir_path;
	char* config_file_path;

	FILE* config_file;

	/************************************************************
	* sub-function to get_config_file: create_config_file
	************************************************************/
	// FIXME - identify name of config file being created
	//         hdaterc or custom_days, or....
	void create_config_file()
	{
		if (!quiet_alerts) printf("Attempting to create a config file ...\n");
 		config_file = fopen(config_file_path, "a");
		if (config_file == NULL)
		{
			if (!quiet_alerts) error(0, errno, "%s: %s", N_("failure attempting to create config file"), config_file_path);
			return;
		}
		fprintf(config_file, default_config_file_text);
		if (fclose(config_file) != 0)
		{
			if (!quiet_alerts) error(0,errno,"%s %s",N_("failure closing"),config_file_name);
		}
		else if (!quiet_alerts) printf("%s: %s\n\n",N_("created config file:"), config_file_path);
	}

	/************************************************************
	* sub-function to get_config_file: open_config_file
	************************************************************/
	int open_config_file()
	{
		config_file = fopen(config_file_path, "r");
		if (config_file == NULL)
		{
			if (errno != ENOENT) return FALSE;
			// maybe replace all this with a single line asprintf()
			path_len = strlen(config_home_path_name)
						+ strlen(config_sub_path_name)
						+ strlen(config_dir_name) +1;
			if (path_len < 1) return FALSE;
			config_dir_path = malloc(path_len);
			if ( (config_dir_path == NULL) && (!quiet_alerts) )
			{
				error(0,errno,"%s",N_("memory allocation failure"));
				return FALSE;
			}
			snprintf(config_dir_path, path_len, "%s%s%s",
					config_home_path_name, config_sub_path_name,
					config_dir_name);
	
			if ((mkdir(config_dir_path, (mode_t) 0700) != 0) &&
				(errno != EEXIST))
			{
				free(config_dir_path);
				return FALSE;
			}
			greetings_to_version_16();
			create_config_file();
			free(config_dir_path);
			return FALSE;
		}
		return TRUE;
	}


/************************************************************
* main part of function get_config_file
************************************************************/
	config_home_path_name = getenv("XDG_CONFIG_HOME");
	if (config_home_path_name == NULL)
	{
		config_home_path_name = getenv("HOME");
		if (config_home_path_name == NULL)
		{
			struct passwd *passwdEnt = getpwuid(getuid());
			config_home_path_name = passwdEnt->pw_dir;
			if (config_home_path_name == NULL) return NULL;
		}
		config_sub_path_name = "/.config";
	}

	// maybe replace all this with a single line asprintf()
	path_len = strlen(config_home_path_name) + strlen(config_sub_path_name)
				+ strlen(config_dir_name) + strlen(config_file_name) + 1;
	if (path_len < 1) return NULL;
	config_file_path = malloc(path_len);
	if (( config_file_path == NULL) && (!quiet_alerts) )
	{
		error(0,errno,"memory allocation failure");
		return NULL;
	}
	snprintf(config_file_path, path_len, "%s%s%s%s",
			config_home_path_name, config_sub_path_name,
			config_dir_name, config_file_name);

	if (open_config_file() == TRUE)
	{
		free(config_file_path);
		return config_file;
	}
	else
		free(config_file_path);
		return NULL;
}




/****************************************************
* read, parse and filter custom_days file
****************************************************/
int get_custom_days_list( int** jdn_list_ptr, char** string_list_ptr,
			const int day, const int month, const int year,
			const char calendar, const int quiet_alerts,
			const hdate_struct range_start,
			const char* config_dir, const char* config_filename,
			const int text_short_form, const int text_hebrew_form )
{
	int number_of_items = 0;

	// FIXME - create an option for both hcal/hdate to allow a custom path for this file
	//         pass the default FULL path or the custom FULL path to read_custom_days_file
	//         and let THAT function open and close the file
	FILE *custom_file = get_config_file(config_dir, config_filename, custom_days_file_text, quiet_alerts);
	if (custom_file != NULL)
	{
		number_of_items = read_custom_days_file(custom_file, jdn_list_ptr, string_list_ptr,
										day, month, year, calendar, range_start,
										text_short_form, text_hebrew_form );
		fclose(custom_file);
	}

	return number_of_items;
}



/**********************************************************************
* parse user's menu selection into options and their posibile
* arguments, which can then be parsed (processed really)
*
* menuptr points to the beginning of the menu entry string. This
*       pointer should initially be set to NULL.
* menu_len is the length of the menu entry string
* menu_index is the current position of the parser within the string.
*       This value should initially be set to 0.
* optptr is a pointer into which we put the location of option text
*       to be parsed. This pointer should initially be set to NULL.
* optarg is a pointer to a possible argument to the option. This is a
*       global variable defined by GNU getopt(), so it shouldn't be
*       declared or passed.
* long_options is a pointer to a structure of type option, as defined
*       by getopt_long()
* long_option_index is a pointer to position found in long_options, and
*       simulates the similar value returned by getopt_long()
* error_detected is a pointer to an accumulator used to delay aborting
*       the program until after ALL input syntax errors are reported.
* 
* RETURN VALUES:
*	-1 means the menu line string has been completely parsed.
*	Otherwise, the return value should be the short option character
*	found, or in the case of a long option found, 0.
*	This should sound suspiciously similar to the return values issued
*   by getopt_long().
***********************************************************************/
int menu_item_parse(char* menuptr, size_t menu_len, int *menu_index,
					char** optptr, char* short_options,
					struct option *long_options, int *long_option_index,
					int *error_detected)
{
	int ahead_index;
	char *ahead;

	while ( (*menu_index < menu_len) &&
			(menuptr[*menu_index] != '#') )
	{
		if (menuptr[*menu_index] == '-')
		{
			if ( ( (*menu_index+1) >= menu_len) ||
				(menuptr[*menu_index+1] == '#') ) break;
			else if (menuptr[*menu_index+1] == '-')
			{
				if ( ( (*menu_index+2) >= menu_len) ||
					(menuptr[*menu_index+2] == '#') ) break;
				*menu_index = *menu_index +2;

				/**************************************************
				* We have identified the beginning of what could
				* be a long option string. We store a pointer to
				* it in optptr and proceed to:
				*   find its endpoint
				*   terminate it with a null
				*   if it has an argument, store a pointer to
				* it in optarg and terminate it with a null.
				* Then we compare the optptr to our list of
				* valid longoptions and pass the information
				* to the option parser
				*************************************************/
				*optptr = &menuptr[*menu_index]; // ??
				ahead = *optptr;
				ahead_index = *menu_index;
				while ( (ahead_index < menu_len) &&
						(strchr(" =#",*ahead) == NULL) )
					{
						ahead_index++;
						ahead++;
					}
				if (memcmp(ahead,"=",1)==0)
				{
					*ahead = '\0';
					ahead++;
					optarg = ahead;
					ahead_index++;
					while ( (ahead_index < menu_len) &&
							(strchr(" #",*ahead) == NULL) )
					{
						ahead_index++;
						ahead++;
					}
				}
				if ((*ahead == '#' ) && ((ahead_index+1) < menu_len))
					ahead[1] = '#';
				*ahead = '\0';
				*long_option_index = 0;
				while ( (long_options[*long_option_index].name != 0) &&
						(strcmp(long_options[*long_option_index].name, *optptr) != 0) )
					*long_option_index = *long_option_index + 1;
				if (long_options[*long_option_index].name == 0)
				{
					*error_detected = *error_detected + 1;
					print_parm_invalid_error(*optptr);
				}
				else
				{
					*menu_index = ahead_index;
					return long_options[*long_option_index].val;
				}
			}
			else
			{
				*menu_index = *menu_index + 1;
				/**************************************************
				* parse a short option from a menu line
				*************************************************/
				if ((strchr(short_options, (int) menuptr[*menu_index]) == NULL) ||
					(menuptr[*menu_index] == ':') )
				{
					*error_detected = *error_detected + 1;
					print_parm_invalid_error(*optptr);
				}
				else
				{
					*optptr = &menuptr[*menu_index];
					*long_option_index = -1;
					if (strchr("lLz", (int) menuptr[*menu_index]) != NULL)
					{
						ahead_index = *menu_index+1;
						while ( (ahead_index < menu_len) &&
								(memcmp(&menuptr[ahead_index]," ",1)==0) ) ahead_index++;
						optarg = &menuptr[ahead_index];
						while ( (ahead_index < menu_len) &&
								(memcmp(&menuptr[ahead_index]," ",1)) ) ahead_index++;
						menuptr[ahead_index] = '\0';
						*menu_index = ahead_index;
					}
					return **optptr;
				}

			}
		}
		*menu_index = *menu_index + 1;
	}
	return -1;
}


/**************************************************
* prompt user for menu selection
*************************************************/
int menu_select( char* menu_list[], int max_menu_items )
{
	int i, j;

	for (i=0; i< max_menu_items; i++)
	{
		if (menu_list[i] == NULL) break;
		if (i==0) printf("\n%s:\n\n", N_("your custom menu options (from your config file)"));
		printf("     %d: %s\n",i,menu_list[i]);
	}
	j = i;
	if (i == 0)
	{
		error(0,0,N_("ALERT: -m (--menu) option specified, but no menu items in config file"));
		return -1;
	}
	printf("\n%s: ",N_("enter your selection, or <return> to continue"));
	i = getchar() - 48; // effectively converts valid values to integers
	if ((i < 0) || (i >= j))
	{
		error(0,0,N_("menu selection received was out of bounds"));
		return -1;
	}
	printf("\n");
	return i;
}
