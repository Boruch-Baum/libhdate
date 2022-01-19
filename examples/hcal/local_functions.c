/** local_functions.c            https://libhdate.sourceforge.net
 * a collection of functions in support of both hcal.c and hdate.c
 * hcal.c  Hebrew calendar              (part of package libhdate)
 * hdate.c Hebrew date/times information(part of package libhdate)
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` local_functions.c -o local_functions
 *
 *  Copyright (C) 2012-2018, 2022 Boruch Baum  <boruch_baum@gmx.com>
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

/**************************************************
*   functions to support hcal and hdate
**************************************************/

#include <hdate.h>    // For hebrew date (gcc -I ../../src)
#include <support.h>  // libhdate general macros, including for gettext
#include <stdlib.h>    // For atoi, malloc
#include <error.h>    // For error
#include <errno.h>    // For errno
#include <time.h>    // For time
#include <ctype.h>    // for isprint
#include <fnmatch.h>  // For fnmatch
#include <string.h>    // For mempcpy
#include <getopt.h>    // For optarg, optind
#include <stdio.h>    // For printf, fopen, fclose, fprintf, snprintf. FILE
#include <pwd.h>    // for get pwuid
#include <unistd.h>    // for getuid
#include <sys/stat.h>  // for mkdir
#include <sys/types.h>  // for mkdir

#include "local_functions.h"  // for macro definitions used by other programs
#include "timezone_functions.h" // for get_lat_lon_from_zonetab_file, read_sys_tz_string_from_file
#include <zdump3.h>        // for zdump

#define EXIT_CODE_BAD_PARMS  1

// Defined in header file
// #define SECONDS_PER_DAY 60*60*24
// #define MIN_CANDLES_MINUTES 15
// #define DEFAULT_CANDLES_MINUTES 20
// #define MAX_CANDLES_MINUTES 90
// #define MIN_MOTZASH_MINUTES 20
// #define DEFAULT_MOTZASH_MINUTES 72
// #define MAX_MOTZASH_MINUTES 90



#define USING_SYSTEM_TZ    -1
#define NOT_USING_SYSTEM_TZ 0
#define TZ_MAX_VALID       14
#define TZ_MIN_VALID      -11

static char * error_text     = N_("error");
static char * latitude_text  = N_("l (latitude)");
static char * longitude_text = N_("L (Longitude)");
static char * equator_text   = N_("equator");
static char * lon_midpoint_text = N_("midpoint of theoretical timezone");
static char * absolute_utc_offset_text = N_("absolute UTC offset entered");
static char * no_timezone_entered_text = N_("no timezone entered");

/************************************************************
* begin - error message functions
************************************************************/
void parm_error ( const char *parm_name )
{
  error(0,0,"%s: %s %s %s",error_text,
      N_("parameter"), parm_name,
      N_("is non-numeric or out of bounds"));
}

void parm_missing_error ( const char *parm_name )
{
  error(0,0,"%s: %s %s %s",error_text,
      N_("option"), parm_name, N_("missing parameter"));
}

void option_unknown_error ( const char *parm_name )
{
  error(0,0,"%s: %s%s",error_text,
      N_("unsupported option -"), parm_name);
}


void alert_tz( int tz )
{
  error(0,0,"%s: %s, %+d:%d UTC",
      N_("ALERT: time zone not entered, using current system UTC offset"),
      *tzname, tz/60, tz%60);
}

void alert_timezone( const char* tz_name_ptr )
{
  error(0,0,"%s: %s",
      N_("ALERT: time zone not entered, using system local time zone"),
      tz_name_ptr);
}

void alert_using_utc_offset( const int utc_offset )
{
  error(0,0,"%s %+.2f %s", N_("ALERT: time zone not entered, guessing UTC offset"), (double) utc_offset/60, N_("hours, based upon longitude") );
}

