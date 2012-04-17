/* local_functions.c            http://libhdate.sourceforge.net
 * a collection of functions in support of both hcal.c and hdate.c
 * hcal.c  Hebrew calendar              (part of package libhdate)
 * hdate.c Hebrew date/times information(part of package libhdate)
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` ocal_functions.c -o local_functions
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

#include <stdio.h>		// For printf, fopen, fclose, FILE
#include <stdlib.h>		// For malloc, free
#include <time.h>		// For time
#include <error.h>		// For error
#include <errno.h>		// For errno


#define FALSE 0
#define TRUE -1

void parse_tz_long( char *destptr, char *sourceptr)
{
	destptr[0] = sourceptr[3];
	destptr[1] = sourceptr[2];
	destptr[2] = sourceptr[1];
	destptr[3] = sourceptr[0];
}


int read_tz_long( char *destptr, FILE * tz_file)
{
	char longbuffer[4];

	if ( fread( &longbuffer, 4, 1, tz_file) != 1 ) return FALSE;
	destptr[0] = longbuffer[3];
	destptr[1] = longbuffer[2];
	destptr[2] = longbuffer[1];
	destptr[3] = longbuffer[0];
	return TRUE;
}

void timezone_display_tzinfo_file_data_v2( FILE *tz_file)
{
	char *temp_ptr;
	long i;
	unsigned long temp_long;


	 /***************************************************
	 * 
	 * Structure of the timezone files
	 * 
	 ***************************************************/
	// Let's start with the header info
	typedef struct {
		char magicnumber[5];	  // = TZif2 or TZif\0
		char reserved[15];		  // nulls
		long unsigned ttisgmtcnt; // number of UTC/local indicators stored in the file.
		long unsigned ttisstdcnt; // number of standard/wall indicators stored in the file.
		long unsigned leapcnt;    // number of leap seconds for which data is stored in the file.
		long unsigned timecnt;    // number of "transition times" for which data is stored in the file.
		long unsigned typecnt;    // number of "local time types" for which data is stored in the file (must not be zero).
		long unsigned charcnt;    // number of  characters  of  "timezone  abbreviation  strings" stored in the file.
		} timezonefileheader;
	timezonefileheader tzh;

//			Next in the file is the following array:
//			long unsigned transition_time[timecnt] // in ascending order, use ctime(2) to parse
		unsigned long *transition_time;
		char          *temp_transition_time_ptr;
		
//			Next in the file is the following array:
//			char unsigned local_time_type[timecnt] // which of the different types of "local time" types
											   // described in the  file  is associated  with  the
											   // same-indexed transition time. These values serve
											   // as indices  into  an  array  of  ttinfo  structures
											   // which follows.
		char *local_time_type;
		char *temp_local_time_ptr;

//			Next in the file is an array of length [typecnt] of the following structure:
		typedef struct {
			long gmtoff;	// seconds to add to UTC
			char is_dst;	// whether tm_isdst should be set by  localtime(3)
			char abbrind;	// index into the array of timezone abbreviation characters
			} ttinfo_struct;
		ttinfo_struct *ttinfo_data_ptr, *temp_ttinfo_data_ptr;
		char *ttinfo_read_ptr, *temp_ttinfo_read_ptr;
/*		   localtime(3) uses the first standard-time ttinfo structure in the  file
       (or simply the first ttinfo structure in the absence of a standard-time
       structure) if either tzh_timecnt is zero or the time argument  is  less
       than the first transition time recorded in the file.
*/

		
//			Next in the file is supposed to an array of timezone aabreviations;
//			However, it was not explicitly defined in tzfile(5), so I'm guessing...
//			typedef char[4]	tz_abbrev;
		char tz_abbrev[5];
		tz_abbrev[4] = '\0';

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
		// whether the transition times associated with local time types were specified as
		// standard  time  or  wall  clock time, and are used when a timezone file is used
		// in handling POSIX-style timezone environment variables.


//			Next in the file is an array of length [ttisgmtcnt] of the following:
//			char local_indicator;
		char *local_indicator_ptr;
		char *temp_local_indicator_ptr;
		// whether the transition times associated
		// with local time types were specified as UTC or local time, and are used
		// when  a timezone file is used in handling POSIX-style timezone environ‐
		// ment variables.

		
		// For version-2-format timezone files, the above header and data is  fol‐
		// lowed  by  a  second  header  and data, identical in format except that
		// eight bytes are used for each  transition  time  or  leap-second  time.
		// After  the  second  header and data comes a newline-enclosed, POSIX-TZ-
		// environment-variable-style string for use in  handling  instants  after
		// the  last  transition time stored in the file (with nothing between the
		// newlines if there is no POSIX representation for such instants).


