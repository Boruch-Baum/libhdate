/** custom_days.c            http://libhdate.sourceforge.net
 * a collection of functions in support of both hcal.c and hdate.c
 * hcal.c  Hebrew calendar              (part of package libhdate)
 * hdate.c Hebrew date/times information(part of package libhdate)
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` custom_days.c -o custom_days
 *
 *  Copyright (C) 2012-2018 Boruch Baum  <boruch_baum@gmx.com>
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
//#define DEBUG

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
#include <sys/stat.h>	/// for mkdir
#include <sys/types.h>	/// for mkdir
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
# How to handle events scheduled for days 1 - 29 of Adar I in a year\n\
# with only one Adar (eg. Purim katan)\n\
#    0 - Do not mark the event in such a calendar year\n\
#    1 - Mark the event in Adar\n\
#ADAR_I = 0\n\
ADAR_I = 0\n\n\n\
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
# Each entry consists of 19 fields, comma-delimited, on a single\n\
# 'logical' line, meaning that if you insist on splitting an entry over\n\
# more than one physical line, use a '\\' to mark the continuation. All\n\
# fields are mandatory. The fields are:\n\
#  1) day_type - H for Hebrew calendar dates, G for gregorian calendar dates.\n\
#                Additionally, types h and g are available for marking an\n\
#                event to occur on the \'nth\' \'day_of_week\' of \'month\'.\n\
#  2) day_symbol - a single byte ascii printable character to be used by hcal\n\
#                to mark the day on its on-screen calendar and on footnotes.\n\
#                The available characters are: #%^&_-=\\;:?.,|[({]}) as well\n\
#                as the latin alpha characters a-z and A-Z. The following\n\
#                characters are reserved and may NOT be used: /+*~!@$\'\"`\n\
#  3) start_year - the first year for this commemoration.\n\
#  4) final_year - the final year for this commemoration, or zero if undefined.\n\
#                If the day_type field is G or g, both year fields must be\n\
#                gregorian year numbers; otherwise, a Hebrew year number is expected.\n\
#  5) month -    numbered 1 - 12 for Tishrei - Ellul; 13-14 for Adar I/II.\n\
#                numbered 1 - 12 for Jan - Dec,\n\
#  6) day_of_month - must be zero for day_type h or g\n#\n\
# The following two fields are only for day_type h or g, and must be zero\n\
# for day_type H and G.\n\
#  7) \'nth\'     - eg. nth Shabbat in Adar, or nth Tuesday in April.\n\
#  8) day_of_week - 7=Shabbat\n#\n\
# The following four fields are NOT to be enclosed in quotes. Each is comma\n\
# delimited, like all the other fields. Fields exceeding the maximum allowed\n\
# length will be truncated. All four fields are mandatory and must have at\n\
# least one printable character. Commas and semi-colons are prohibited in order\n\
# not to confuse hdate\'s tabular output (option -T --tabular), which is output\n\
# in comma-delimited format. Because it is possible to have more than one\n\
# holiday and custom_day on the same calendar date, hdate outputs tabular data\n\
# between holiday and custom_day fields using a semi-colon delimiter.\n\
#  9) Locale_language_description     - max 40 chars\n\
# 10) Locale_abbreviated_description  - max 15 chars\n\
# 11) Hebrew_language_description      - max 40 chars\n\
# 12) Hebrew_abbreviated_description   - max 15 chars\n#\n\
# The next three fields allow the custom day to be advanced or postponed\n\
# should that day occur on a friday, Shabbat, or sunday - in the cases of\n\
# day_types H or G. Values are the number of days to advance or postpone the\n\
# commemoration, in the range -9 to +9. For day_types h or g, the plan is that,\n\
# in a future release, these fields will allow the custom day to be advanced\n\
# or postponed should that day occur on an erev khag, khag, or motzei khag.\n\
# 13) if_Shishi    (if_erev_khag)\n\
# 14) if_Shabbat   (if_khag)\n\
# 15) if_Rishon    (if_motzei_khag)\n\n\
# The final four fields are advancement/postponement values for the remaining\n\
# days of the week  - for the cases of day_types H or G. Values are the number\n\
# of days to advance or postpone the commemoration, in the range -9 to +9. For\n\
# day_types h or g, the plan is that, in a future release, these fields will\n\
# allow the custom day to be advanced or postponed should that day occur on the\n\
# two days prior to erev khag or after motzei khag.\n\
# 16) if_day_2     (if_2_days_after_khag)\n\
# 17) if_day_3     (if_3_days_after_khag)\n\
# 18) if_day_4     (if_3_days_prior_khag)\n\
# 19) if_day_5     (if_2_days_prior_khag)\n\
#\n\
# Examples\n\
# ========\n\
# 1] One following minhag ashkenaz for selichot can mark the first night of\n\
#    selichot as follows:\n\
H, _, 3001, 0000, 1,  1, 0, 0, יום ראשון של סליחות, סליחות יום א, First night of selichot, Selichot I,-5,-6,-7,-8,-9,-3,-4\n\
# 2] To mark the Shabbat prior to a yahrtzeit (for various customs)\n\
H, _, 5741, 0000, 6, 27, 0, 0, שבת זכרון של פלוני, זכרון פלוני, Shabbat yahrtzeit for xxxx, Shabbat yahrtzeit,-6,-7,-1,-2,-3,-4,-5\n\
#\n\
# Examples - Special Shabbatot\n\
# ============================\n\
# These next few may in the future be hard-coded into libhdate ...\n\
H, _, 3001, 0000,  1,  1, 0, 0, שבת שובה, שבת שובה, Shabbat Shuva, Shabbat Shuva,           0, 7, -1,  5,  4,  0,  2\n\
H, _, 3001, 0000,  5, 15, 0, 0, שבת שירה, שבת שירה, Shabbat Shira, Shabbat Shira,           0, 0,  0, -2, -3, -4, -5\n\
H, _, 3001, 0000,  6,  1, 0, 0, שבת שקלים, שבת שקלים, Shabbat Shekalim, Shabbat Shekalim,  -6, 0,  0, -2, -3, -4,  0\n\
H, _, 3001, 0000,  6, 14, 0, 0, שבת זכור, זבת זכור, Shabbat Zachor, Shabbat Zachor,        -6, 0, -1,  0, -3,  0, -5\n\
H, _, 3001, 0000,  6, 14, 0, 0, שבת פרה, שבת פרה, Shabbat Parah, Shabbat Parah,             0, 0,  6,  0,  0,  0,  0\n\
H, _, 3001, 0000,  7,  1, 0, 0, שבת החודש, שבת החודש, Shabbat HaChodesh, Shabbat HaChodesh, 0, 0, -1,  0, -3,  0, -5\n\
H, _, 3001, 0000,  7, 15, 0, 0, שבת הגדול, שבת הגדול, Shabbat HaGadol, Shabbat HaGadol,     0,-7, -1,  0, -3,  0, -5\n\
H, _, 3001, 0000, 11,  9, 0, 0, שבת חזון, שבת חזון, Shabbat Chazon, Shabbat Chazon,         0, 0, -1,  0, -3,  0, -5\n\
H, _, 3001, 0000, 11,  9, 0, 0, שבת נחמו, שבת נחמו, Shabbat Nachamu, Shabbat Nachamu,       0, 7,  6,  0,  4,  0,  2\n\
\n\
# Examples - Other special days\n\
# =============================\n\
# These next few may in the future be hard-coded into libhdate ...\n\
H, _, 3001, 0000,  8, 14, 0, 0, פסח שני, פסח שני, Pesach Sheni, Pesach Sheni,               0, 0,  0,  0,  0,  0,  0\n\
H, _, 3001, 0000, 13, 14, 0, 0, פורים קטן, פורים קטן, Purim Katan, Purim Katan,             0, 0,  0,  0,  0,  0,  0\n");

static const char* custom_days_file_debug_text = N_("\n\
# DEBUG TESTS - REMOVE PRIOR TO RELEASE\n\
# =====================================\n\
H, :, 5700, 0000,  2,  6, 0, 0, יום הוקרת הפינגיונים עברי,   _, Penguin Appreciation Day Heb test,  _,  0,  0, 0, 0, 0, 0, 0\n\
H, :, 5700, 0000,  2,  6, 0, 0, יום הוקרת הפינגיונים עברי,   _, Duplicate day Heb test,             _,  0,  0, 0, 0, 0, 0, 0\n\
G, :, 1960, 0000,  1,  5, 0, 0, יום הוקרת הפינגיונים לועז,   _, Penguin Appreciation Day Greg test, _,  0,  0, 0, 0, 0, 0, 0\n\
G, :, 1960, 0000,  1,  5, 0, 0, יום הוקרת הפינגיונים לועז,   _, Duplicate day Greg test,            _,  0,  0, 0, 0, 0, 0, 0\n\
h, :, 5730, 0000,  4,  0, 2, 5, יום הוקרת הפינגיונים עברי ב, _, Penguin Appreciation Day heb test,  _,  0,  0, 0, 0, 0, 0, 0\n\
g, :, 1980, 0000,  5,  0, 1, 2, יום הוקרת הפינגיונים לועז ב, _, Penguin Appreciation Day greg test, _,  0,  0, 0, 0, 0, 0, 0\n\
H, :, 5750, 0000,  6,  8, 0, 0, פינגיונים מעדיפים יום ד,     _, Penguins Appreciate Wednesday test, _, -2, -3, 3, 0, 0, 0, 0\n\
G, :, 2000, 0000,  7,  1, 0, 0, פינגיונים בעד סופש ארוך,     _, Penguins want long weekends test,   _,  0, -1, 1, 0, 0, 0, 0\n");

static const char* custom_israeli_days_text_for_israel = N_("\
\n\
# Examples - Israeli national custom days\n\
# =======================================\n\
H, ^, 5758, 0000,  2, 12, 0, 0, יום הזכרון ליצחק רבין, יום רבין, Yitzhak Rabin memorial day, Rabin memorial day, -1, -2, 0, 0, 0, 0, 0\n\
H, ^, 5708, 0000,  5, 30, 0, 0, יום המשפחה, יום המשפחה, Family day, Family day, 0, 0, 0, 0, 0, 0, 0\n\
H, ^, 5718, 0000,  7, 27, 0, 0, יום השואה, יום השואה, Yom HaShoah, Yom HaShoah, -1, 0, 1, 0, 0, 0, 0\n\
H, ^, 5728, 0000,  8, 28, 0, 0, יום ירושלים, יום י-ם, Yom Yerushalayim, Yom Yerushalayim, 0, 0, 0, 0, 0, 0, 0\n\
H, ^, 5765, 0000, 10, 29, 0, 0, יום הזכרון לזאב זבוטינסק, יום זבוטינסק, Zeev Zhabotinsky day, Zhabotinsky day, 0, 1, 0, 0, 0, 0, 0\n\
H, ^, 5708, 5764,  8,  4, 0, 0, יום הזכרון, יום הזכרון, Yom HaZikaron, Yom HaZikaron, -2,  0, 0, 0, 0, 0, -1\n\
H, ^, 5708, 5764,  8,  5, 0, 0, יום העצמאות, יום העצמאות, Yom HaAtzma\'ut, Yom HaAtzma\'ut, -1, -2, 0, 0, 0, 0, 0\n\
H, ^, 5765, 0000,  8,  4, 0, 0, יום הזכרון, יום הזכרון, Yom HaZikaron, Yom HaZikaron, -2, 0, 1, 0, 0, 0, -1\n\
H, ^, 5765, 0000,  8,  5, 0, 0, יום העצמאות, יום העצמאות, Yom HaAtzma\'ut, Yom HaAtzma\'ut, -1, -2, 0, 1, 0, 0, 0\n\
");

static const char* custom_israeli_days_text_for_diaspora = N_("\
\n\
# Examples - Israeli national custom days\n\
# =======================================\n\
# H, ^, 5758, 0000,  2, 12, 0, 0, יום הזכרון ליצחק רבין, יום רבין, Yitzhak Rabin memorial day, Rabin memorial day, -1, -2, 0, 0, 0, 0, 0\n\
# H, ^, 5708, 0000,  5, 30, 0, 0, יום המשפחה, יום המשפחה, Family day, Family day, 0, 0, 0, 0, 0, 0, 0\n\
# H, ^, 5718, 0000,  7, 27, 0, 0, יום השואה, יום השואה, Yom HaShoah, Yom HaShoah, -1, 0, 1, 0, 0, 0, 0\n\
# H, ^, 5728, 0000,  8, 28, 0, 0, יום ירושלים, יום י-ם, Yom Yerushalayim, Yom Yerushalayim, 0, 0, 0, 0, 0, 0, 0\n\
# H, ^, 5765, 0000, 10, 29, 0, 0, יום הזכרון לזאב זבוטינסק, יום זבוטינסק, Zeev Zhabotinsky day, Zhabotinsky day, 0, 1, 0, 0, 0, 0, 0\n\
# H, ^, 5708, 5764,  8,  4, 0, 0, יום הזכרון, יום הזכרון, Yom HaZikaron, Yom HaZikaron, -2,  0, 0, 0, 0, 0, -1\n\
# H, ^, 5708, 5764,  8,  5, 0, 0, יום העצמאות, יום העצמאות, Yom HaAtzma\'ut, Yom HaAtzma\'ut, -1, -2, 0, 0, 0, 0, 0\n\
# H, ^, 5765, 0000,  8,  4, 0, 0, יום הזכרון, יום הזכרון, Yom HaZikaron, Yom HaZikaron, -2, 0, 1, 0, 0, 0, -1\n\
# H, ^, 5765, 0000,  8,  5, 0, 0, יום העצמאות, יום העצמאות, Yom HaAtzma\'ut, Yom HaAtzma\'ut, -1, -2, 0, 1, 0, 0, 0\n\
#\n");

/*****************************************************************
* get custom day text pointer
*
* returns the pointer to the nth string in string_list
*****************************************************************/
char* get_custom_day_text_ptr(const int index, char* string_list_ptr)
{
	return string_list_ptr + sizeof(size_t) + ( sizeof(char)*( ((size_t)*string_list_ptr + 2) * index) + sizeof(char) );
	/// the "+ 1" is because the first char is the 'custom day symbol'
}