void alert_missing_coordinate( char* parm_name, double val, char* tz_name )
{
  error(0,0,"%s",
      N_("ALERT: coordinate not entered or invalid..."));
  error(0,0,"%s %s %s %s: %lf",
      N_("ALERT: guessing"), parm_name, N_("of"), tz_name, val );
}

void alert_delta_coordinate( char* coordinate, double hdate_val , double zonetab_val)
{
  error(0,0,"%s %s %+lf %s %+lf",
      N_("ALERT: the input"), coordinate, hdate_val,
      N_("is at great variance from the chosen time zone default value"), zonetab_val );
}

void alert_not_dst_aware( char* reason_text)
{
  error(0,0,"%s ( %s )",
      N_("ALERT: daylight savings time can not be determined"), reason_text );
}


void parm_invalid_error( char *parm_name )
{
  error(0,0,"%s: %s %s %s",error_text,
      N_("option"), parm_name, N_("is not a valid option"));
}

void parm_mismatch_error()
{
  error(0,0,"%s: %s",error_text,
      N_("month and year parameters mismatched (Hebrew/gregorian mix)"));
}

void config_file_create_error( int error_code, char* config_file_path )
{
  error(0, error_code, "%s: %s",
      N_("failure attempting to create config file"), config_file_path);
}

/************************************************************
* end - error message functions
************************************************************/




/************************************************************
* set default location
************************************************************/
int set_default_location( const int tz, char** tz_name_ptr,
              double *lat, double *lon)
{

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
/**-10 **/  case -600:  *lat =  21.30    ;  *lon = 157.82    ;  *tz_name_ptr = N_("Honolulu");break;
/** -8 **/  case -480:  *lat =  34.052222;  *lon =-118.242778;  *tz_name_ptr = N_("America/Los_Angeles"); break;
/** -6 **/  case -360:  *lat =  19.400000;  *lon = -99.150000;  *tz_name_ptr = N_("America/Mexico_City"); break;
/** -5 **/  case -300:  *lat =  40.714167;  *lon = -74.638800;  *tz_name_ptr = N_("America/New_York"); break;
/** -5 **///case -300:  *lat =  43.71    ;  *lon = -79.43    ;  *tz_name_ptr = N_("Toronto"); break;
/** -4.5 **/case -270:  *lat =  10.500000;  *lon = -66.933333;  *tz_name_ptr = N_("America/Caracas"); break;
/** -3 **///case -180:  *lat = -23.55    ;  *lon = -46.61    ;  *tz_name_ptr = N_("Sao Paolo"); break;
/** -3 **/  case -180:  *lat = -34.600000;  *lon = -58.450000;  *tz_name_ptr = N_("America/Argentina/Buenos_Aires"); break;
/**  0 **/  case    0:  *lat =  51.508333;  *lon =   0.125278;  *tz_name_ptr = N_("Europe/London"); break;
/**  1 **/  case   60:  *lat =  48.866667;  *lon =   2.333333;  *tz_name_ptr = N_("Europe/Paris"); break;
/**  2 **/  case  120:  *lat =  31.766667;  *lon =  35.233333;  *tz_name_ptr = N_("Asia/Jerusalem"); break;
/**  3.5 **/case  210:  *lat =  35.666667;  *lon =  51.433333;  *tz_name_ptr = N_("Asia/Tehran"); break;
/**  4 **/  case  240:  *lat =  55.750000;  *lon =  37.583333;  *tz_name_ptr = N_("Europe/Moscow"); break;
/**  5 **/  case  300:  *lat =  41.333333;  *lon =  69.300000;  *tz_name_ptr = N_("Asia/Tashkent"); break;
/**  5.5 **/case  330:  *lat =  22.533333;  *lon =  88.366667;  *tz_name_ptr = N_("Asia/Kolkata"); break;
/**  8 **/  case  480:  *lat =  31.233333;  *lon = 121.466667;  *tz_name_ptr = N_("Asia/Shanghai"); break;
/**  8 **///case  480:  *lat =  39.90    ;  *lon = 116.38    ;  *tz_name_ptr = N_("Asia/Beijing"); break;
/**  8 **///case  480:  *lat =  22.26    ;  *lon = 114.15    ;  *tz_name_ptr = N_("Hong Kong");break;
/** 10 **/  case  600:  *lat = -33.866667;  *lon = 151.216667;  *tz_name_ptr = N_("Australia/Sydney");break;
  default:  // .25 = (15 degrees) / 60 [because tz is in minutes, not hours]
        *lat =   0.0;  *lon =  tz * .25;
        return FALSE; break;
  }
  return TRUE;
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
// FIXME - logic to recognize build for big-endian or little-endian architecture
int revstr( char *source, const size_t source_len)
{
  #define H_CHAR_WIDTH 2
  int retval = 0;

#define DEBUG 0
#if DEBUG
printf("\nrevstr: entry: sourcelen = %ld, source = %s\n",source_len, source);
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
printf("\nrevstr: before free(tempbuff): sourcelen = %ld, source = %s\n",source_len, source);
#endif
  free(temp_buff);
  return retval;
}


