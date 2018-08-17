/** custom_days.h            http://libhdate.sourceforge.net
 * a collection of functions in support of both hcal.c and hdate.c
 * hcal.c  Hebrew calendar              (part of package libhdate)
 * hdate.c Hebrew date/times information(part of package libhdate)
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` custom_days.c -o custom_days
 *
 *  Copyright (C) 2011-2018 Boruch Baum  <boruch_baum@gmx.com>
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

extern char* get_custom_day_text_ptr(const int index, char* string_list_ptr);

char* get_custom_day_symbol_ptr(const int index, char* string_list_ptr);

extern int get_custom_days_file( const char* config_dir,
								 const char* config_filename,
								 const char* tz_name_str,
								 const int quiet_alerts,
								 FILE** custom_file );

int read_custom_days_file(
			FILE* config_file,
			int** jdn_list_ptr, char** string_list_ptr,
			const int d_todo, const int m_todo, const int y_todo,
			const char calendar_type,
			hdate_struct range_start,
			const int text_short_form, const int text_hebrew_form);
