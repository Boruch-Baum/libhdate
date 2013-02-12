/** timezone_functions.h            http://libhdate.sourceforge.net
 * Decode and dump information from a zonedata (TZif) timezone file.
 * The file is expected to have been compiled using the glibc 'zic'
 * command.
 *
 * Copyright:  2013 (c) Boruch Baum <zdump@gmx.com>
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

#define JERUSALEM_STANDARD_TIME_IN_MINUTES 120

int
get_lat_lon_from_zonetab_file( const char* search_string, char** tz_name, double *lat, double *lon, int quiet_alerts );

char*
read_sys_tz_string_from_file();

int
get_tz_adjustment(  const time_t t, const int tz, int *tzif_index,
					const int tzif_entries, const void *tzif_data );
