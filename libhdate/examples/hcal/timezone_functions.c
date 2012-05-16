/** timezone_functions.c            http://libhdate.sourceforge.net
 * a collection of functions in support of both hcal.c and hdate.c
 * hcal.c  Hebrew calendar              (part of package libhdate)
 * hdate.c Hebrew date/times information(part of package libhdate)
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` timezone_functions.c -o timezone_functions
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

#define _GNU_SOURCE		/// For mempcpy, asprintf
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>		/// For printf, fopen, fclose, FILE
#include <stdlib.h>		/// For malloc, free
#include <time.h>		/// For time
#include <error.h>		/// For error
#include <errno.h>		/// For errno
#include <unistd.h>		/// for sleep
#include <string.h>		/// for memset, memcpy
#include <sys/stat.h>	/// for stat
#include <locale.h>		/// for setlocale

#define FALSE 0
#define TRUE -1

#define TZIF1_FIELD_SIZE 4
#define TZIF2_FIELD_SIZE 8

#define BAD_COORDINATE	999

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


/***************************************************
* 
* definition of a tzfile header
* 
***************************************************/
#define HEADER_LEN 44
typedef struct {
	char magicnumber[6];	  // = TZif2 or TZif\0
//	char reserved[15];		  // nulls
	long unsigned ttisgmtcnt; // number of UTC/local indicators stored in the file.
	long unsigned ttisstdcnt; // number of standard/wall indicators stored in the file.
	long unsigned leapcnt;    // number of leap seconds for which data is stored in the file.
	long unsigned timecnt;    // number of "transition times" for which data is stored in the file.
	long unsigned typecnt;    // number of "local time types" for which data is stored in the file (must not be zero).
	long unsigned charcnt;    // number of  characters  of  "timezone  abbreviation  strings" stored in the file.
	} timezonefileheader;



/***************************************************
* 
* parse a tzfile "long format" value
* 
***************************************************/
long parse_tz_long( const char *sourceptr, const int field_size)
{
	long retval = 0;
	char *long_ptr;

//	if (sizeof(long) < field_size)
//		printf("warning: field truncated because it is larger than a 'long' integer\n\n");

	int i,j;
	long_ptr = (char*) &retval;
	j = 0;
	for (i=field_size-1; (i>=0) && (j<sizeof(long)); i--)
	{
		long_ptr[j] = sourceptr[i];
		j++;
	}
	return retval;
}



/***************************************************
* 
* read a tzheader file
* 
***************************************************/
int read_tz_header( timezonefileheader *header,  const char *temp_buffer)
{
	const int field_size = 4;

	memcpy( header->magicnumber, &temp_buffer[0], 5 );
	header->magicnumber[5] = '\0';

	header->ttisgmtcnt = parse_tz_long(&temp_buffer[20], field_size);
	header->ttisstdcnt = parse_tz_long(&temp_buffer[24], field_size);
	header->leapcnt = parse_tz_long(&temp_buffer[28], field_size);
	header->timecnt = parse_tz_long(&temp_buffer[32], field_size);
	header->typecnt = parse_tz_long(&temp_buffer[36], field_size);
	header->charcnt = parse_tz_long(&temp_buffer[40], field_size);

	printf("Header format ID: %s\n",header->magicnumber);
	printf("number of UTC/local indicators stored in the file = %ld\n", header->ttisgmtcnt);
	printf("number of standard/wall indicators stored in the file = %ld\n",header->ttisstdcnt);
	printf("number of leap seconds for which data is stored in the file = %ld\n",header->leapcnt);
	printf("number of \"transition times\" for which data is stored in the file = %ld\n",header->timecnt);
	printf("number of \"local time types\" for which data is stored in the file (must not be zero) = %ld\n",header->typecnt);
	printf("number of  characters  of  \"timezone  abbreviation  strings\" stored in the file = %ld\n\n",header->charcnt);

	if (header->typecnt == 0)
	{
		printf("Error in file format. Zero local time types suggested.\n");
		return FALSE;
	}

	if (header->timecnt == 0)
	{
		printf("No transition times recorded in this file\n");
		return FALSE;
	}

return TRUE;
}





 /***************************************************
 * 
 * Display contents of a timezone file
 * 
 ***************************************************/