// HELP - I haven't yet done this next part ...
/*
   For version-2-format timezone files, the above header and data is  fol‐
   lowed  by  a  second  header  and data, identical in format except that
   eight bytes are used for each  transition  time  or  leap-second  time.

   After  the  second  header and data comes a newline-enclosed, POSIX-TZ-
   environment-variable-style string for use in  handling  instants  after
   the  last  transition time stored in the file (with nothing between the
   newlines if there is no POSIX representation for such instants).
*/




	// Initialize malloc() pointers so we know what to free()
	transition_time = NULL;
	local_time_type = NULL;
	leapinfo_ptr = NULL;
	wall_indicator_ptr = NULL;
	local_indicator_ptr = NULL;
	

	if ( fread( &tzh, 20, 1, tz_file) != 1 ) {printf("error reading header\n"); return; }

	temp_ptr = (char *) &tzh.ttisgmtcnt;
	for (i=0;i<6;i++)
	{
		read_tz_long(temp_ptr, tz_file);
		temp_ptr = temp_ptr + 4;
	}

	printf("\nContents of file /etc/localtime\n");
	printf("%s\n",tzh.magicnumber);
	printf("number of UTC/local indicators stored in the file = %ld\n", tzh.ttisgmtcnt);
	printf("number of standard/wall indicators stored in the file = %ld\n",tzh.ttisstdcnt);
	printf("number of leap seconds for which data is stored in the file = %ld\n",tzh.leapcnt);
	printf("number of \"transition times\" for which data is stored in the file = %ld\n",tzh.timecnt);
	printf("number of \"local time types\" for which data is stored in the file (must not be zero) = %ld\n",tzh.typecnt);
	printf("number of  characters  of  \"timezone  abbreviation  strings\" stored in the file = %ld\n",tzh.charcnt);

	if (tzh.typecnt == 0)
	{
		printf("Error in file format. Zero local time types suggested.\n");
		return;
	}

	if (tzh.timecnt == 0)
	{
		printf("No transition times recorded in this file\n");
		return;
	}

	transition_time = malloc(tzh.timecnt*4);
	if (transition_time == NULL)
	{
		printf("memory allocation error - transition times");
		return;
	}

	if ( fread( transition_time, tzh.timecnt*4, 1, tz_file) != 1 )
	{
		printf("error reading transition times\n");
		free(transition_time);
		return;
	}
	

	local_time_type = malloc(tzh.timecnt);
	if (local_time_type == NULL)
	{
		printf("memory allocation error - local time type");
		free(transition_time);
		return;
	}

	if ( fread( local_time_type, tzh.timecnt, 1, tz_file) != 1 )
	{
		printf("error reading local time types\n");
		free(local_time_type);
		free(transition_time);
		return;
	}

	ttinfo_read_ptr = malloc(tzh.typecnt*6);
	if (ttinfo_read_ptr == NULL)
	{
		printf("memory allocation error - ttinfo_read");
		free(local_time_type);
		free(transition_time);
		return;
	}

	ttinfo_data_ptr = malloc(tzh.typecnt);
	if (ttinfo_data_ptr == NULL)
	{
		printf("memory allocation error - ttinfo_data");
		free(ttinfo_read_ptr);
		free(local_time_type);
		free(transition_time);
		return;
	}


	if ( fread( ttinfo_read_ptr, 6, tzh.typecnt, tz_file) != tzh.typecnt)
	{
		printf("error reading ttinfo\n");
		free(ttinfo_read_ptr);
		free(ttinfo_data_ptr);
		free(local_time_type);
		free(transition_time);
		return;
	}

	printf("\n List of transition times\n #    epoch  local_time_type  human-readable_time\n");
	temp_transition_time_ptr = (char *) transition_time + ((tzh.timecnt-1) * 4);
	temp_local_time_ptr = (char *) local_time_type + (tzh.timecnt-1);
	for (i=tzh.timecnt-1;i>=0; i--)
	{
		parse_tz_long( (char *) &temp_long, temp_transition_time_ptr );
		printf("%ld: %ld -- %x --- %s",i, temp_long,*temp_local_time_ptr,ctime( (const time_t *) &temp_long));
		temp_transition_time_ptr = temp_transition_time_ptr - 4;
		temp_local_time_ptr = temp_local_time_ptr - 1;
	}

	temp_ttinfo_read_ptr = ttinfo_read_ptr;
	printf("\nttinfo data\n  gmt_offset  HH:mm:ss  is_dst  abbr_ind\n");
	for (i=0;i<tzh.typecnt;i++)
	{
		temp_ttinfo_data_ptr = ttinfo_data_ptr +i;
		long gmtoff = 0;
		parse_tz_long( (char *) &gmtoff, temp_ttinfo_read_ptr );
		temp_ttinfo_data_ptr->gmtoff = gmtoff;
		temp_ttinfo_data_ptr->is_dst = temp_ttinfo_read_ptr[4];
		temp_ttinfo_data_ptr->abbrind = temp_ttinfo_read_ptr[5];
		printf("%ld: %ld     %.2ld:%.2ld:%.2ld    %d       %d\n",i, temp_ttinfo_data_ptr->gmtoff,
				temp_ttinfo_data_ptr->gmtoff/3600, temp_ttinfo_data_ptr->gmtoff%3600/60, temp_ttinfo_data_ptr->gmtoff%60,
				temp_ttinfo_data_ptr->is_dst, temp_ttinfo_data_ptr->abbrind );
		temp_ttinfo_read_ptr = temp_ttinfo_read_ptr + 6;
	}

	free(local_time_type);
	free(transition_time);
	free(ttinfo_data_ptr);
	free(ttinfo_read_ptr);

	printf("\ntime zone abbreviations\n");
	for (i=0;i<tzh.typecnt;i++)
	{
		if ( fread( &tz_abbrev, 4, 1, tz_file) != 1 )
		{
			printf("error reading timezone abbreviations\n");
			return;
		}
		printf("%ld: %s\n",i, tz_abbrev );
	}


	if (tzh.leapcnt != 0)
	{
		leapinfo_ptr = malloc(tzh.leapcnt*8);
		if (leapinfo_ptr == NULL)
		{
			printf("memory allocation error - leapinfo");
			return;
		}

		if ( fread( leapinfo_ptr, tzh.leapcnt*8, 1, tz_file) != 1 )
		{
			printf("error reading leapinfo\n");
			free(leapinfo_ptr);
			return;
		}

		temp_leapinfo_ptr = leapinfo_ptr;
		printf("\nleapinfo data\n  when  human_readable  num_of_seconds\n");
		for (i=0;i<tzh.leapcnt;i++)
		{
			parse_tz_long( (char *) &leapinfo.when, temp_leapinfo_ptr );
			temp_leapinfo_ptr = temp_leapinfo_ptr +4;
			parse_tz_long( (char *) &leapinfo.amt, temp_leapinfo_ptr );
			printf("%ld: %ld        %s     %ld\n",i, leapinfo.when, ctime( (const time_t *) &leapinfo.when), leapinfo.amt );
			temp_leapinfo_ptr = temp_leapinfo_ptr +4;
		}
		free(leapinfo_ptr);
	}

	if (tzh.ttisstdcnt != 0)
	{
		wall_indicator_ptr = malloc(tzh.ttisstdcnt);
		if (wall_indicator_ptr == NULL)
		{
			printf("memory allocation error - wall_indicator");
			return;
		}

		if ( fread( wall_indicator_ptr, tzh.ttisstdcnt, 1, tz_file) != 1 )
		{
			printf("error reading wall_indicator\n");
			free(wall_indicator_ptr);
			return;
		}

		temp_wall_indicator_ptr = wall_indicator_ptr;
		printf("\nwall_indicator data\n");
		for (i=0;i<tzh.ttisstdcnt;i++)
		{
			printf("%ld: %x\n",i, *temp_wall_indicator_ptr);
			temp_wall_indicator_ptr = temp_wall_indicator_ptr + 1;
		}
		free(wall_indicator_ptr);
	}

	if (tzh.ttisgmtcnt != 0)
	{
		local_indicator_ptr = malloc(tzh.ttisgmtcnt);
		if (local_indicator_ptr == NULL)
		{
			printf("memory allocation error - local_indicator");
			return;
		}

		if ( fread( local_indicator_ptr, tzh.ttisgmtcnt, 1, tz_file) != 1 )
		{
			printf("error reading local_indicator\n");
			free(local_indicator_ptr);
			return;
		}

		temp_local_indicator_ptr = local_indicator_ptr;
		printf("\nlocal_indicator data\n");
		for (i=0;i<tzh.ttisgmtcnt;i++)
		{
			printf("%ld: %x\n",i, *temp_local_indicator_ptr);
			temp_local_indicator_ptr = temp_local_indicator_ptr + 1;
		}
		free(local_indicator_ptr);
	}

	if (tzh.magicnumber[4] == 50) printf("\nNotice: This file is in version 2 of the format.\n\
At this point in the file, the data basically repeats;\n\
However, there is a final line for handling times beyond\n\
the last recorded transition. ... TBD ... well, maybe ....\n");
	return;
}



