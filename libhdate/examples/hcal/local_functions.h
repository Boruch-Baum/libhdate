/** local_functions.h            http://libhdate.sourceforge.net
 * a collection of functions in support of both hcal.c and hdate.c
 * hcal.c  Hebrew calendar              (part of package libhdate)
 * hdate.c Hebrew date/times information(part of package libhdate)
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

#define EXIT_CODE_BAD_PARMS	1

#define SECONDS_PER_DAY 60*60*24

/// support for options parsing
#define MIN_CANDLES_MINUTES 15
#define DEFAULT_CANDLES_MINUTES 20
#define MAX_CANDLES_MINUTES 90
#define MIN_MOTZASH_MINUTES 20
#define MAX_MOTZASH_MINUTES 90



/// validate_hdate(...)
#define CHECK_DAY_PARM   1
#define CHECK_MONTH_PARM 2
#define CHECK_YEAR_PARM  3
int validate_hdate (const int parameter_to_check,
					const int day, const int month, const int year,
					int h_set, hdate_struct* h);

/// check_for_sunset
int check_for_sunset (hdate_struct * h, double lat, double lon, int timezone );

/// get_config_file(...)
FILE* get_config_file(	const char* config_dir_name,
						const char* config_file_name,
						const char* default_config_file_text,
						const int quiet_alerts );

/// revstr(...)
int revstr( char *source, const size_t source_len);

/// parse_coordinate(...)
int parse_coordinate( const int type_flag, char *input_string,
						double *coordinate, int *opt_found);

/// parse_timezone(...)
int parse_timezone_alpha(const char* search_string, char* result_str, int* tz, double* tz_lat, double* tz_lon);
int parse_timezone_numeric( char *input_string, int *tz);

/// parse_date(...)
int parse_date( const char* parm_a, const char* parm_b, const char* parm_c,
					 int* ret_year, int* ret_month, int* ret_day, const int parm_cnt );

void print_parm_error ( const char *parm_name );
void print_parm_missing_error ( const char *parm_name );

int menu_select( char* menu_list[], int max_menu_items );
int menu_item_parse(char* menuptr, size_t menu_len, int *menu_index,
					char** optptr, const char* short_options,
					const struct option *long_options, int *long_option_index,
					int *error_detected);

int process_location_parms( const int opt_latitude, const int opt_Longitude,
							 double *lat, double *lon,
							 int *tz, char* tz_name_ptr, const int quiet_alerts );