/************************************************************
* parse coordinate
*
* presumes commandline options were parsed using getopt.
* Upon success, returns the floating point value
*               of the coordinate in *coordinate
* Upon failure, returns BAD_COORDINATE in *coordinate
*
* type_flag (1-latitude, 2=longitude)
************************************************************/
int parse_coordinate( const int type_flag, char *input_string,
            double *coordinate)
{
  char* lattitude_globs[] = {
    "?([nNsS+-])[[:digit:]]?([[:digit:]]?([[:digit:]]))?(.+([[:digit:]]))",
    "[[:digit:]]?([[:digit:]]?([[:digit:]]))?(.+([[:digit:]]))?([nNsS])",
    "?([nNsS+-])[[:digit:]]?([[:digit:]]?([[:digit:]]))?([':\"]?([012345])[[:digit:]]?([:'\"]?([012345])[[:digit:]]))",
    "[[:digit:]]?([[:digit:]]?([[:digit:]]))?([':\"]?([012345])[[:digit:]]?([:'\"]?([012345])[[:digit:]]))?([nNsS])"
    };
  char* longitude_globs[] = {
    "?([eEwW+-])[[:digit:]]?([[:digit:]]?([[:digit:]]))?(.+([[:digit:]]))",
    "[[:digit:]]?([[:digit:]]?([[:digit:]]))?(.+([[:digit:]]))?([eEwW])",
    "?([eEwW+-])[[:digit:]]?([[:digit:]]?([[:digit:]]))?([':\"]?([012345])[[:digit:]]?([:'\"]?([012345])[[:digit:]]))",
    "[[:digit:]]?([[:digit:]]?([[:digit:]]))?([':\"]?([012345])[[:digit:]]?([:'\"]?([012345])[[:digit:]]))?([eEwW])"
    };
  int glob_type_found;
  #define GLOB_NOT_FOUND 4
  char* seconds;
  double fractional_part = 0;
  int direction_modifier = 1;

  /************************************************************
  * input_string should hold the value to parse
  * It must exist and be numeric
  * we don't want confuse it with the next option of for "-x"
  ************************************************************/
  if  (!input_string)
  {
    if    (type_flag == 1) parm_missing_error(latitude_text);
    else if (type_flag == 2) parm_missing_error(N_("L (Longitue)"));
    *coordinate = BAD_COORDINATE;
    return TRUE; // error_detected = TRUE; ie failure
  }

  /************************************************************
  * parse latitude
  ************************************************************/
  if  (type_flag == 1)
  {
    for ( glob_type_found=0; glob_type_found<GLOB_NOT_FOUND; glob_type_found++)
    {
      if(  (fnmatch( lattitude_globs[glob_type_found], input_string, FNM_EXTMATCH)==0)) break;
    }
    if (glob_type_found == GLOB_NOT_FOUND)
    {
      parm_error(latitude_text);
      *coordinate = BAD_COORDINATE;
      return TRUE; // error_detected = TRUE; ie failure
    }
    if ( strpbrk(input_string, "sS") != NULL ) direction_modifier = -1;
    if (glob_type_found < 2) // decimal format
    {
      *coordinate = (double) atof( strtok( input_string, "nNsS"));
    }
    else // degrees, minutes, seconds format
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
    if(  (*coordinate > -90) && (*coordinate < 90) )
    {
      return FALSE; // error_detected = FALSE; ie. success
    }
    parm_error(latitude_text);
    *coordinate = BAD_COORDINATE;
    return TRUE; // error_detected = TRUE; ie failure
  }


  /************************************************************
  * parse longitude
  ************************************************************/
  if  (type_flag == 2)
  {
    for ( glob_type_found=0; glob_type_found<GLOB_NOT_FOUND; glob_type_found++)
    {
      if(  (fnmatch( longitude_globs[glob_type_found], input_string, FNM_EXTMATCH)==0)) break;
    }
    if (glob_type_found == GLOB_NOT_FOUND)
    {
      parm_error(longitude_text);
      *coordinate = BAD_COORDINATE;
      return TRUE; // error_detected = TRUE; ie failure
    }
    if ( strpbrk(input_string, "wW") != NULL ) direction_modifier = -1;
    if (glob_type_found < 2) // decimal format
    {
      *coordinate = (double) atof( strtok( input_string, "eEwW"));
    }
    else // degrees, minutes, seconds format
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
    if(  (*coordinate > -180) && (*coordinate < 180) )
    {
      return FALSE; // error_detected = FALSE; ie. success
    }
    parm_error(longitude_text);
    *coordinate = BAD_COORDINATE;
    return TRUE; // error_detected = TRUE; ie failure
  }

printf("INTERNAL ERROR: coordinate parse\n");
*coordinate = BAD_COORDINATE;
return TRUE; // error_detected = TRUE; ie failure
}