void display_tzfile_data(	const timezonefileheader tzh,
							const char* tzfile_buffer_ptr,
							const int field_size /// 4bytes for tzif1, 8bytes for tzif2
							)
{
	long i;
	unsigned long temp_long;
	char ctime_buffer[200]; /// arbitrarily > 26 bytes
	long gmtoff;

	 /***************************************************
	 * 
	 * Structure of the timezone files
	 * 
	 ***************************************************/
//	start with the header info
//	timezonefileheader tzh;

//	Next in the file is the following array:
///	long unsigned transition_time[timecnt]
//	in ascending order, use ctime(2) to parse
	char	*transition_time_ptr;
	char	*temp_transition_time_ptr;
		
//	Next in the file is the following array:
///	char unsigned local_time_type[timecnt]
// which of the different types of "local time" types
// described in the  file  is associated  with  the
// same-indexed transition time. These values serve
// as indices  into  an  array  of  ttinfo  structures
// which follows.
		char *local_time_type_ptr;
		char *temp_local_time_ptr;

//	Next in the file is an array of length [typecnt] of the following structure:
//		typedef struct {
//			long gmtoff;	// seconds to add to UTC
//			char is_dst;	// whether tm_isdst should be set by  localtime(3)
//			char abbrind;	// index into the array of timezone abbreviation characters
//			} ttinfo_struct;
//	However, in practice, we will not implement it as a structure, in order to simplify
//	pointer arithmetic ...
		char *ttinfo_data_ptr, *temp_ttinfo_data_ptr;
		char *ttinfo_read_ptr, *temp_ttinfo_read_ptr;
/*		   localtime(3) uses the first standard-time ttinfo structure in the  file
       (or simply the first ttinfo structure in the absence of a standard-time
       structure) if either tzh_timecnt is zero or the time argument  is  less
       than the first transition time recorded in the file.
*/

		
//			Next in the file is supposed to an array of timezone aabreviations;
//			However, it was not explicitly defined in tzfile(5), so I'm guessing...
//			typedef char[4]	tz_abbrev;
		char* tz_abbrev_ptr;
		char* temp_tz_abbrev_ptr;

//			Next in the file is an array of length [leapcnt] of the following structure:
		typedef struct {
			long when; // time (as returned by time(2)) at which a leap second occurs
			long amt;  // total  number  of leap seconds to be applied after the given time
			}  leapinfo_struct;
		leapinfo_struct leapinfo;
		char *leapinfo_ptr;
		char *temp_leapinfo_ptr;
// HELP! - the leapinfo struct was not tested because my test file had it as a zero value !!


//			Next in the file is an array of length [ttisstdcnt] of the following:
//			char wall_indicator;
		char *wall_indicator_ptr;
		char *temp_wall_indicator_ptr;
		// whether the transition times associated with local_time_types were specified as
		// standard_time  or  wall_clock_time, and are used when a timezone file is used
		// in handling POSIX-style timezone environment variables.


//			Next in the file is an array of length [ttisgmtcnt] of the following:
//			char local_indicator;
		char *local_indicator_ptr;
		char *temp_local_indicator_ptr;
		// whether the transition times associated
		// with local_time types were specified as UTC or local time, and are used
		// when  a timezone file is used in handling POSIX-style timezone environ‐
		// ment variables.

//	This concludes the contents of the file if it was encoded using version 1 of
//	the format standard.
//  man tzfile claims that Version 2 of the format standard is identical to version
//	1, but appends the following, basically a repetition but using 8-byte values
//	for each  transition  time  or  leap-second  time, and concluding with a
//	'general rule' for transition times after the final one explicitly listed in
//	the file, or in their words "After the second  header and data comes a
//	newline-enclosed, POSIX-TZ-environment-variable-style string for use in
//	handling instants after the last transition time stored in the file (with
//	nothing between the newlines if there is no POSIX representation for such
//	instants).


 /********************************************************************
 * 
 * Beginning of code for function display_tzfile_data
 * 
 ********************************************************************/

	printf("\nContents of tzfile (eg. /etc/localtime)\n");

	transition_time_ptr = (char*) tzfile_buffer_ptr;
	local_time_type_ptr = (char*) tzfile_buffer_ptr + tzh.timecnt*field_size;
	ttinfo_read_ptr = local_time_type_ptr + tzh.timecnt;

	ttinfo_data_ptr = (char*) malloc( (sizeof(long) + (sizeof(char)*2) ) * tzh.typecnt);
	if (ttinfo_data_ptr == NULL)
	{
		printf("memory allocation error - ttinfo_data\ntzname = ");
		return;
	}


	/// Print transition times
	printf("\n List of transition times\n #    epoch  local_time_type  human-readable_time\n");
	temp_transition_time_ptr = (char *) transition_time_ptr + ((tzh.timecnt-1) * field_size);
	temp_local_time_ptr = (char *) local_time_type_ptr + (tzh.timecnt-1);
	for (i=tzh.timecnt-1;i>0; i--)
	{
		temp_long = parse_tz_long( temp_transition_time_ptr, field_size );
		if ( ctime_r( (const time_t *) &temp_long, (char*) &ctime_buffer) == NULL )
		{
			error(0,errno,"error returned by ctime_r in transition time data\n");
			free(ttinfo_data_ptr);
			return;
		}
		printf("%ld: %ld -- %x --- %s",i, temp_long,*temp_local_time_ptr, (char*) &ctime_buffer);
		temp_transition_time_ptr = temp_transition_time_ptr - field_size;
		temp_local_time_ptr = temp_local_time_ptr - 1;
	}


	/// Print ttinfo data
	temp_ttinfo_read_ptr = ttinfo_read_ptr;
	printf("\nttinfo data\n  gmt_offset  HH:mm:ss  is_dst  abbr_ind\n");
	for (i=0;i<tzh.typecnt;i++)
	{
		temp_ttinfo_data_ptr = ttinfo_data_ptr + i;
		gmtoff = parse_tz_long( temp_ttinfo_read_ptr, 4 );
		temp_ttinfo_data_ptr[0] = gmtoff;
		temp_ttinfo_data_ptr[4] = temp_ttinfo_read_ptr[4]; /// is_dst
		temp_ttinfo_data_ptr[5] = temp_ttinfo_read_ptr[5]; /// abbrind
		printf("%ld: %ld     %.2ld:%.2ld:%.2ld    %d       %d\n", i,
				gmtoff,	gmtoff/3600, gmtoff%3600/60, gmtoff%60,
				(short int) temp_ttinfo_data_ptr[4], (short int) temp_ttinfo_data_ptr[4] );
		temp_ttinfo_read_ptr = temp_ttinfo_read_ptr + 6;
	}

	/// timezone abbreviation list
	tz_abbrev_ptr = ttinfo_read_ptr + (tzh.typecnt * 6);
	printf("\ntime zone abbreviations\n");
	temp_tz_abbrev_ptr = tz_abbrev_ptr;
	for (i=0;i<tzh.typecnt;i++)
	{
		if ( (temp_tz_abbrev_ptr - tz_abbrev_ptr) > tzh.charcnt) printf("%ld:\n",i);
		else printf("%ld: %s\n",i, temp_tz_abbrev_ptr );
		temp_tz_abbrev_ptr = strchr( temp_tz_abbrev_ptr, 0 ) + 1;
	}


//	leapinfo_ptr = tz_abbrev_ptr + (tzh.typecnt*4);
	leapinfo_ptr = tz_abbrev_ptr + (tzh.charcnt);
	if (tzh.leapcnt != 0)
	{
		temp_leapinfo_ptr = leapinfo_ptr;
		printf("\nleapinfo data\n  when  human_readable  num_of_seconds\n");
		for (i=0;i<tzh.leapcnt;i++)
		{
			leapinfo.when = parse_tz_long( temp_leapinfo_ptr, field_size );
			temp_leapinfo_ptr = temp_leapinfo_ptr +field_size;
			leapinfo.amt = parse_tz_long( temp_leapinfo_ptr, field_size );
			if ( ctime_r( (const time_t *) &leapinfo.when, (char*) &ctime_buffer) == NULL )
			{
				error(0,errno,"error returned by ctime_r in leapinfo data\n");
				free(ttinfo_data_ptr);
				return;
			}
			printf("%ld: %ld        %s     %ld\n",
					i, leapinfo.when, (char*) &ctime_buffer, leapinfo.amt );
			temp_leapinfo_ptr = temp_leapinfo_ptr +field_size;
		}
	}


	wall_indicator_ptr = leapinfo_ptr + (tzh.leapcnt*field_size*2);
	if (tzh.ttisstdcnt != 0)
	{
		temp_wall_indicator_ptr = wall_indicator_ptr;
		printf("\nwall_indicator data\n");
		for (i=0;i<tzh.ttisstdcnt;i++)
		{
			printf("%ld: %x\n",i, *temp_wall_indicator_ptr);
			temp_wall_indicator_ptr = temp_wall_indicator_ptr + 1;
		}
	}


	local_indicator_ptr = wall_indicator_ptr + tzh.ttisstdcnt;
	if (tzh.ttisgmtcnt != 0)
	{
		temp_local_indicator_ptr = local_indicator_ptr;
		printf("\nlocal_indicator data\n");
		for (i=0;i<tzh.ttisgmtcnt;i++)
		{
			printf("%ld: %x\n",i, *temp_local_indicator_ptr);
			temp_local_indicator_ptr = temp_local_indicator_ptr + 1;
		}
	}

	if (field_size == TZIF2_FIELD_SIZE)
	{
		/// parse 'general rule' at end of file
		/// format should be per man(3) tzset
		printf("\n\'general rule\' information\n");
		char *start_line_ptr;
		start_line_ptr = strchr( local_indicator_ptr + tzh.ttisgmtcnt, 10 );
		if (start_line_ptr == NULL)
			printf("no \'general rule\' information found at end of file\n");
		else
		{
			char *end_line_ptr;
			end_line_ptr = strchr( start_line_ptr+1, 10 );
			if (end_line_ptr == NULL)
				printf("error finding \'general rule\' info terminator symbol\n");
			else
			{
				*end_line_ptr = '\0';
				int bytes_printed;
				bytes_printed = printf("general rule (unparsed): %s\n", start_line_ptr+1 ) - 27;
				printf("bytes printed = %d\n", bytes_printed);
			}
			
		}
			
	}

	free(ttinfo_data_ptr);
}

