/** local_functions.h            http://libhdate.sourceforge.net
 * a collection of functions in support of both hcal.c and hdate.c
 * hcal.c  Hebrew calendar              (part of package libhdate)
 * hdate.c Hebrew date/times information(part of package libhdate)
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

#define EXIT_CODE_BAD_PARMS	1

#define SECONDS_PER_DAY 60*60*24

/// support for options parsing
#define MIN_CANDLES_MINUTES 15
#define DEFAULT_CANDLES_MINUTES 20
#define MAX_CANDLES_MINUTES 90
#define MIN_MOTZASH_MINUTES 20
#define DEFAULT_MOTZASH_MINUTES 72
#define MAX_MOTZASH_MINUTES 90



/// validate_hdate(...)
#define CHECK_DAY_PARM   1
#define CHECK_MONTH_PARM 2
#define CHECK_YEAR_PARM  3
int validate_hdate (const int parameter_to_check,
					const int day, const int month, const int year,
					int h_set, hdate_struct* h);


int length_of_hmonth( const int month, const int year_type );

int length_of_gmonth( const unsigned int month, const unsigned int year );


/// check_for_sunset(...)
int check_for_sunset (hdate_struct * h, double lat, double lon, int timezone );

/// assemble config file path name(...)
char* assemnble_config_file_pathname (	const char* config_dir_name,
										const char* config_file_name,
										const int quiet_alerts );

/// get_config_file(...)
int get_config_file(	const char* config_dir_name,
						const char* config_file_name,
						const char* default_config_file_text,
						const int quiet_alerts,
						FILE** config_file);

///  greetings_to_version_18
void greetings_to_version_18();

/// revstr(...)
int revstr( char *source, const size_t source_len);

/// parse_coordinate(...)
int parse_coordinate( const int type_flag, char *input_string,
						double *coordinate);

/// parse_timezone(...)
int parse_timezone_alpha(const char* search_string, char** result_str, int* tz, double* tz_lat, double* tz_lon);
int parse_timezone_numeric( char *input_string, int *tz);

/// parse_date(...)
int parse_date( const char* parm_a, const char* parm_b, const char* parm_c,
					 int* ret_year, int* ret_month, int* ret_day, const int parm_cnt,
					 const int prefer_hebrew, const int base_year_h, const int base_year_g );

void parm_error ( const char *parm_name );
void parm_missing_error ( const char *parm_name );
void option_unknown_error ( const char *parm_name );
void config_file_create_error( int error_code, char* config_file_path );

int menu_select( char* menu_list[], int max_menu_items );
int menu_item_parse(char* menuptr, size_t menu_len, int *menu_index,
					char** optptr, const char* short_options,
					const struct option *long_options, int *long_option_index,
					int *error_detected);

void process_location_parms( double *lat, double *lon, double tz_lon,
							int *tz, char* tz_name_in, char** tz_name_out,
	 						const time_t start_time, const time_t end_time,
							int * num_dst_entries_ptr, void** returned_dst_data,
							const int quiet_alerts );

void get_epoch_time_range( time_t *retval_start, time_t *retval_end,
					const char * tz_string, const int tz,
					const int gyear0, const int gmonth0, const int gday0,
					const int gyear1, const int gmonth1, const int gday1
					);

int parse_epoch_value( const char* epoch_string, time_t* epoch_val, int* epoch_parm_received );