/************************************************************
* parse_timezone_numeric
*
* presumes commandline options were parsed using getopt.
* timezone is returned in minutes, not hours
************************************************************/
int parse_timezone_numeric( char *input_string, int *tz)
{
  #define TIMEZONE_GLOB_DECIMAL  "?([+-])?(1)[[:digit:]]?(.+([[:digit:]]))"
  #define TIMEZONE_GLOB_DM    "?([+-])?(1)[[:digit:]]?([:'\"]+([[:digit:]]))"

  int fractional_part;

  *tz = BAD_TIMEZONE;
  if (fnmatch(TIMEZONE_GLOB_DECIMAL, input_string, FNM_EXTMATCH)==0)
  {
    *tz = (int) ( atof (input_string) * 60 );
  }
  else if (fnmatch(TIMEZONE_GLOB_DM, input_string, FNM_EXTMATCH)==0)
  {
      *tz = atoi( strtok( input_string, ":'\"")) * 60;
      fractional_part = atoi( strtok( NULL, ":'\""));
      if (*tz < 0)  *tz = *tz - fractional_part;
      else      *tz = *tz + fractional_part;
  }

  if( (*tz < (TZ_MIN_VALID*60) ) || (*tz > (TZ_MAX_VALID*60) ) )
    return FALSE;

  return TRUE;
}



/************************************************************
* parse_timezone_alpha
*
* Look for a timezone name (eg. America/New York)
************************************************************/
int parse_timezone_alpha(const char* search_string, char** result_str, int* tz, double* tz_lat, double* tz_lon)
{
  const int quiet_alerts = FALSE;
  // TODO - check that these three initializations aren't redundant
  *tz_lat = BAD_COORDINATE;
  *tz_lon = BAD_COORDINATE;
  *tz = BAD_TIMEZONE; // check if redundant or necessary or desirable
  *result_str = NULL; // check if redundant
  if (get_lat_lon_from_zonetab_file( search_string, result_str, tz_lat, tz_lon, quiet_alerts ))
    return TRUE;
  // else handle possibility of timezone name not in zonetab file
  // else handle possibility of timezone abbreviations
  return FALSE;
}