// Find the current time interval and return information for it
// ie local time type, leap seconds
// maybe use mktime to get seconds_since_epoch and compare to data in this file



void test_mktime ()
/************************************************************************************************************************************************************************************

       The  mktime()  function converts a broken-down time structure, expressed as local time, to calendar time representation.  The function ignores the values supplied by
       the caller in the tm_wday and tm_yday fields.  The value specified in the tm_isdst field informs mktime() whether or not daylight saving time (DST) is in effect  for
       the time supplied in the tm structure: a positive value means DST is in effect; zero means that DST is not in effect; and a negative value means that mktime() should
       (use timezone information and system databases to) attempt to determine whether DST is in effect at the specified time.

        The mktime() function modifies the fields of the tm structure as follows: tm_wday and tm_yday are set to values determined from the contents of the other fields;  if
       structure  members  are outside their valid interval, they will be normalized (so that, for example, 40 October is changed into 9 November); tm_isdst is set (regard‐
       less of its initial value) to a positive value or to 0, respectively, to indicate whether DST is or is not in effect at the specified time.   Calling  mktime()  also
       sets the external variable tzname with information about the current timezone.
           struct tm {
               int tm_sec;         / seconds /
               int tm_min;         / minutes /
               int tm_hour;        / hours /
               int tm_mday;        / day of the month /
               int tm_mon;         / month /
               int tm_year;        / year /
               int tm_wday;        / day of the week /
               int tm_yday;        / day in the year /
               int tm_isdst;       / daylight saving time /
               long tm_gmtoff;     / seconds east of UTC - GLIBC only
               const char* tm_zone;/ Timezone abbreviation - GLIBC only
           };

		from command line, date +%s gives epoch time

************************************************************************************************************************************************************************************/
{

	FILE *tzfile;
	struct stat file_status;
	char* buffer_ptr;

	tzfile = fopen("/etc/timezone", "rb");
	if (tzfile == NULL)
	{
		error(errno,0,"tz file /etc/timezone not found\n");
		return;
	}
	if (fstat( fileno( tzfile), &file_status) != 0)
	{
		printf("error retreiving file status\n");
		return;
	}

	printf("file size is %ld bytes\n", file_status.st_size);

	buffer_ptr = (char *) malloc( file_status.st_size );
	if (buffer_ptr == NULL)
	{
		printf("memory allocation error - buffer\n");
		return;
	}

	if ( fread( buffer_ptr, file_status.st_size, 1, tzfile) != 1 )
	{
		printf("error reading into buffer\n");
		free(buffer_ptr);
		return;
	}
	fclose(tzfile);

	buffer_ptr[file_status.st_size-1] = '\0';
	printf("system timezone name = %s\n\n",buffer_ptr);




	struct tm tm1;
	struct tm* tm2;
	time_t time_in_sec;

	tzset();
	char *tza;
	tzset();
	printf("Environmental variables:\n tzname = %s, timezone =  %ld:%ld:%ld, daylight = %d, TZ = ",
			*tzname, timezone/3600, timezone%3600/60, timezone%60, daylight);
	tza = getenv("TZ");
	if (tza== NULL) printf("null, ");
	else printf("%s, ",tza);
	printf("tzname[0]: %s, tzname[1]: %s\n\n",tzname[0], tzname[1]);

	if (time(&time_in_sec) == -1) printf("error %d retreiving time using time() function\n", errno);

	printf("ctime output: %s", ctime(&time_in_sec));
	// ctime doesn't return any timezone indication whatsoever
	printf("tzname: %s, timezone: %ld:%ld:%ld, daylight: %d\n",
			*tzname, timezone/3600, timezone%3600/60, timezone%60, daylight);
	// returns EST (should be EDT)
	// daylight does correctly equal 1, but it is 'obsolete', so on't rely on it (man 3 tzset)
	// timezone returns 18000, which is 5 hours; it should be 14400 (4) for EDT


	tm1.tm_sec = 0;
	tm1.tm_min = 0;
	tm1.tm_hour = 6;
	tm1.tm_mday = 21;
	tm1.tm_mon = 5;
	tm1.tm_year = 112; // since 1900 , NOT EPOCH!
	tm1.tm_wday = 0;
	tm1.tm_yday = 0;
	tm1.tm_isdst = -1; // force mktime to figure it out
	// now let's account for the GLIBC extensions
	tm1.tm_gmtoff = 0;
	tm1.tm_zone = NULL;

	printf("\nasctime: %s\n", asctime(&tm1));

	time_in_sec = mktime(&tm1);
	printf("mktime: %ld. tzname: %s, dst flag: %d, gmtoff: %ld, tm_zone: %s\n\n",
			time_in_sec, *tzname, tm1.tm_isdst, tm1.tm_gmtoff, tm1.tm_zone);
	// tzname incorrectly reports EST
	// tm_tm_isdst correctly modified to 1
	// tm1.tm_gmtoff correctly reports -14400
	// tm1.tm_zone reports correctly EDT

	tm2 = localtime(&time_in_sec);
	printf("localtime output: %d:%d:%d %d:%d:%d, isdst: %d, gmt_off: %ld, tm_zone: %s\n",
			tm2->tm_year, tm2->tm_mon, tm2->tm_mday, tm2->tm_hour, tm2->tm_min, tm2->tm_sec,
			tm2->tm_isdst, tm2->tm_gmtoff, tm2->tm_zone);
	// returns proper structure
	// USE THREAD SAFE VERSION: struct tm *localtime_r(const time_t *timep, struct tm *result);

	printf("EXTERNAL VARIABLES tzname: %s, timezone: %ld, daylight: %d\n", *tzname, timezone, daylight);
	// returns EST (should be EDT)
	// daylight does correctly equal 1, but it is 'obsolete', so on't rely on it (man 3 tzset)
	// timezone returns 18000, which is 5 hours; it should be 14400 (4) for EDT
	

	printf("\nmktime: test result when setting tm1.tm_zone to PST\n");
	tm1.tm_isdst = -1; // force mktime to figure it out
	tm1.tm_gmtoff = 0;
	tm1.tm_zone = "PST";
	time_in_sec = mktime(&tm1);
	printf("mktime: %ld. tzname: %s, dst flag: %d, gmtoff: %ld, tm_zone: %s\n\n",
			time_in_sec, *tzname, tm1.tm_isdst, tm1.tm_gmtoff, tm1.tm_zone);
	// tzname incorrectly reports EST
	// tm_tm_isdst correctly modified to 1
	// tm1.tm_gmtoff correctly reports -14400
	// tm1.tm_zone reports correctly EDT


	return;
}