/*****************************************************************
* get custom day symbol pointer
*
* returns the pointer to the single character symbol for the nth
* string in string_list
*****************************************************************/
char* get_custom_day_symbol_ptr(const int index, char* string_list_ptr)
{
	return string_list_ptr + sizeof(size_t) + ( sizeof(char)*( ((size_t)*string_list_ptr + 2) * index) );
	/// the "+ 1" is because the first char is the 'custom day symbol'
}


/************************************************************
* test_print_custom_days
*
* This is a debug routine
************************************************************/
void test_print_custom_days(int custom_days_cnt, int* jdn_list_ptr, char* string_list_ptr)
{
	size_t print_len;
	int i, day, month, year;


	if (!custom_days_cnt) return;
	print_len = (size_t) *string_list_ptr;
	printf("       entered test print routine\n       number of custom_days = %d\n       print_len size = %ld\n",
			custom_days_cnt, print_len);
	string_list_ptr = string_list_ptr + sizeof(size_t);
	for (i=0; i<custom_days_cnt; i++)
	{
		hdate_jd_to_gdate ( *jdn_list_ptr, &day, &month, &year);
		printf("       %d: %d %4d-%02d-%02d, %s\n",i,*jdn_list_ptr,year,month,day, string_list_ptr);
		jdn_list_ptr = jdn_list_ptr + 1;
		string_list_ptr = string_list_ptr + (sizeof(char)*print_len) + sizeof(char)*2;
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
//  TODO - advancements/postponements for day_types h/g when erev khag, khag, or motzei khag
int read_custom_days_file(
			FILE* config_file,
			int** jdn_list_ptr, char** string_list_ptr,
			const int d_todo, const int m_todo, const int y_todo,
			const char calendar_type,
			/*****************************************************
			*  calendar_type should always be consistent with
			*  d_todo, m_todo, y_todo, ie. there should be no
			*  need to calculate whether any is G or H
			*****************************************************/
			hdate_struct range_start,
			const int text_short_form, const int text_hebrew_form)
					/// Values for text_short_form, text_hebrew_form
					/// are defined in libhdate (hdate_strings.c):
					/// #define HDATE STRING_SHORT   1
					/// #define HDATE_STRING_LONG    0
					/// #define HDATE_STRING_HEBREW  1
					/// #define HDATE_STRING_LOCAL   0
{
	/// There are 19 mandatory fields on each line of the custom days fiile,
	/// but since we will only use one of the four text description options,
	/// the following is 19 - 3.
	#define NUMBER_OF_CUSTOM_DAYS_FIELDS 16

	char*	input_string = NULL;
	size_t	input_str_len = 0;

	int		bytes_read = 0;
	int		line_count = 0;
	int		match_count = 0;
	int		key_match_count = 0;
	int		number_of_items = 0;

	int		leap_adj = 0;
	int		year_bound_adj = 0;
	char	custom_day_type = '\0'; /// H, G, h, g
	#define CUSTOM_SYMBOL_LEN 1
	char	custom_symbol[2] = {'\0','\0'}; /// single char, and string delimiter
	int		custom_start_year = 0;
	int		custom_final_year = 0;
	int		custom_month = 0;
	int		custom_day_of_month = 0;
	int		custom_nth = 0 ;		/// 1 <= n <=  5
	int		custom_day_of_week = 0;

	char*	input_key;
	int 	input_value;
	int		key_name_found = FALSE;
	int		key_value_found = FALSE;
	const int	num_of_keys = 7;
	const char*	key_list[] = {
		"CHESHVAN_30",
		"KISLEV_30",
		"FEBRUARY_29",
		"ADAR_I",
		"ADAR_I_30",
		"ADAR_II",
		"ADAR_IN_LEAP_YEAR" };
	int 	key_hleap[2] = {1,1};	/// for Cheshvan_30, Kislev_30
	int 	key_february_29 =  1;
	int		key_adar_I_30   =  1;
	int		key_adar_I		=  0;
	int 	key_adar_II     = -1;
	int 	key_adar_in_leap_year = 2;
	int		i;



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
	int temp_jd;
	hdate_struct custom_day_h;
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
	************************************************************/
	void free_my_mallocs()
	{
		if (print_ptr != NULL) {free(print_ptr); print_ptr = NULL;}
	}


	/************************************************************
	* begin main section of procedure read_custom_days_file()
	************************************************************/
	*jdn_list_ptr = NULL;
	*string_list_ptr = NULL;

	/// set the size of each element in the text buffer array
	if   (text_short_form)	print_len = MAX_STRING_SIZE_SHORT;
	else 					print_len = MAX_STRING_SIZE_LONG;

	while ( (bytes_read = getline(&input_string, &input_str_len, config_file)) != -1)
	{
		line_count++;
		errno = 0;
		if ( (input_string[0] == '#') || (input_string[0] == '\n') || (bytes_read == 0) ) continue;
		free_my_mallocs();
		const char* scan_spec[4] = {
			"%1[gGhHY], %1[][({})a-zA-Z#%^&_=\\;:?.,|-], %u, %u, %u, %u, %u, %u,  %m[^,] , %*m[^,] , %*m[^,] , %*m[^,] , %d, %d, %d, %d, %d, %d, %d",
			"%1[gGhHY], %1[][({})a-zA-Z#%^&_=\\;:?.,|-], %u, %u, %u, %u, %u, %u, %*m[^,] ,  %m[^,] , %*m[^,] , %*m[^,] , %d, %d, %d, %d, %d, %d, %d",
			"%1[gGhHY], %1[][({})a-zA-Z#%^&_=\\;:?.,|-], %u, %u, %u, %u, %u, %u, %*m[^,] , %*m[^,] ,  %m[^,] , %*m[^,] , %d, %d, %d, %d, %d, %d, %d",
			"%1[gGhHY], %1[][({})a-zA-Z#%^&_=\\;:?.,|-], %u, %u, %u, %u, %u, %u, %*m[^,] , %*m[^,] , %*m[^,] ,  %m[^,] , %d, %d, %d, %d, %d, %d, %d" };
		match_count = sscanf(input_string, scan_spec[ (abs(text_hebrew_form-1)*2) + text_short_form],
			&custom_day_type, &custom_symbol, &custom_start_year, &custom_final_year, &custom_month, &custom_day_of_month, &custom_nth, &custom_day_of_week,
			&print_ptr,	&adj[WHEN_SHISHI], &adj[WHEN_SHABBAT], &adj[WHEN_RISHON],
			&adj[WHEN_DAY_2], &adj[WHEN_DAY_3], &adj[WHEN_DAY_4], &adj[WHEN_DAY_5]);
		if (errno)
		{
			// test this error message
			error(0,errno,"scan error (a)%d at line %d of custom days file, field %d\n", errno, line_count, (match_count<10 ? match_count+1 : match_count+4) );
			continue;
		}
		if (match_count != NUMBER_OF_CUSTOM_DAYS_FIELDS)
		{
			key_name_found = FALSE;
			key_value_found = FALSE;
			key_match_count = sscanf(input_string,"%m[A-Z_0-9] = %i",&input_key,&input_value);
			if (errno != 0) error(0,errno,"scan error (b)%d at line %d of custom days file, field %d\n", errno, line_count, match_count+1 );
			if (key_match_count)
			{
				for (i=0; (i<num_of_keys) && (!key_name_found) && (!key_value_found) ; i++)
				{
					if (strcmp(input_key, key_list[i]) == 0)
					{
						key_name_found = TRUE;
						if (key_match_count == 2) switch(i)
						{
						/** CHESHVAN_30 */
						case  0: if ((input_value >= -1) && (input_value <= 1))
								 {
									 key_hleap[0] = input_value;
									 key_value_found = TRUE;
								 }
								 break;
						/** KISLEV_30   */
						case  1: if ((input_value >= -1) && (input_value <= 1))
								 {
									 key_hleap[1] = input_value;
									 key_value_found = TRUE;
								 }
								 break;
						/** FEBRUARY_29 */
						case  2: if ((input_value >= -1) && (input_value <= 1))
								 {
									 key_february_29 = input_value;
									 key_value_found = TRUE;
								 }
								 break;
						/** ADAR_I   */
						case  3: if ((input_value == 1) || (input_value == 0))
								 {
									 key_adar_I = input_value;
									 key_value_found = TRUE;
								 }
								 break;
						/** ADAR_I_30   */
						case  4: if ((input_value >= -1) && (input_value <= 1))
								 {
									 key_adar_I_30 = input_value;
									 key_value_found = TRUE;
								 }
								 break;
						/** ADAR_II     */
						case  5: if ((input_value >= -1) && (input_value <= 1))
								 {
									 key_adar_II = input_value;
									 key_value_found = TRUE;
								 }
								 break;
						/** ADAR_IN_LEAP_YEAR */
						case  6: if ((input_value == 1) || (input_value == 2))
								 {
									 key_adar_in_leap_year = input_value;
									 key_value_found = TRUE;
								 }
								 break;
						} /// end switch
					} /// end comparison to key
				} /// end loop of all keys
			}
			if (key_match_count) free(input_key);
			if ( !key_value_found )
			{
				if (key_name_found == TRUE) match_count = 1;
				error(0,errno,"scan error (c)%d at line %d of custom days file, field %d\n", errno, line_count, (match_count<10 ? match_count+1 : match_count+4) );
			}
			continue;
		}

		/************************************************************
		* At this point, we have successfully scanned/parsed a line
		* as a basically valid adjustment line of 19 ( minus 3) fields
		* and are ready to begin sanity and bounds checking. The
		* sscanf call above has already insured that the values for
		* adjustments are signed integers, and for years are
		* unsigned integers.
		************************************************************/
		int i;
		for ( i = 0; i < 7; i++ )
		{
			if ( ( adj[i] < -9 ) || ( adj[i] > 9 ) )
			{
				error(0,errno,"error at line %d of custom days file, adjustment field %d invalid: %d\n", line_count, i+1, adj[i]);
				continue;
			}
		}

		switch ( custom_day_type )
		{
		case 'G':
		case 'g':
			if ( (custom_start_year < HDATE_GREG_YR_LOWER_BOUND) ||
				 (custom_start_year > HDATE_GREG_YR_UPPER_BOUND) ||
				 ( (custom_final_year) &&
				   ( (custom_start_year > custom_final_year)   ||
					 (custom_final_year > HDATE_GREG_YR_UPPER_BOUND) ||
					 (custom_final_year < HDATE_GREG_YR_LOWER_BOUND)
				 ) ) )
			{
				error(0,errno,"parameter error (a)%d at line %d of custom days file, start year: %d. end year: %d\n", errno, line_count, custom_start_year, custom_final_year);
				continue;
			}
			break;
		case 'H':
		case 'h':
			if ( (custom_start_year < HDATE_HEB_YR_LOWER_BOUND) ||
				 (custom_start_year > HDATE_HEB_YR_UPPER_BOUND) ||
				 ( (custom_final_year) &&
				   ( (custom_start_year > custom_final_year)   ||
					 (custom_final_year > HDATE_HEB_YR_UPPER_BOUND) ||
					 (custom_final_year < HDATE_HEB_YR_LOWER_BOUND)
				) ) )
			{
				error(0,errno,"parameter error (a)%d at line %d of custom days file, start year: %d. end year: %d\n", errno, line_count, custom_start_year, custom_final_year);
				continue;
			}
			break;
		} /// end switch ( custom_day_type )


		/************************************************************
		* Perform the relatively easy checks to eliminate ALMOST all
		* the cases of custom days being out of bounds.
		************************************************************/
		//*
		//* The more difficult cases are those in which a custom day's
		//* 'adjustments' will make it cross over a year boundary.
		/************************************************************/
		/// The bit operation on custom_day_type
		/// is to make g==G and h==H
		if (calendar_type == (custom_day_type & 0xdf) )
		{
			if ( (y_todo < (custom_start_year - 1) ) ||
				 ( (y_todo < custom_start_year) &&
				   ( (m_todo != 12) || (custom_month != 1) || (custom_day_of_month > 9) ||
				     ( ( (calendar_type == 'G') && (d_todo < 22) ) ||
					   ( (calendar_type == 'H') && (d_todo < 20) ) ) ) ) ) continue;
			if ( (custom_final_year != 0) &&
				 ( (y_todo > (custom_final_year + 1) ) ||
				   ( (y_todo > custom_final_year) &&
				     ( (m_todo != 1) || (custom_month != 12) || (d_todo > 9) ||
				       ( ( (calendar_type == 'G') && (custom_day_of_month < 22) ) ||
					     ( (calendar_type == 'H') && (custom_day_of_month < 20) ) ) ) ) ) ) continue;
		}
		else
		{
			if (calendar_type == 'G') /// && (custom_day_type == 'H' or 'h')
			{
				if ( (y_todo < (custom_start_year - 3761) ) ||
					 ( (y_todo == (custom_start_year - 3761) ) && /// aug-dec or tish-shvat may be good
					   ( (m_todo < 8 /** august **/) || (custom_month > 5 /** shvat **/ ) ) ) ) continue;
				if ( ( custom_final_year != 0 ) &&
					 ( (y_todo > (custom_final_year - 3760) ) ||
					   ( (y_todo == (custom_final_year - 3760) ) && /// jan-oct or kislev-elul may be good
					     ( (m_todo > 10 /** october **/) || (custom_month < 3 /** kislev **/ ) ) ) ) ) continue;
			}
			else /// (calendar_type == 'H') && (custom_day_type == 'G' or 'g')
			{
				if ( (y_todo < (custom_start_year + 3760) ) ||
					 ( (y_todo == (custom_start_year + 3760) ) && /// jan-oct or kislev-elul may be good
					 ( (m_todo < 3 /** kislev **/) || (custom_month > 10 /** octoner **/ ) ) ) ) continue;

				if ( ( custom_final_year != 0 ) &&
					 ( (y_todo > (custom_final_year + 3761) ) ||
					   ( (y_todo == (custom_final_year + 3761) ) && /// aug-dec or tish-shvat may be good
					   ( (m_todo > 5 /** Shvat **/) || (custom_month < 8 /** august **/ ) ) ) ) ) continue;
			}
		}
		// TODO - d_to_do, m_to_do, y_to_do may still be out of bounds. We
		// must take into account that adjustments may be +/-9 days which could
		// bump the year and also Heb year switching in Autumn

		/************************************************************
		* 1] Perform sanity and bounds checking of the remaining
		*    parameters scanned/parsed
		* 2] convert the custom_day to the julian_day_number for the
		*    year the user requested, and
		* 3] apply any advancements/postponements.
		************************************************************/
		leap_adj = 0;
		switch ( custom_day_type )
		{
		case 'G':
			/// start by tackling a Feb 29 custom day, when user request is not a leap year
			if ((custom_month==2) && (custom_day_of_month==29)  &&
				(!validate_hdate(CHECK_DAY_PARM, custom_day_of_month, custom_month, range_start.gd_year, TRUE, &range_start))	)
			{
				if (!key_february_29) continue;
				if (key_february_29 == -1) leap_adj = -1;
			}
			else /// not a Feb 29 custom day
			{
				if ((!validate_hdate(CHECK_MONTH_PARM, custom_day_of_month, custom_month, range_start.gd_year, TRUE, &range_start)       ) ||
					(!validate_hdate(CHECK_DAY_PARM, custom_day_of_month, custom_month, range_start.gd_year, TRUE, &range_start)         )  )
					continue;
			}
			/// deal with cases where an adjustment causes a date to cross between December and January
			year_bound_adj = 0;
			if (!d_todo)
			{
				if      ((m_todo == 12) && (custom_month ==  1)) year_bound_adj =  1;
				else if ((m_todo ==  1) && (custom_month == 12)) year_bound_adj = -1;
			}

			hdate_set_gdate( &custom_day_h, custom_day_of_month + leap_adj, custom_month, range_start.gd_year + year_bound_adj);
			if (adj[custom_day_h.hd_dw-1])
			{
				temp_jd = custom_day_h.hd_jd + adj[custom_day_h.hd_dw-1];
				hdate_set_jd(&custom_day_h, temp_jd);
			}
			if ( (!d_todo) && (y_todo > HDATE_HEB_YR_LOWER_BOUND) && (y_todo != custom_day_h.hd_year) )
			{
				hdate_set_gdate( &custom_day_h, custom_day_of_month, custom_month, range_start.gd_year+1);
				if (adj[custom_day_h.hd_dw-1])
				{
					temp_jd = custom_day_h.hd_jd + adj[custom_day_h.hd_dw-1];
					hdate_set_jd(&custom_day_h, temp_jd);
				}
			}
			break;
		case 'H':
			/// tackle Cheshvan 30 or Kislev 30 custom day, when user request hasn't those days
			if ((custom_day_of_month==30) && ((custom_month==2) || (custom_month==3))  &&
				(!validate_hdate(CHECK_DAY_PARM, custom_day_of_month, custom_month, range_start.hd_year, TRUE, &range_start))	)
			{
				if (!key_hleap[custom_month-2]) continue;
				else if (key_hleap[custom_month-2] == -1) leap_adj = -1;
			}
			/// now deal with Adar II when user request isn't a leap year
			else if ((custom_month==14) && (custom_day_of_month > 0) &&
				(custom_day_of_month < 30) && (range_start.hd_size_of_year < 383) )
			{
				if (!key_adar_II) continue;
				else if (key_adar_II == -1) custom_month = 6;
				else if (key_adar_II == 1) custom_month = 7;
			}
			/// now deal with Adar I when user request isn't a leap year
			else if ((custom_month==13) && (custom_day_of_month > 0) && (range_start.hd_size_of_year < 383) )
			{
				if (custom_day_of_month > 30) continue; // ?? This should never be possible !
				custom_month = 6;
				if (custom_day_of_month == 30)
				{
					if (!key_adar_I_30) continue;
					else if (key_adar_I_30 == -1) leap_adj = -1;
				}
				else ///  (custom_day_of_month < 30)
				{
					if (!key_adar_I) continue;
				}
			}
			/// now deal with Adar in a leap year
			else if ((custom_month==6) && (custom_day_of_month > 0) &&
				(custom_day_of_month < 31) && (range_start.hd_size_of_year > 355) )
			{
				if (key_adar_in_leap_year == 1) custom_month = 13;
				else if (key_adar_in_leap_year == 2) custom_month = 14;
				else continue;
			}
			else /// not a Cheshvan 30, Kislev 30, or Adar custom day for years that don't have them
			{
				if ((!validate_hdate(CHECK_MONTH_PARM, custom_day_of_month, custom_month, range_start.hd_year, TRUE, &range_start)       ) ||
					(!validate_hdate(CHECK_DAY_PARM, custom_day_of_month, custom_month, range_start.hd_year, TRUE, &range_start)         )  )
					continue;
			}
			/// deal with cases where an adjustment causes a date to cross between Ellul and Tishrei
			year_bound_adj = 0;
			if (!d_todo)
			{
				if      ((m_todo == 12) && (custom_month ==  1)) year_bound_adj =  1;
				else if ((m_todo ==  1) && (custom_month == 12)) year_bound_adj = -1;
			}

			hdate_set_hdate( &custom_day_h, custom_day_of_month + leap_adj, custom_month, range_start.hd_year + year_bound_adj);
			if (adj[custom_day_h.hd_dw-1])
			{
				temp_jd = custom_day_h.hd_jd + adj[custom_day_h.hd_dw-1];
				hdate_set_jd(&custom_day_h, temp_jd);
			}
			if ( (!d_todo) && (y_todo <= HDATE_HEB_YR_LOWER_BOUND) && (y_todo != custom_day_h.gd_year) )
			{
				hdate_set_hdate( &custom_day_h, custom_day_of_month, custom_month, range_start.hd_year+1); //?
				if (adj[custom_day_h.hd_dw-1])
				{
					temp_jd = custom_day_h.hd_jd + adj[custom_day_h.hd_dw-1];
					hdate_set_jd(&custom_day_h, temp_jd);
				}
			}
			break;
		case 'g':
			if ((!validate_hdate(CHECK_MONTH_PARM, 1, custom_month, range_start.gd_year, TRUE, &range_start)                         ) ||
				(custom_nth < 1) || (custom_nth > 5) ||
				(custom_day_of_week < 1) || (custom_day_of_week > 7) )
				continue;
			hdate_set_gdate( &custom_day_h, 1, custom_month, range_start.gd_year);
			temp_jd = custom_day_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - custom_day_h.hd_dw);
			hdate_set_jd(&custom_day_h, temp_jd);
			if ((custom_nth==5) && ( custom_month != custom_day_h.gd_mon))
			{
				if ( (!d_todo) && (y_todo > HDATE_HEB_YR_LOWER_BOUND) && (y_todo != custom_day_h.hd_year) )
				{
					hdate_set_gdate( &custom_day_h, 1, custom_month, range_start.gd_year+1);
					temp_jd = custom_day_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - custom_day_h.hd_dw);
					hdate_set_jd(&custom_day_h, temp_jd);
					if ((custom_nth==5) && ( custom_month != custom_day_h.gd_mon)) continue;
				}
				else continue;
			}
			else if ( (!d_todo) && (y_todo > HDATE_HEB_YR_LOWER_BOUND) && (y_todo != custom_day_h.hd_year) )
			{
				hdate_set_gdate( &custom_day_h, 1, custom_month, range_start.gd_year+1);
				temp_jd = custom_day_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - custom_day_h.hd_dw);
				hdate_set_jd(&custom_day_h, temp_jd);
				if ((custom_nth==5) && ( custom_month != custom_day_h.gd_mon)) continue;
			}
			// perform adjustments and verify (again!?!)
			break;
		case 'h':
			if ((!validate_hdate(CHECK_MONTH_PARM, 1, custom_month, range_start.hd_year, TRUE, &range_start)                          )||
				(custom_nth < 1) || (custom_nth > 5) ||
				(custom_day_of_week < 1) || (custom_day_of_week > 7) )
				continue;
			hdate_set_hdate( &custom_day_h, 1, custom_month, range_start.hd_year);
			temp_jd = custom_day_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - custom_day_h.hd_dw);
			hdate_set_jd(&custom_day_h, temp_jd);
			if ((custom_nth==5) && ( custom_month != custom_day_h.hd_mon))
			{
				if ( (!d_todo) && (y_todo <= HDATE_HEB_YR_LOWER_BOUND) && (y_todo != custom_day_h.gd_year) )
				{
					hdate_set_hdate( &custom_day_h, 1, custom_month, range_start.hd_year+1);
					temp_jd = custom_day_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - custom_day_h.hd_dw);
					hdate_set_jd(&custom_day_h, temp_jd);
					if ((custom_nth==5) && ( custom_month != custom_day_h.hd_mon)) continue;
				}
				else continue;
			}
			else if ( (!d_todo) && (y_todo <= HDATE_HEB_YR_LOWER_BOUND) && (y_todo != custom_day_h.gd_year) )
			{
				hdate_set_hdate( &custom_day_h, 1, custom_month, range_start.hd_year+1);
				temp_jd = custom_day_h.hd_jd + ((custom_nth-1) * 7) + (custom_day_of_week - custom_day_h.hd_dw);
				hdate_set_jd(&custom_day_h, temp_jd);
				if ((custom_nth==5) && ( custom_month != custom_day_h.hd_mon)) continue;
			}
			// perform adjustments and verify (again!?!)
			break;
		default:
			error(0,0,"%s: %c %s %d",N_("internal error: illegal custom day type"),custom_day_type, N_("at line number"), line_count);
			break;
		} /// end switch ( custom_day_type )

		/************************************************************
		* At this point, we have computed the jd of the occurrence
		* of the parsed line's custom_day for the year in question.
		* Now we range check it against the interval being processed
		* - either a single day, single month, or single year. This
		* is non-trivial because the interval and the parse line may
		* have been denominated in different calendars.
		* We are depending upon range_start, a parameter passed to us, being
		* the correct hdate_struct for the first day of the interval.
		* We compute the hdate_struct for our jd (if hadn't already
		* been done in the switch for sanity and bounds checking) and
		* compare its values to the interval parameters passed to us
		************************************************************/
		if (d_todo)
		{
			/// interval is a single day
			if (custom_day_h.hd_jd != range_start.hd_jd) continue;
		}
		else
		{
			/// our custom_day is too early
			if (custom_day_h.hd_jd < range_start.hd_jd) continue;

			if (m_todo)
			{
				/// interval is for an entire month
				if (y_todo > HDATE_HEB_YR_LOWER_BOUND)
				{
					if ( m_todo != custom_day_h.hd_mon) continue;
				}
				else
				{
					if ( m_todo != custom_day_h.gd_mon) continue;
				}
			}
			else
			{
				/// (!m) interval is for an entire year
				if (y_todo > HDATE_HEB_YR_LOWER_BOUND)
				{
					if (y_todo != custom_day_h.hd_year) continue;
				}
				else if ( y_todo != custom_day_h.gd_year) continue;
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
				free_my_mallocs();
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
		*jdn_entry = custom_day_h.hd_jd;
		jdn_entry = jdn_entry + 1;

		/// manage 'text string' target buffer size
		if (!(number_of_items%LIST_INCREMENT))
		{
			string_list_buffer_size = string_list_buffer_size +
					sizeof(char) * ( (print_len+CUSTOM_SYMBOL_LEN+1) * LIST_INCREMENT);
			new_string_ptr = realloc(*string_list_ptr, string_list_buffer_size);
			if (new_string_ptr == NULL)
			{
				/// realloc has failed. However, the original pointer and
				/// all its data are supposed to be fine. For now, let's
				/// silently abort reading the file because, in practice
				/// if we've really exhausted memory, we're about to
				/// seriouly crash anyway
				// TODO - consider issuing a warning / aborting
				free_my_mallocs();
				return number_of_items;
			}
			else
			{
				*string_list_ptr = new_string_ptr;
				// FIXME - should only have to do this once,
				//         upon the initial m(re)alloc.
				size_t* print_len_ptr = (size_t*) new_string_ptr;
				*print_len_ptr = print_len;
				new_string_ptr = NULL;
			}
		}
		/// store the custom_day's text_string
		memset(*string_list_ptr + string_list_index, custom_symbol[0], sizeof(char) );
		string_list_index = string_list_index + sizeof(char);
		memcpy(*string_list_ptr + string_list_index, print_ptr, (sizeof(char) * print_len) );
		string_list_index = string_list_index + ( sizeof(char) * print_len );
		memset(*string_list_ptr + string_list_index, '\0', sizeof(char));
		string_list_index = string_list_index + sizeof(char);
		number_of_items++;
	}
	free_my_mallocs();

	// debug routine
	// test_print_custom_days(number_of_items, *jdn_list_ptr, *string_list_ptr);
	return number_of_items;
}


