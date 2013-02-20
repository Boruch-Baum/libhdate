/** custom_days.h            http://libhdate.sourceforge.net
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
 
extern char* get_custom_day_text_ptr(const int index, char* string_list_ptr);

char* get_custom_day_symbol_ptr(const int index, char* string_list_ptr);

extern int get_custom_days_list( int** jdn_list_ptr, char** string_list_ptr,
			const int day, const int month, const int year,
			const char calendar, const int quiet_alerts,
			const hdate_struct range_start,
			const char* config_dir, const char* config_filename,
			const int text_short_form, const int text_hebrew_form );