/***********************************************************************
* tzif2_handle
*
* 
* 
***********************************************************************/

char* tzif2_handle( timezonefileheader *tzh, const char *tzfile_buffer_ptr, size_t buffer_size )
{
	char *start_ptr;
	char magicnumber[5] = "TZif2";

	printf("handle format 2\n");

	start_ptr = memmem( tzfile_buffer_ptr, buffer_size, &magicnumber, 5 );
	if (start_ptr == NULL)
	{
		printf("error finding tzif2 header\n");
		return NULL;
	}

	printf("tzif2 header found at position %ld\n", start_ptr - tzfile_buffer_ptr);
	if ( read_tz_header( tzh, (char*) start_ptr ) == FALSE )
	{
		printf("Error reading header file version 2\n");
		return NULL;
	}

	return start_ptr + HEADER_LEN;
}




/***********************************************************************
* timezone_display_local_info
*
* This function was written as an exercise to verify how to parse/read
* all the contents of a TZif (timezone definition) file.
* 
***********************************************************************/
void timezone_display_local_info()
{
	FILE *tz_file;
	char *tzif_buffer_ptr;
	char *start_ptr;
	int   field_size;
	struct stat file_status;
	timezonefileheader tzh;


	char *tza;
	tzset();
	printf("Environmental variables:\n tzname = %s, timezone =  %ld:%ld:%ld, daylight = %d, TZ = ",
			*tzname, timezone/3600, timezone%3600/60, timezone%60, daylight);
	tza = getenv("TZ");
	if (tza== NULL) printf("null\n\n");
	else printf("%s\n\n",tza);
	

char* locale_ptr = setlocale(LC_TIME,"");
char* tzdir_ptr = getenv("TZDIR");
printf("locale string: %s\n tzdir string: %s\n\n", locale_ptr, tzdir_ptr);

//	FIXME - use $TZDIR !
	tz_file = fopen("/etc/localtime", "rb");
//	tz_file = fopen("/usr/share/zoneinfo/posix/US/Alaska", "rb");
//	tz_file = fopen("/usr/share/zoneinfo/posix/MST", "rb");			// no transition times in part one
//	tz_file = fopen("/usr/share/zoneinfo/posix/MST7MDT", "rb");
//	tz_file = fopen("/usr/share/zoneinfo/posix/Asia/Baghdad", "rb");
//	tz_file = fopen("/usr/share/zoneinfo/posix/Asia/Kabul", "rb");
//	tz_file = fopen("/usr/share/zoneinfo/posix/Asia/Kathmandu", "rb");
//	tz_file = fopen("/usr/share/zoneinfo/posix/Asia/Calcutta", "rb");
//	tz_file = fopen("/usr/share/zoneinfo/posix/Asia/Shanghai", "rb");
//	tz_file = fopen("/usr/share/zoneinfo/posix/Asia/Jerusalem", "rb");
	tz_file = fopen("/usr/share/zoneinfo/posix/America/Buenos_Aires", "rb");
//	tz_file = fopen("/usr/share/zoneinfo/posix/America/Sao_Paulo", "rb");

	if (tz_file == NULL)
	{
		error(errno,0,"tz file /etc/localtime not found\n");
		return;
	}




	if (fstat( fileno( tz_file), &file_status) != 0)
	{
		printf("error retreiving file status\n");
		return;
	}

	printf("file size is %ld bytes\n\n", file_status.st_size);

	tzif_buffer_ptr = (char *) malloc( file_status.st_size );
	if (tzif_buffer_ptr == NULL)
	{
		printf("memory allocation error - tzif buffer\n");
		return;
	}

	if ( fread( tzif_buffer_ptr, file_status.st_size, 1, tz_file) != 1 )
	{
		printf("error reading into tzif buffer\n");
		free(tzif_buffer_ptr);
		return;
	}
	fclose(tz_file);


	if ( read_tz_header( &tzh, tzif_buffer_ptr ) == FALSE )
	{
		printf("Error reading header file version 1\n");
		return;
	}

	if (tzh.magicnumber[4] == 50 )
	{
		start_ptr = tzif2_handle( &tzh, &tzif_buffer_ptr[HEADER_LEN], file_status.st_size - HEADER_LEN);
		field_size = TZIF2_FIELD_SIZE;
	}
	else
	{
		start_ptr = &tzif_buffer_ptr[HEADER_LEN];
		field_size = TZIF1_FIELD_SIZE;
	}
	if (start_ptr != NULL ) display_tzfile_data( tzh, start_ptr, field_size );

	


	free(tzif_buffer_ptr);
}