void timezone_display_tzinfo_file_data_v1( FILE *tz_file)
{
	char *temp_ptr;
	long i;
	unsigned long temp_long;

	 /***************************************************
	 * 
	 * Structure of the timezone files
	 * 
	 ***************************************************/
	// Let's start with the header info
	typedef struct {
		char magicnumber[5];	  // = TZif2 or TZif\0
		char reserved[15];		  // nulls
		long unsigned ttisgmtcnt; // number of UTC/local indicators stored in the file.
		long unsigned ttisstdcnt; // number of standard/wall indicators stored in the file.
		long unsigned leapcnt;    // number of leap seconds for which data is stored in the file.
		long unsigned timecnt;    // number of "transition times" for which data is stored in the file.
		long unsigned typecnt;    // number of "local time types" for which data is stored in the file (must not be zero).
		long unsigned charcnt;    // number of  characters  of  "timezone  abbreviation  strings" stored in the file.
		} timezonefileheader;
	timezonefileheader tzh;

//			Next in the file is the following array:
//			long unsigned transition_time[timecnt] // in ascending order, use ctime(2) to parse
		unsigned long *transition_time;
		char          *temp_transition_time_ptr;
		
//			Next in the file is the following array:
//			char unsigned local_time_type[timecnt] // which of the different types of "local time" types
											   // described in the  file  is associated  with  the
											   // same-indexed transition time. These values serve
											   // as indices  into  an  array  of  ttinfo  structures
											   // which follows.
		char *local_time_type;
		char *temp_local_time_ptr;

//			Next in the file is an array of length [typecnt] of the following structure:
		typedef struct {
			long gmtoff;          // seconds to add to UTC
			int is_dst;           // whether tm_isdst should be set by  localtime(3)
			unsigned int abbrind; // index into the array of timezone abbreviation characters
			} ttinfo_struct;
		ttinfo_struct ttinfo;
		char *ttinfo_ptr;
		char *temp_ttinfo_ptr;
/*		   localtime(3) uses the first standard-time ttinfo structure in the  file
       (or simply the first ttinfo structure in the absence of a standard-time
       structure) if either tzh_timecnt is zero or the time argument  is  less
       than the first transition time recorded in the file.
*/

		
//			Next in the file is supposed to an array of timezone aabreviations;
//			However, it was not explicitly defined in tzfile(5), so I'm guessing...
//			typedef char[4]	tz_abbrev;
		char tz_abbrev[5];
		tz_abbrev[4] = '\0';

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
		// whether the transition times associated with local time types were specified as
		// standard  time  or  wall  clock time, and are used when a timezone file is used
		// in handling POSIX-style timezone environment variables.


//			Next in the file is an array of length [ttisgmtcnt] of the following:
//			char local_indicator;
		char *local_indicator_ptr;
		char *temp_local_indicator_ptr;
		// whether the transition times associated
		// with local time types were specified as UTC or local time, and are used
		// when  a timezone file is used in handling POSIX-style timezone environ‐
		// ment variables.

		
		// For version-2-format timezone files, the above header and data is  fol‐
		// lowed  by  a  second  header  and data, identical in format except that
		// eight bytes are used for each  transition  time  or  leap-second  time.
		// After  the  second  header and data comes a newline-enclosed, POSIX-TZ-
		// environment-variable-style string for use in  handling  instants  after
		// the  last  transition time stored in the file (with nothing between the
		// newlines if there is no POSIX representation for such instants).


// HELP - I haven't yet done this next part ...
/*
   For version-2-format timezone files, the above header and data is  fol‐
   lowed  by  a  second  header  and data, identical in format except that
   eight bytes are used for each  transition  time  or  leap-second  time.

   After  the  second  header and data comes a newline-enclosed, POSIX-TZ-
   environment-variable-style string for use in  handling  instants  after
   the  last  transition time stored in the file (with nothing between the
   newlines if there is no POSIX representation for such instants).
*/




	// Initialize malloc() pointers so we know what to free()
	transition_time = NULL;
	local_time_type = NULL;
	ttinfo_ptr = NULL;
	leapinfo_ptr = NULL;
	wall_indicator_ptr = NULL;
	local_indicator_ptr = NULL;
	

	if ( fread( &tzh, 20, 1, tz_file) != 1 ) {printf("error reading header\n"); return; }

	temp_ptr = (char *) &tzh.ttisgmtcnt;
	for (i=0;i<6;i++)
	{
		read_tz_long(temp_ptr, tz_file);
		temp_ptr = temp_ptr + 4;
	}

	printf("\nContents of file /etc/localtime\n");
	printf("%s\n",tzh.magicnumber);
	printf("number of UTC/local indicators stored in the file = %ld\n", tzh.ttisgmtcnt);
	printf("number of standard/wall indicators stored in the file = %ld\n",tzh.ttisstdcnt);
	printf("number of leap seconds for which data is stored in the file = %ld\n",tzh.leapcnt);
	printf("number of \"transition times\" for which data is stored in the file = %ld\n",tzh.timecnt);
	printf("number of \"local time types\" for which data is stored in the file (must not be zero) = %ld\n",tzh.typecnt);
	printf("number of  characters  of  \"timezone  abbreviation  strings\" stored in the file = %ld\n",tzh.charcnt);

	if (tzh.typecnt == 0)
	{
		printf("Error in file format. Zero local time types suggested.\n");
		return;
	}

	if (tzh.timecnt == 0)
	{
		printf("No transition times recorded in this file\n");
		return;
	}

	transition_time = malloc(tzh.timecnt*4);
	if (transition_time == NULL)
	{
		printf("memory allocation error - transition times");
		return;
	}

	if ( fread( transition_time, tzh.timecnt*4, 1, tz_file) != 1 )
	{
		printf("error reading transition times\n");
		free(transition_time);
		return;
	}
	

	local_time_type = malloc(tzh.timecnt);
	if (local_time_type == NULL)
	{
		printf("memory allocation error - local time type");
		free(transition_time);
		return;
	}

	if ( fread( local_time_type, tzh.timecnt, 1, tz_file) != 1 )
	{
		printf("error reading local time types\n");
		free(local_time_type);
		free(transition_time);
		return;
	}

	printf("\n List of transition times\n #    epoch  local_time_type  human-readable_time\n");
	temp_transition_time_ptr = (char *) transition_time + ((tzh.timecnt-1) * 4);
	temp_local_time_ptr = (char *) local_time_type + (tzh.timecnt-1);
	for (i=tzh.timecnt-1;i>=0; i--)
	{
		parse_tz_long( (char *) &temp_long, temp_transition_time_ptr );
		printf("%ld: %ld -- %x --- %s",i, temp_long,*temp_local_time_ptr,ctime( (const time_t *) &temp_long));
		temp_transition_time_ptr = temp_transition_time_ptr - 4;
		temp_local_time_ptr = temp_local_time_ptr - 1;
	}

	free(local_time_type);
	free(transition_time);

	ttinfo_ptr = malloc(tzh.typecnt*6);
	if (ttinfo_ptr == NULL)
	{
		printf("memory allocation error - ttinfo");
		return;
	}

	if ( fread( ttinfo_ptr, tzh.typecnt*6, 1, tz_file) != 1 )
	{
		printf("error reading ttinfo\n");
		free(ttinfo_ptr);
		return;
	}

	temp_ttinfo_ptr = ttinfo_ptr;
	printf("\nttinfo data\n  gmt_offset  HH:mm:ss  is_dst  abbr_ind\n");
	for (i=0;i<tzh.typecnt;i++)
	{
		parse_tz_long( (char *) &ttinfo.gmtoff, temp_ttinfo_ptr );
		ttinfo.is_dst = temp_ttinfo_ptr[4];
		ttinfo.abbrind = temp_ttinfo_ptr[5];
		printf("%ld: %ld     %.2ld:%.2ld:%.2ld    %d       %d\n",i, ttinfo.gmtoff,
				ttinfo.gmtoff/3600, ttinfo.gmtoff%3600/60, ttinfo.gmtoff%60, ttinfo.is_dst, ttinfo.abbrind );
		temp_ttinfo_ptr = temp_ttinfo_ptr +6;
	}
	free(ttinfo_ptr);

	printf("\ntime zone abbreviations\n");
	for (i=0;i<tzh.typecnt;i++)
	{
		if ( fread( &tz_abbrev, 4, 1, tz_file) != 1 )
		{
			printf("error reading timezone abbreviations\n");
			return;
		}
		printf("%ld: %s\n",i, tz_abbrev );
	}


	if (tzh.leapcnt != 0)
	{
		leapinfo_ptr = malloc(tzh.leapcnt*8);
		if (leapinfo_ptr == NULL)
		{
			printf("memory allocation error - leapinfo");
			return;
		}

		if ( fread( leapinfo_ptr, tzh.leapcnt*8, 1, tz_file) != 1 )
		{
			printf("error reading leapinfo\n");
			free(leapinfo_ptr);
			return;
		}

		temp_leapinfo_ptr = leapinfo_ptr;
		printf("\nleapinfo data\n  when  human_readable  num_of_seconds\n");
		for (i=0;i<tzh.leapcnt;i++)
		{
			parse_tz_long( (char *) &leapinfo.when, temp_leapinfo_ptr );
			temp_leapinfo_ptr = temp_leapinfo_ptr +4;
			parse_tz_long( (char *) &leapinfo.amt, temp_leapinfo_ptr );
			printf("%ld: %ld        %s     %ld\n",i, leapinfo.when, ctime( (const time_t *) &leapinfo.when), leapinfo.amt );
			temp_leapinfo_ptr = temp_leapinfo_ptr +4;
		}
		free(leapinfo_ptr);
	}

	if (tzh.ttisstdcnt != 0)
	{
		wall_indicator_ptr = malloc(tzh.ttisstdcnt);
		if (wall_indicator_ptr == NULL)
		{
			printf("memory allocation error - wall_indicator");
			return;
		}

		if ( fread( wall_indicator_ptr, tzh.ttisstdcnt, 1, tz_file) != 1 )
		{
			printf("error reading wall_indicator\n");
			free(wall_indicator_ptr);
			return;
		}

		temp_wall_indicator_ptr = wall_indicator_ptr;
		printf("\nwall_indicator data\n");
		for (i=0;i<tzh.ttisstdcnt;i++)
		{
			printf("%ld: %x\n",i, *temp_wall_indicator_ptr);
			temp_wall_indicator_ptr = temp_wall_indicator_ptr + 1;
		}
		free(wall_indicator_ptr);
	}

	if (tzh.ttisgmtcnt != 0)
	{
		local_indicator_ptr = malloc(tzh.ttisgmtcnt);
		if (local_indicator_ptr == NULL)
		{
			printf("memory allocation error - local_indicator");
			return;
		}

		if ( fread( local_indicator_ptr, tzh.ttisgmtcnt, 1, tz_file) != 1 )
		{
			printf("error reading local_indicator\n");
			free(local_indicator_ptr);
			return;
		}

		temp_local_indicator_ptr = local_indicator_ptr;
		printf("\nlocal_indicator data\n");
		for (i=0;i<tzh.ttisgmtcnt;i++)
		{
			printf("%ld: %x\n",i, *temp_local_indicator_ptr);
			temp_local_indicator_ptr = temp_local_indicator_ptr + 1;
		}
		free(local_indicator_ptr);
	}

	if (tzh.magicnumber[4] == 50) printf("\nNotice: This file is in version 2 of the format.\n\
At this point in the file, the data basically repeats;\n\
However, there is a final line for handling times beyond\n\
the last recorded transition. ... TBD ... well, maybe ....\n");
	return;
}



void timezone_display_local_info()
{
	FILE *tz_file;

	char *tza;
	tzset();
	printf("Environmental variables:\n tzname = %s, timezone =  %ld:%ld:%ld, daylight = %d, TZ = ",
			*tzname, timezone/3600, timezone%3600/60, timezone%60, daylight);
	tza = getenv("TZ");
	if (tza== NULL) printf("null\n");
	else printf("%s\n",tza);

	tz_file = fopen("/etc/localtime", "rb");
	if (tz_file == NULL)
	{
		error(errno,0,"tz file /etc/localtime not found\n");
		return;
	}

	timezone_display_tzinfo_file_data_v2( tz_file );

	fclose(tz_file);
}
