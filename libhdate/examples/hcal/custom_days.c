/** custom_days.c            http://libhdate.sourceforge.net
 * a collection of functions in support of both hcal.c and hdate.c
 * hcal.c  Hebrew calendar              (part of package libhdate)
 * hdate.c Hebrew date/times information(part of package libhdate)
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` custom_days.c -o custom_days
 *
 * Copyright:  2012-2013 (c) Boruch Baum
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

#include <hdate.h>		/// For hebrew date (gcc -I ../../src)
#include <support.h>	/// libhdate general macros, including for gettext
#include <stdlib.h>		/// For atoi, malloc
#include <error.h>		/// For error
#include <errno.h>		/// For errno
#include <time.h>		/// For time
#include <ctype.h>		/// for isprint
#include <fnmatch.h>	/// For fnmatch
#include <string.h>		/// For mempcpy
#include <getopt.h>		/// For optarg, optind
#include <stdio.h>		/// For printf, fopen, fclose, fprintf, snprintf. FILE
#include "local_functions.h" /// hcal,hdate common_functions


#define EXIT_CODE_BAD_PARMS	1

/// for custom days text string
#define MAX_STRING_SIZE_LONG  40
#define MAX_STRING_SIZE_SHORT 15


static const char* custom_days_file_text = N_("\
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
# How to handle events scheduled for Adar in a year with two Adars\n\
#    1 - Mark the event in Adar I\n\
#    2 - Mark the event in Adar II\n\
#ADAR_IN_LEAP_YEAR = 2\n\
ADAR_IN_LEAP_YEAR = 2\n\n\
# How to handle events scheduled for the 30th day of Adar I in a year\n\
# with only one Adar (which only has 29 days)\n\
#    0 - Do not mark the event in such a calendar year\n\
#    1 - Mark the event as 1 Nissan\n\
#   -1 - Mark the event as 29 Adar\n\
#ADAR_I_30 = 1\n\
ADAR_I_30 = -1\n\n\
# How to handle events scheduled for Adar II in a year without Adar II\n\
#    0 - Do not mark the event in such a calendar year\n\
#    1 - Mark the event in Nissan\n\
#   -1 - Mark the event in Adar\n\
#ADAR_II = -1\n\
ADAR_II = -1\n\n\
# NOTE: Because the data you provide in this file is processed\n\
#       sequentially, should you re-assign any of the above values\n\
#       amongst your list of custom_day definitions, that new value\n\
#       value assignment will take effect only for the subsequent\n\
#       custom_day definitions in this file. Thus, for example, you\n\
#       can have the rule \"CHESHVAN_30=1\" for one custom_day, and the\n\
#       rule  \"CHESHVAN_30=0\" for another custom_day\n\n\
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
#  7) Locale_language_description     - max 40 chars\n\
#  8) Locale_abbreviated_description  - max 15 chars\n\
#  9) Hebrew_language_description      - max 40 chars\n\
# 10) Hebrew_abbreviated_description   - max 15 chars\n#\n\
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
# Examples - Israeli national custom days\n\
# =======================================\n\
# code 35 = rabin day\n\
H, 5778,  2, 12, 0, 0, יום הזכרון ליצחק רבין, יום רבין, יום הזכרון ליצחק רבין,  Yitzhak Rabin memorial day, Rabin memorial day, -1, -2, 0, 0, 0, 0, 0\n\
# code 33 = family day\n\
H, 5708,  5, 30, 0, 0, יום המשפחה, יום המשפחה, Family day, Family day, 0, 0, 0, 0, 0, 0, 0\n\
# code 24 = Nisan 27 yom ha shoaa\n\
H, 5718,  7, 27, 0, 0, יום השואה, יום השואה, Yom HaShoah, Yom HaShoah, -1, 0, 1, 0, 0, 0, 0\n\
# code 26 = Iyyar 28 yom yerushalym 1968\n\
H, 5728,  8, 28, 0, 0, יום ירושלים, יום י-ם, Yom Yerushalayim, Yom Yerushalayim, 0, 0, 0, 0, 0, 0, 0\n\
# code 36 = Tammuz 29 Zhabotinsky day 2005\n\
H, 5765, 10, 29, 0, 0, יום הזכרון לזאב זבוטינסק, יום זבוטינסק, Zeev Zhabotinsky day, Zhabotinsky day, 0, 1, 0, 0, 0, 0, 0\n\
# code 17 = yom ha azmaot\n\
# code 25 = yom zikaron\n\
# complex logic for this pair, and it changes starting 2004\n\
# 1948-2004\n\
# iyyar 4, yom zikkaron, -2,  0, 0, 0, 0, 0, -1\n\
# iyyar 5, independence, -1, -2, 0, 0, 0, 0,  0\n\
# 2005-\n\
# iyyar 4, yom zikkaron, -1, -2, 1, 0, 0, 0, 0\n\
# iyyar 5, independence, -1, -2, 0, 1, 0, 0, 0\n\
#\n\
# DEBUG TESTS - REMOVE PRIOR TO RELEASE\n\
# =====================================\n\
H, 5700,  2,  6, 0, 0, יום הוקרת הפינגיונים עברי,   _, Penguin Appreciation Day Heb test,  _,  0,  0, 0, 0, 0, 0, 0, 0\n\
H, 5700,  2,  6, 0, 0, יום הוקרת הפינגיונים עברי,   _, Duplicate day Heb test,             _,  0,  0, 0, 0, 0, 0, 0, 0\n\
G, 1960,  1,  5, 0, 0, יום הוקרת הפינגיונים לועז,   _, Penguin Appreciation Day Greg test, _,  0,  0, 0, 0, 0, 0, 0, 0\n\
G, 1960,  1,  5, 0, 0, יום הוקרת הפינגיונים לועז,   _, Duplicate day Greg test,            _,  0,  0, 0, 0, 0, 0, 0, 0\n\
h, 5730,  4,  0, 2, 5, יום הוקרת הפינגיונים עברי ב, _, Penguin Appreciation Day heb test,  _,  0,  0, 0, 0, 0, 0, 0, 0\n\
g, 1980,  5,  0, 1, 2, יום הוקרת הפינגיונים לועז ב, _, Penguin Appreciation Day greg test, _,  0,  0, 0, 0, 0, 0, 0, 0\n\
H, 5750,  6,  8, 0, 0, פינגיונים מעדיפים יום ד,     _, Penguins Appreciate Wednesday test, _, -2, -3, 3, 0, 0, 0, 0, 0\n\
G, 2000,  7,  1, 0, 0, פינגיונים בעד סופש ארוך,     _, Penguins want long weekends test,   _,  0, -1, 1, 0, 0, 0, 0, 0\n\
");


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
					/// Values for text_short_form, text_hebrew_form
					/// are defined in libhdate (hdate_strings.c):
					/// #define HDATE STRING_SHORT   1
					/// #define HDATE_STRING_LONG    0
					/// #define HDATE_STRING_HEBREW  1
					/// #define HDATE_STRING_LOCAL   0
{
#define NUMBER_OF_CUSTOM_DAYS_FIELDS 14

	char*	input_string = NULL;
	size_t	input_str_len;

	int		line_count = 0;
	int		match_count;
	int		number_of_items = 0;

	int		leap_adj = 0;
	int 	key_hleap[2] = {-1,-1};			/// for Cheshvan_30, Kislev_30
	int 	key_february_29 =  1;
	int		key_adar_I_30   =  1;
	int 	key_adar_II     = -1;
	int 	key_adar_in_leap_year = 2;
	char	custom_day_type = '\0'; 		/// H, G, h, g
	int		custom_start_year = 0;
	int		custom_month = 0;
	int		custom_day_of_month = 0;
	int		custom_nth = 0 ;				/// 1 <= n <=  5
	int		custom_day_of_week = 0;

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

	char* print_ptr = NULL;
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
		if (print_ptr != NULL) {free(print_ptr); print_ptr = NULL;}
	}

	/************************************************************
	* sub-procedure for read_custom_days_file()
	************************************************************/
	void parse_option_line_for_keys()
	{
		char* input_key;
		int   input_value;
		const int	num_of_keys = 6;
		const char*	key_list[] = {
			"CHESHVAN_30",
			"KISLEV_30",
			"FEBRUARY_29",
			"ADAR_I_30",
			"ADAR_II",
			"ADAR_LEAP_YEAR" };
		int i;
		match_count = sscanf(input_string,"%m[A-Z_0-9] = %i",&input_key,&input_value);
		if (errno != 0) error(0,errno,"scan error at line %d of custom days file\n", line_count);
// DEBUG - 	printf("line number = %d, matches made = %d, key = %s, value = %s, string = %s",
//					line_count, match_count, input_key, input_value, input_string);
		if (match_count == 2)
		{
			for (i=0; i<num_of_keys; i++)
			{
				if (strcmp(input_key, key_list[i]) == 0)
				{
					switch(i)
					{
/** CHESHVAN_30 */	case  0: if ((input_value >= -1) && (input_value <= 1)) key_hleap[0] = input_value;
							 break;
/** KISLEV_30   */	case  1: if ((input_value >= -1) && (input_value <= 1)) key_hleap[1] = input_value;
							 break;
/** FEBRUARY_29 */	case  2: if ((input_value >= -1) && (input_value <= 1)) key_february_29 = input_value;
							 break;
/** ADAR_I_30   */	case  3: if ((input_value >= -1) && (input_value <= 1)) key_adar_I_30 = input_value;
							 break;
/** ADAR_II     */	case  4: if ((input_value >= -1) && (input_value <= 1)) key_adar_II = input_value;
							 break;
/** ADAR_IN_LEAP_YEAR */
					case  5: if ((input_value == 1) || (input_value == 2)) key_adar_in_leap_year = input_value;
							 break;
					} /// end switch
				} /// end comparison to key
			} /// end loop of all keys
		}
		if (match_count > 0 ) free(input_key);
	}



	/************************************************************
	* begin main section of procedure read_custom_days_file()
	************************************************************/
	*jdn_list_ptr = NULL;
	*string_list_ptr = NULL;

	/// set the size of each element in the text buffer array
	if   (text_short_form)	print_len = MAX_STRING_SIZE_SHORT;
	else 					print_len = MAX_STRING_SIZE_LONG;

	while ( getline(&input_string, &input_str_len, config_file) != -1)
	{
		free_my_mallocs();
		errno = 0;
		if (input_string[0] == '#') continue;
		const char* scan_spec[4] = {
			"%1[gGhHY], %u, %u, %u, %u, %u,  %m[^,] , %*m[^,] , %*m[^,] , %*m[^,] , %i, %i, %i, %i, %i, %i, %i",
			"%1[gGhHY], %u, %u, %u, %u, %u, %*m[^,] ,  %m[^,] , %*m[^,] , %*m[^,] , %i, %i, %i, %i, %i, %i, %i",
			"%1[gGhHY], %u, %u, %u, %u, %u, %*m[^,] , %*m[^,] ,  %m[^,] , %*m[^,] , %i, %i, %i, %i, %i, %i, %i",
			"%1[gGhHY], %u, %u, %u, %u, %u, %*m[^,] , %*m[^,] , %*m[^,] ,  %m[^,] , %i, %i, %i, %i, %i, %i, %i" };
		match_count = sscanf(input_string, scan_spec[ (abs(text_hebrew_form-1)*2) + text_short_form],
			&custom_day_type, &custom_start_year, &custom_month, &custom_day_of_month, &custom_nth, &custom_day_of_week,
			&print_ptr,	&adj[WHEN_SHISHI], &adj[WHEN_SHABBAT], &adj[WHEN_RISHON],
			&adj[WHEN_DAY_2], &adj[WHEN_DAY_3], &adj[WHEN_DAY_4], &adj[WHEN_DAY_5]);
		line_count++;
		if (errno)
		{
			error(0,errno,"scan error at line %d of custom days file\n", line_count);
			continue; // exit(0); // FIXME - why so fatal?
		}
		/* DEBUG  	if (match_count)
			printf("line number = %d, matches made = %d\n\tday_type = %c start_year = %d month = %d, day_of_month = %d\n\tnth = %d day_of_week = %d\n\t%s\n\t%s\n\t%s\n\t%s\n\tif_fri = %d if_sbt = %d if_sun = %d\n\n",
				line_count, match_count, custom_day_type, custom_start_year, custom_month, custom_day_of_month, custom_nth, custom_day_of_week,
				heb_long_desc, heb_abbr_desc, eng_long_desc, eng_abbr_desc,
				adj[WHEN_SHISHI], adj[WHEN_SHABBAT], adj[WHEN_RISHON]); */

		if (match_count != NUMBER_OF_CUSTOM_DAYS_FIELDS)
		{
			parse_option_line_for_keys();
			continue;
		}

		/************************************************************
		* At this point, we have successfully scanned/parsed a line
		* and are ready to begin sanity and bounds checking. The
		* switch() validates all of the parameters scanned/parsed,
		* converts the custom_day to the julian_day_number of the
		* relevant year, and applies any advancements/postponements.
		************************************************************/
		leap_adj = 0;
		switch ( custom_day_type )
		{
		case 'G':
			/// start by tackling a Feb 29 custom day, when user request is not a leap year
			if ((custom_month==2) && (custom_day_of_month==29)  &&
				(validate_hdate(CHECK_YEAR_PARM, custom_day_of_month, custom_month, custom_start_year, TRUE, &h1)) &&
				(validate_adjustments() )  &&
				(!validate_hdate(CHECK_DAY_PARM, custom_day_of_month, custom_month, h1.gd_year, TRUE, &h1))	)
			{
				if (!key_february_29) continue;
				if (key_february_29 == -1) leap_adj = -1;
			}
			else /// not a Feb 29 custom day
			{
				if ((!validate_hdate(CHECK_YEAR_PARM, custom_day_of_month, custom_month, custom_start_year, TRUE, &h1))  ||
					(!validate_hdate(CHECK_MONTH_PARM, custom_day_of_month, custom_month, h1.gd_year, TRUE, &h1)       ) ||
					(!validate_hdate(CHECK_DAY_PARM, custom_day_of_month, custom_month, h1.gd_year, TRUE, &h1)         ) ||
					(!validate_adjustments())      )
					continue;
			}
			hdate_set_gdate( &temp_h, custom_day_of_month + leap_adj, custom_month, h1.gd_year);
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
			/// start with absolute must validations for all cases
			if ((!validate_hdate(CHECK_YEAR_PARM, custom_day_of_month, custom_month, custom_start_year, TRUE, &h1)) ||
				(!validate_adjustments() )) continue;
			/// tackle Cheshvan 30 or Kislev 30 custom day, when user request hasn't those days
			if ((custom_day_of_month==30) && ((custom_month==2) || (custom_month==3))  &&
				(!validate_hdate(CHECK_DAY_PARM, custom_day_of_month, custom_month, h1.hd_year, TRUE, &h1))	)
			{
				if (!key_hleap[custom_month-2]) continue;
				if (key_hleap[custom_month-2] == -1) leap_adj = -1;
			}
			/// now deal with Adar II when user request isn't a leap year
			if ((custom_month==14) && (custom_day_of_month > 0) &&
				(custom_day_of_month < 30) && (h1.hd_size_of_year < 383) )
			{
				if (!key_adar_II) continue;
				if (key_adar_II == -1) custom_month = 6;
				else custom_month = 7; /// case key_adar_II = 1;
			}
			/// now deal with Adar I when user request isn't a leap year
			if ((custom_month==13) && (custom_day_of_month > 0) && (h1.hd_size_of_year < 383) )
			{
				if (custom_day_of_month > 30) continue;
				custom_month = 6;
				if (custom_month == 30)
				{
					if (!key_adar_I_30) continue;
					if (key_adar_I_30 == -1) leap_adj = -1;
				}
			}
			/// now deal with Adar in a leap year
			if ((custom_month==6) && (custom_day_of_month > 0) &&
				(custom_day_of_month < 31) && (h1.hd_size_of_year > 355) )
			{
				if (key_adar_in_leap_year == 1) custom_month = 13;
				else custom_month = 14; /// case key_adar_in_leap_year = 2;
			}
			else /// not a Cheshvan 30, Kislev 30, or Adar custom day for years that don't have them
			{
				if ((!validate_hdate(CHECK_MONTH_PARM, custom_day_of_month, custom_month, h1.hd_year, TRUE, &h1)       ) ||
					(!validate_hdate(CHECK_DAY_PARM, custom_day_of_month, custom_month, h1.hd_year, TRUE, &h1)         )  )
					continue;
			}
			hdate_set_hdate( &temp_h, custom_day_of_month + leap_adj, custom_month, h1.hd_year);
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
			new_jdn_list_ptr = realloc(*jdn_list_ptr, sizeof(int)*(number_of_items + LIST_INCREMENT));
			if (new_jdn_list_ptr == NULL)
			{
				/// realloc has failed. However, the original pointer and
				/// all its data are supposed to be fine. For now, let's
				/// silently abort reading the file because, in practice
				/// if we've really exhausted memory, we're about to
				/// seriouly crash anyway
				// TODO - consider issuing a warning / aborting
				free_my_mallocs(); /// the four string pointers
				return number_of_items;
			}
			else
			{
				*jdn_list_ptr = new_jdn_list_ptr;
				jdn_entry = ((int*) new_jdn_list_ptr) + number_of_items;
				new_jdn_list_ptr = NULL;
			}
		}

		/// store the custom_day's jdn
		*jdn_entry = jd;
		jdn_entry = jdn_entry + 1;

		/// manage 'text string' target buffer size
		if (!(number_of_items%LIST_INCREMENT))
		{
			string_list_buffer_size = string_list_buffer_size +
					sizeof(char) * ( (print_len+1) * LIST_INCREMENT);
			new_string_ptr = realloc(*string_list_ptr, string_list_buffer_size);
			if (new_string_ptr == NULL)
			{
				/// realloc has failed. However, the original pointer and
				/// all its data are supposed to be fine. For now, let's
				/// silently abort reading the file because, in practice
				/// if we've really exhausted memory, we're about to
				/// seriouly crash anyway
				// TODO - consider issuing a warning / aborting
				free_my_mallocs(); /// the four string pointers
				return number_of_items;
			}
			else
			{
				*string_list_ptr = new_string_ptr;
				// FIXME - should only have to do this once,
				//         upon the initial m(re)alloc.
				size_t* print_len_ptr = (size_t*) new_string_ptr;
				*print_len_ptr = print_len + 1;
				new_string_ptr = NULL;
			}
		}
		/// store the custom_day's text_string
		memcpy(*string_list_ptr + string_list_index, print_ptr, print_len);
		string_list_index = string_list_index + print_len;
		memset(*string_list_ptr + string_list_index, '\0', 1);
		string_list_index = string_list_index + 1;
		number_of_items++;
	}
	free_my_mallocs(); /// the four string pointers
	return number_of_items;
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
//  test_print_custom_days(number_of_items, *jdn_list_ptr, *string_list_ptr);
	return number_of_items;
}