/***********************************************************************
* read_sys_tz_string_from_file
* 
* returns a pointer to a malloc'd buffer containing the contents of
* /etc/timezone, only it is null-terminated, not end-of-line terminate.
* This should be something like 'America/New_York'. This buffer must be
* free()'d.
* 
* returns a NULL pointer upon any failure.
*  
***********************************************************************/
char* read_sys_tz_string_from_file()
{
	FILE *tzfile;
	struct stat file_status;
	char* retval_ptr;

	tzfile = fopen("/etc/timezone", "rb");
	if (tzfile == NULL)
	{
		// error(errno,0,"tz file /etc/timezone not found\n");
		return NULL;
	}
	if (fstat( fileno(tzfile), &file_status) != 0)
	{
		//printf("error retreiving file status\n");
		return NULL;
	}

	retval_ptr = (char *) malloc( file_status.st_size );
	if (retval_ptr == NULL)
	{
		// printf("memory allocation error - buffer\n");
		return NULL;
	}

	if ( fread( retval_ptr, file_status.st_size, 1, tzfile) != 1 )
	{
		// printf("error reading into buffer\n");
		free(retval_ptr);
		return NULL;
	}
	fclose(tzfile);

	retval_ptr[file_status.st_size-1] = '\0';
	// printf("system timezone name = %s\n\n",retval_ptr);
	return retval_ptr;
}