/************************************************************
* parse_epoch_value
*
* returns 0 = no error detected
*         1 = error detected
*
************************************************************/
int parse_epoch_value( const char* epoch_string, time_t* epoch_val, int* epoch_parm_received )
{
  if (fnmatch( "?(@)?(-)*([[:digit:]])", epoch_string, FNM_EXTMATCH) == 0)
  {
    int i = 0;
    if (epoch_string[0] == '@') i = 1;
    {
      if      ( sizeof(time_t) == sizeof(int) )  *epoch_val = atoi(&epoch_string[i]);
      else if ( sizeof(time_t) == sizeof(long) ) *epoch_val = atol(&epoch_string[i]);
      else if ( sizeof(time_t) == sizeof(long long) ) *epoch_val = atoll(&epoch_string[i]);
      else
      {
        error(0,0,"%s: %s", error_text, N_("internal size mismatch, parameter '--epoch' being ignored"));
        return 1;
      }
    }
  }
  else
  {
    parm_error("--epoch"); // do not gettext!
    return 1;
  }
  *epoch_parm_received = TRUE;
  return 0;
}



/************************************************************
* process location parameters - lat, lon, tz
*  Originally, this was only for hdate, now also for hcal.
*  Originally, we did this only if we needed the information.
*  Now, because of sunset-awareness, we always perform these checks.
*  if ( (opt.sun) || (opt.times) || (opt.candles) || (opt.havdalah) )
*  originally, we required lat/lon pairing, now we alert and guess
*
* Formerly returned "input_info", the state of the information parsed.
*  See below for the #defines. The idea was that the return value indicates
*  whether timezone information should be picked up from /etc/localtime or
*  from the file indicated by tz_name_ptr
*
* -z number (absolute over-ride of system time zone)
*           longitude mis-sync -> just alert
*           no longitude -> guess (including zone.tab for current)
*           alert no dst awareness
* -z name   (absolute over-ride of system time zone)
*           longitude mis-sync -> just alert
*           latitude mis-sync (re:zone.tab) - just alert
*           no longitude -> use zone.tab entry
* -z null   system time zone (/etc/timezone)
*           system tzfile    (/etc/localtime)
*           longitude mis-sync -> just alert
*           latitude mis-sync (re:zone.tab) - just alert
*           no longitude -> use zone.tab entry
*
************************************************************/
void process_location_parms( double *lat, double *lon, double tz_lon,
            int *tz, char* tz_name_in, char** tz_name_out,
            const time_t start_time, const time_t end_time,
            int * num_dst_entries_ptr, void** returned_dst_data,
            const int quiet_alerts )
{
  char* zonetab_name  = NULL;
  double guessed_lat = BAD_COORDINATE;
  double guessed_lon = BAD_COORDINATE;
  int    guessed_tz  = BAD_TIMEZONE;
  int    guess_found = FALSE;
  char*  guess_name  = NULL;

  #define DELTA_LONGITUDE 45
  #define DELTA_LATITUDE  60

  #define HDVL_NO_INFO    1
  #define HDVL_TZ_INFO    0
  #define HDVL_NAME_INFO  3
  #define HDVL_LOCAL_INFO 2
  #define HDVL_ALL_INFO  4
  int    input_info = HDVL_ALL_INFO;

  *tz_name_out = NULL;
  if (*tz != BAD_TIMEZONE) input_info = HDVL_TZ_INFO;
  else
  {
    if (tz_name_in==NULL)
    {
      *tz_name_out = read_sys_tz_string_from_file();
      // now *tz_name_out must be free()d
      if (*lon == BAD_COORDINATE)
      {
        if (*tz_name_out == NULL) input_info = HDVL_LOCAL_INFO;
        else input_info = HDVL_NAME_INFO;
      }
      else // (*lon != BAD_COORDINATE)
      {
        if ( get_lat_lon_from_zonetab_file( *tz_name_out, &zonetab_name,
              &guessed_lat, &guessed_lon, quiet_alerts ) )
        {
          if (abs(*lon - guessed_lon) > DELTA_LONGITUDE)
          {
            guessed_tz = (((int) *lon) / 15 ) * 60; // we use tz in minutes
            *tz = guessed_tz;
            guessed_lat = 0; // equator
            free(*tz_name_out);
            *tz_name_out = NULL;
            input_info = HDVL_TZ_INFO;
          }
          else input_info = HDVL_NAME_INFO;
        }
        else
        {
          // NOTE: if we reached here via case HDVL_NO_INFO:
          // something very wrong has happened
          // ie. /etc/timezone had a value for which no entry
          // exists in zonetab file, so no default lat/lon available
          // however,  there may be a valid tzif file
          if (!quiet_alerts) alert_timezone( tz_name_in );
          input_info = HDVL_LOCAL_INFO;
        }
      }
    }
    else
    {
      *tz_name_out = tz_name_in;
      if ( (*lat == BAD_COORDINATE) || (*lon == BAD_COORDINATE) )
        input_info = HDVL_NAME_INFO;
    }
  }

  if (input_info == HDVL_NAME_INFO)
  {
    if( get_lat_lon_from_zonetab_file( *tz_name_out, &zonetab_name,
              &guessed_lat, &guessed_lon, quiet_alerts ) )
    {
      guess_found = TRUE;
      // But what if zonetab_tz_name_ptr isn't identical to tz_name_ptr?
      // free(zonetab_tz_name_ptr);
      *tz_name_out = zonetab_name;
      if (!quiet_alerts) alert_timezone(zonetab_name);
    }
    else
    {
      // NOTE: if we reached here via case HDVL_NO_INFO:
      // something very wrong has happened
      // ie. /etc/timezone had a value for which no entry
      // exists in zonetab file, so no default lat/lon available
      // however,  there may be a valid tzif file
      if (!quiet_alerts) alert_timezone( tz_name_in );
      input_info = HDVL_LOCAL_INFO;
    }
  }

  if (input_info == HDVL_LOCAL_INFO)
  {
    // system timezone is in seconds, but we deal in minutes
    // not sure if this tzset() is still necessary
    tzset();
    *tz = timezone / (-60);
    // double-check that when using non-system default timezone that
    // *tz is set properly
    if (!quiet_alerts)
    {
      alert_tz( *tz );
      alert_not_dst_aware( no_timezone_entered_text );
    }
    // try reading tzif file from /etc/localtime
    if ( (*lat==BAD_COORDINATE) || (*lon==BAD_COORDINATE) )
    {
      guess_found = set_default_location( *tz, tz_name_out,
                        &guessed_lat, &guessed_lon );
    }
  }

  if (input_info == HDVL_TZ_INFO)
  {
    if (guessed_tz != BAD_TIMEZONE)
    {
      alert_using_utc_offset( guessed_tz );
      alert_not_dst_aware( no_timezone_entered_text );
    }
    else
    {
      alert_not_dst_aware( absolute_utc_offset_text );
      if ( (*lat==BAD_COORDINATE) || (*lon==BAD_COORDINATE) )
        guess_found = set_default_location( *tz, tz_name_out,
                        &guessed_lat, &guessed_lon );
    }
  }


  if (*lon==BAD_COORDINATE)
  {
    *lon = guessed_lon;
    if (!quiet_alerts)
    {
      if (guess_found) guess_name = *tz_name_out;
      else             guess_name = lon_midpoint_text;
      alert_missing_coordinate(longitude_text, *lon, guess_name );
    }
  }
//  else if ( (!quiet_alerts) && (guessed_tz == BAD_TIMEZONE) && (abs(*lon - guessed_lon) > DELTA_LONGITUDE) )
  else if ( (!quiet_alerts) && (guessed_tz == BAD_TIMEZONE) )
  {
    if (guess_found) tz_lon = guessed_lon;
    if (abs(*lon - tz_lon) > DELTA_LONGITUDE)
      alert_delta_coordinate(longitude_text, *lon, tz_lon);
  }

  if (*lat==BAD_COORDINATE)
  {
    *lat = guessed_lat;
    if (!quiet_alerts)
    {
      if (guess_found) guess_name = *tz_name_out;
      else             guess_name = equator_text;
      alert_missing_coordinate(latitude_text, *lat, guess_name );
    }
  }


  // Get timezone and DST information
  if (input_info != HDVL_TZ_INFO)
  {
    int check_localtime = FALSE;
    if (*tz_name_out != NULL)
    {
      if ( zdump( *tz_name_out, start_time, end_time, num_dst_entries_ptr, returned_dst_data) !=0 )
         check_localtime = TRUE;
    }
    if ((*tz_name_out == NULL) || (check_localtime == TRUE))
    {
      if ( ( zdump( "localtime", start_time, end_time, num_dst_entries_ptr, returned_dst_data) !=0 )    &&
         ( zdump( "Asia/Jerusalem", start_time, end_time, num_dst_entries_ptr, returned_dst_data) !=0 ) &&
         ( zdump( "America/New York", start_time, end_time, num_dst_entries_ptr, returned_dst_data) !=0 ) )
      {
        error(0,0,"%s",N_("unable to parse timezone or dalight savings time data;\n      using Jerusalem Standard Time"));
        *tz = JERUSALEM_STANDARD_TIME_IN_MINUTES;
      }
      else
      {
        // TODO - each of the zdump invocations above needs a call to error to alert user of state
      }
    }
  }
  return;
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
    if (year > HDATE_HEB_YR_LOWER_BOUND)
    {
      if (year > HDATE_HEB_YR_UPPER_BOUND) return FALSE;
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
    if (year > HDATE_HEB_YR_LOWER_BOUND)
    {
      if (year > HDATE_HEB_YR_UPPER_BOUND) return FALSE;
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

  // revisit the necessity of this; also the validation of gyears
  // (validate_hdate(CHECK_YEAR_PARM
  // see snippet in custom_days.c:read_custom_days_file
  case CHECK_YEAR_PARM:
    if (year > HDATE_HEB_YR_UPPER_BOUND) return FALSE;
    return TRUE;
    break;
  }
  return FALSE;
}



/************************************************************
* Greeting message to new version
************************************************************/
void greetings_to_version_18()
{
  printf("%s\n",N_("\
This seems to be to be your first time using this version (1.8).\n\
Please read the new documentation in the man page and config files.\n\
Press <enter> to continue."));
  getchar();
}



/************************************************************
* assemnble_config_file_pathname
************************************************************/
char* assemnble_config_file_pathname (  const char* config_dir_name,
                    const char* config_file_name,
                    const int quiet_alerts )
{
  size_t path_len;

  char* config_home_path_name = "";
  char* config_sub_path_name = "";
  char* config_file_path = "";

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
  return config_file_path;
}


/****************************************************
* get_config_file
*
*   read, create, read a config file for hcal/hdate
****************************************************/
// TODO - report errors at every step!
int get_config_file(  const char* config_dir_name,
            const char* config_file_name,
            const char* default_config_file_text,
            const int quiet_alerts,
            FILE** config_file)
{
  *config_file = NULL;
  char *config_file_path = NULL;
  char *last_slash_location = NULL;

  config_file_path = assemnble_config_file_pathname (
                  config_dir_name, config_file_name,
                  quiet_alerts );
  if (config_file_path == NULL) return FALSE;
  *config_file = fopen(config_file_path, "r");
  if (*config_file == NULL)
  {
    if (errno != ENOENT) { free(config_file_path); return FALSE; };
    last_slash_location = strrchr(config_file_path, '/');
    if (last_slash_location ==  NULL) { free(config_file_path); return FALSE; };
    *last_slash_location = '\0';
    if ((mkdir(config_file_path, (mode_t) 0700) != 0) && (errno != EEXIST)) { free(config_file_path); return FALSE; };
    *last_slash_location = '/';
    greetings_to_version_18();
    if (!quiet_alerts) printf("%s\n", N_("attempting to create a config file ..."));
    *config_file = fopen(config_file_path, "a+");
    if ( (*config_file == NULL) ||
       ( fprintf(*config_file, "%s", default_config_file_text) < 0 ) )

    {
      if (!quiet_alerts) config_file_create_error(errno, config_file_path);
      { free(config_file_path); return FALSE; };
    }
    if (!quiet_alerts) printf("%s: %s\n", N_("succeeded creating config file"), config_file_path);
    if ( fseek(*config_file, 0, SEEK_SET) != 0 ) { free(config_file_path); return FALSE; }
  }
  free(config_file_path);
  return TRUE;
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
*  -1 means the menu line string has been completely parsed.
*  Otherwise, the return value should be the short option character
*  found, or in the case of a long option found, 0.
*  This should sound suspiciously similar to the return values issued
*   by getopt_long().
***********************************************************************/
int menu_item_parse(char* menuptr, size_t menu_len, int *menu_index,
          char** optptr, const char* short_options,
          const struct option *long_options, int *long_option_index,
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
          parm_invalid_error(*optptr);
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
          parm_invalid_error(*optptr);
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

/**************************************************
*
*  Get epoch time, presuming midnight of day yy mm dd
* This is for comparing with DST transition values
*
*************************************************/
// This may need to be adjusted, and have an additional
// parameter once option --emesh --erev --eve is coded
void get_epoch_time_range( time_t *retval_start, time_t *retval_end,
          const char * tz_string, const int tz,
          const int gyear0, const int gmonth0, const int gday0,
          const int gyear1, const int gmonth1, const int gday1
          )
{
  size_t tz_str_len = 0;
  char * original_system_timezone_string = NULL;
  char * tz_rule_string = NULL;

  struct tm tmx;
  tmx.tm_sec = 0;
  tmx.tm_min = 0;
  tmx.tm_hour = 0;
  tmx.tm_mday = gday0;
  tmx.tm_mon =  gmonth0 - 1;
  tmx.tm_year = gyear0 - 1900;
  tmx.tm_wday = 0;
  tmx.tm_yday = 0;
  tmx.tm_isdst = 0;

  original_system_timezone_string = getenv("TZ");
  if (tz != BAD_TIMEZONE)
  {
    // We have an absolute timezone value adjustment, so we don't
    // ever use the tz database or have dst transitions.
    // per man 3 tzset "If the TZ variable does appear in the environment
    // but its value is empty ... (UTC) is used"
    setenv("TZ", "", 1);
    tzset();
    *retval_start = mktime(&tmx);
    tmx.tm_mday = gday1;
    tmx.tm_mon =  gmonth1 - 1;
    tmx.tm_year = gyear1 - 1900;
    *retval_end = mktime(&tmx);
    // TODO - verify that tz is in seconds, not minutes!
    *retval_start = *retval_start + tz;
    *retval_end = *retval_end + tz - 1;
    if (original_system_timezone_string) setenv("TZ", original_system_timezone_string, 1);
    tzset();
    return;
  }
  if (!tz_string) return ;// ie. null, 0x0

  tz_str_len = strlen(tz_string);
  tz_rule_string = (char*) malloc(tz_str_len+2);
  if (tz_rule_string == NULL) {error(0,0,"get_epoch_time: malloc failure"); exit(0);};
  memset(tz_rule_string,':',1);
  strncpy(tz_rule_string + 1, tz_string, tz_str_len + 1);

  setenv("TZ", tz_rule_string, 1);
  tzset();
  *retval_start = mktime(&tmx);
  tmx.tm_mday = gday1;
  tmx.tm_mon =  gmonth1 - 1;
  tmx.tm_year = gyear1 -1900;
  *retval_end = mktime(&tmx);

  free(tz_rule_string);
  if (original_system_timezone_string) setenv("TZ", original_system_timezone_string, 1);
  tzset();
  return;
}