/****************************************************
* read, parse and filter custom_days file
****************************************************/
int get_custom_days_file( const char* config_dir,
						  const char* config_filename,
						  const char* tz_name_str,
						  const int quiet_alerts,
						  FILE** custom_file )
{
	// TODO - create an option for both hcal/hdate to allow a custom path for this file
	//        pass the default FULL path or the custom FULL path to read_custom_days_file
	//        and let THAT function open and close the file
	*custom_file = NULL;
	char *custom_file_path = NULL;
	char *last_slash_location = NULL;
	int bytes_written = -1;

	custom_file_path = assemnble_config_file_pathname (
									config_dir, config_filename,
									quiet_alerts );
	if (custom_file_path == NULL) return FALSE;
	*custom_file = fopen(custom_file_path, "r");
	if (*custom_file == NULL)
	{
		if (errno != ENOENT) { free(custom_file_path); return FALSE; };
		last_slash_location = strrchr(custom_file_path, '/');
		if (last_slash_location ==  NULL) { free(custom_file_path); return FALSE; };
		*last_slash_location = '\0';
		if ((mkdir(custom_file_path, (mode_t) 0700) != 0) && (errno != EEXIST)) { free(custom_file_path); return FALSE; };
		*last_slash_location = '/';
		greetings_to_version_18();
		if (!quiet_alerts) printf("%s\n", N_("attempting to create a config file ..."));
		*custom_file = fopen(custom_file_path, "a+");
		if (*custom_file != NULL)
			bytes_written = fprintf(*custom_file, "%s", custom_days_file_text);
		if (bytes_written > 0)
		{
			if ((tz_name_str == NULL) || (strcmp(tz_name_str, "Asia/Jerusalem") != 0))
				 bytes_written = fprintf(*custom_file, "%s", custom_israeli_days_text_for_diaspora);
			else bytes_written = fprintf(*custom_file, "%s", custom_israeli_days_text_for_israel);
#ifdef DEBUG
  		if (bytes_written > 0)
				bytes_written = fprintf(*custom_file, "%s", custom_days_file_debug_text);
#endif
		}
		if (bytes_written <= 0)
		{
			if (!quiet_alerts) config_file_create_error(errno, custom_file_path);
			{ free(custom_file_path); return FALSE; };
		}
		if (!quiet_alerts) printf("%s: %s\n", N_("succeeded creating config file"), custom_file_path);
		if ( fseek(*custom_file, 0, SEEK_SET) != 0 ) { free(custom_file_path); return FALSE; };
	}
	free(custom_file_path);
	return TRUE;
}