/***********************************************************************
* print_alert_coordinate
* 
***********************************************************************/
void print_alert_coordinate( const char *city_name )
{
	error(0,0,"%s",
			N_("ALERT: coordinates not entered or invalid..."));
	error(0,0,"%s %s",
			N_("ALERT: guessing... will use co-ordinates for"),
			city_name);
}


/***********************************************************************
* get_lat_lon_from_zonetab_file
*
* The zone.tab file is often "/usr/share/zoneinfo/zone.tab". It is a
* table in the format:
* aa [+-]nnnnnn[+-]nnnnnnn bbbbbb/bbbbbbb cccccccccccccccccccc
* where:
* aa	is a two-character country code
* nn	is latitude an longitude in form ddmm and optionally ss,
*   	with NO space between the numbers, only the +/- sign.
* bb	is the official name of the timezone which, I guess is
*   	what is at the location nn above.
* cc	is an extended description of the timezone location.
* 
* returns FALSE upon any failure.
*  
***********************************************************************/
int get_lat_lon_from_zonetab_file( const char* search_string, double *lat, double *lon, int quiet_alerts )
{

	double convert_from_dms( long in_val, int num_of_digits )
	{
		/// sanity check performed by caller, so we know
		/// that the number is four, six or seven digits
		
		/// Let the compiler do the arithmetic an make it clear for us

		if (num_of_digits == 4)
			return ( (double) in_val/100 )   + ((double)(in_val%100))/60;
		else
			return ( (double) (in_val/10000) ) + (double) ((in_val%10000)/100) /60 + (double) (in_val%100)/3600;
	}


	char*	tzdir_path;
	char*	zonetab_path;
	const char*	default_zonetab_path = "/usr/share/zoneinfo/zone.tab";

	FILE*	tzfile;
	char*	lat_and_lon = NULL;
	char*	location = NULL;
	char*	input_string = NULL;
	size_t	input_str_len;
	int		end_of_input_file = FALSE;
	int		match_count;
	char*	search_result_ptr = NULL;
	char	number_string[9];
	size_t	lat_end, lon_end;


	/** In Debian, the TZDIR environmental variable is unset
	 ** and the default is used, but the POSIX spec allows for
	 ** a flexible location ofTZDIR. */
	setlocale(LC_TIME,"");
	tzdir_path = getenv("TZDIR");
	if ((tzdir_path == NULL) ||
		(asprintf( &zonetab_path, "%s/zone,tab", tzdir_path) == -1) )
		tzfile = fopen(default_zonetab_path, "rb");
	else
	{
		tzfile = fopen(zonetab_path, "rb");
		free(zonetab_path);
	}
	if (tzfile == NULL)	return FALSE;

	while ( (end_of_input_file!=TRUE) && (search_result_ptr == NULL) )
	{
		end_of_input_file = getline(&input_string, &input_str_len, tzfile);
		if ( end_of_input_file!=TRUE )
		{
			match_count = sscanf(input_string,"%*2c %a[+0-9-] %as %*[^\n]", &lat_and_lon, &location); 
			if (match_count == 2)
			{
				search_result_ptr = strstr(location, search_string);
				if (search_result_ptr != NULL)
				{
					/// maybe replace all this next part with
					/// two calls to strtol
					lat_end = strcspn( &lat_and_lon[1], "+-");
					if ((lat_end < 4) || (lat_end > 6) )
					{
						if (!quiet_alerts) printf("error parsing latitude from zone.tab file\n");
						*lat = BAD_COORDINATE;
						*lon = BAD_COORDINATE;
					}
					else
					{
						memcpy(&number_string, lat_and_lon, lat_end+1);
						number_string[lat_end+1] = '\0';
						*lat = convert_from_dms ( atol(number_string), lat_end);
						lon_end = strlen( &lat_and_lon[lat_end+1]);
						if ((lon_end < 5) || (lon_end > 8) )
						{
							if (!quiet_alerts) printf("error parsing longitude from zone.tab file. lon_end = %ld\n", lon_end);
							*lat = BAD_COORDINATE;
							*lon = BAD_COORDINATE;
						}
						else
						{
							memcpy(&number_string, &lat_and_lon[lat_end+1], lon_end+1);
							number_string[lon_end+1] = '\0';

							*lon = convert_from_dms ( atol(number_string), lon_end);
							if (!quiet_alerts) print_alert_coordinate(search_string);
//							printf("lat %lf lon %lf\n\n", *lat, *lon);
						}
					}
				}
			}
			if (lat_and_lon != NULL)
			{
				free(lat_and_lon);
				lat_and_lon = NULL;
			}
			if (location != NULL)
			{
				free(location);
				location = NULL;
			}
		}
	}
	fclose(tzfile);
	if (search_result_ptr != NULL) return TRUE;
	return FALSE;
}
