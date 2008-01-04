(*  libhdate
   Copyright  2004-2005  Yaacov Zamir <kzamir@walla.co.il>
	 Copyright  2005       Ido Kanner   <idokan@gmail.com>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
	 
History:
     25/11/2005 - Added some constants and fixed comments according the orginal API file
     13/03/2005 - Bug Fixed Added {$PACKRECORDS C}
  10-11/03/2005 - Initial Translation of version 0.31.0
*)
{$MODE OBJFPC}{$PACKRECORDS C}

unit hdate;

interface
uses cTypes;

const
  LIBHDATE_LIBRARY_NAME = 'hdate';

(** @def HDATE_DIASPORA_FLAG
  @brief use diaspora dates and holydays flag
*)
  HDATE_DIASPORA_FLAG = -1;

(** @def HDATE_ISRAEL_FLAG
  @brief use israel dates and holydays flag
*)
  HDATE_ISRAEL_FLAG   = 0;

(** @def HDATE_SHORT_FLAG
  @brief use short strings flag
*)
  HDATE_SHORT_FLAG    = -1;

(** @def HDATE_LONG_FLAG
  @brief use long strings flag
*)
  HDATE_LONG_FLAG     = 0;


{ Base structore for hebrew dates }

type
  Phdate_struct = ^Thdate_struct;
	Thdate_struct = record
                   hd_day          : cInt; //The number of day in the hebrew month (1..31).
                   hd_mon          : cInt; //The number of the hebrew month 1..14 (1 - tishre, 13 - adar 1, 14 - adar 2).
                   hd_year         : cInt; //The number of the hebrew year.
                   gd_day          : cInt; //The number of the day in the month. (1..31)
                   gd_mon          : cInt; //The number of the month 1..12 (1 - jan).
                   gd_year         : cInt; //The number of the year.
                   hd_dw           : cInt; //The day of the week 1..7 (1 - sunday).
                   hd_size_of_year : cInt; //The length of the year in days.
                   hd_new_year_dw  : cInt; //The week day of Hebrew new year.
                   hd_year_type    : cInt; //The number type of year.
                   hd_jd           : cInt; //The Julian day number
                   hd_days         : cInt; //The number of days passed since 1 tishrey
                   hd_weeks        : cInt; //The number of weeks passed since 1 tishrey
                  end;
(********************************************************************************)
(********************************************************************************)

(**
 @brief compute date structure from the Gregorian date

 @param h pointer this hdate struct.
 @param d Day of month 1..31
 @param m Month 1..12 
 @param y Year in 4 digits e.g. 2001
 @return pointer to this hdate struct
 *)
 
 function hdate_set_gdate (h : Phdate_struct; d, m, y : cInt) : Phdate_struct;
    cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_set_gdate';

(**
 @brief compute date structure from the Hebrew date

 @param h pointer this hdate struct.
 @param d Day of month 1..31
 @param m Month 1..14 , (13 - Adar 1, 14 - Adar 2)
 @param y Year in 4 digits e.g. 5731
 @return pointer to this hdate struct
 *)

function hdate_set_hdate (h : Phdate_struct; d, m, y : cInt) : Phdate_struct;
    cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_set_hdate';

(**
 @brief compute date structure from the Julian day

 @param h pointer this hdate struct.
 @param jd the julian day number.
 @return pointer to this hdate struct
 *)
function hdate_set_jd (h : Phdate_struct; jd : cInt) : Phdate_struct;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_set_jd';
     
(*************************************************************)
(*************************************************************)

(**
 @brief get formated hebrew date.

 return the short ( e.g. "1 Tishrey" ) or 
 long (e.g. "Tuesday 18 Tishrey 5763 Hol hamoed Sukot" ) formated date.

 @param h pointer this hdate struct.
 @param diaspora if true give diaspora holydays.
 @param s short flag.
 @return a static string of foramted date
*)
function hdate_get_format_date (h : Phdate_struct; diaspora, s : cInt) : PChar;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_format_date';

(**
 @brief get the number of hebrew parasha.

 @param h pointer this hdate struct.
 @param diaspora if true give diaspora readings
 @return the number of parasha 1. Bereshit etc..
   (55 trow 61 are joined strings e.g. Vayakhel Pekudei)
*)
function hdate_get_parasha (h : Phdate_struct; diaspora : cInt) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_parasha';

(**
 @brief get the number of hebrew holyday.

 @param h pointer this hdate struct.
 @param diaspora if true give diaspora holydays
 @return the number of holyday.
*)
function hdate_get_holyday (h : Phdate_struct; diaspora : cInt) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_holyday';

(*************************************************************)
(*************************************************************)

(**
 @brief convert an integer to hebrew string. 
 
 @param n The int to convert
 @return a static string of the hebrew number UTF-8 (logical)
 @attention ( 0 < n < 10000)
*)
function hdate_get_int_string (n : cInt) : PChar;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_int_string';

(**
 @brief get name of week day.

 @param day The number of the day 1..7 (1 - sun).
 @param s short flag 
   true - returns a short string: sun, false returns: sunday.
 @return a static string of the day of the week
*)
function hdate_get_day_string (day : cInt; s : cInt) : PChar;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_day_string';

(**
 @brief name of month.

 @param month the number of the month 1..12 (1 - jan).
 @param s short flag.
 @return a static string of month name
*)
function hdate_get_month_string (month : cInt; s : cInt) : PChar;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_month_string';

(**
 @brief name of hebrew month.

 @param month the number of the month 1..14 
   (1 - tishre, 13 - adar 1, 14 - adar 2).
 @param s short flag.
 @return a static string of month name
*)
function hdate_get_hebrew_month_string (month : cInt; s : cInt) : PChar;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_hebrew_month_string';

(**
 @brief name of hebrew holyday.

 @param holyday the holyday number.
 @param s short flag.
 @return a static string of holyday name
*)
function hdate_get_holyday_string (holyday, s : cInt) : PChar;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_holyday_string';

(**
 @brief name of parasha

 @param parasha the number of parasha 1-Bereshit
   (55 trow 61 are joined strings e.g. Vayakhel Pekudei)
 @param s short flag.
 @return a static string of parasha name
*)
function hdate_get_parasha_string (parasha, s : cInt) : PChar;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_parasha_string';

(*************************************************************)
(*************************************************************)

(**
 @brief get the hebrew holyday type.

 @param holyday the holyday number.
 @return the number of holyday type.
*)
function hdate_get_holyday_type (holyday : cInt) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_holyday_type';

(**
 @brief size of hebrew year in days.
 
 @param hebrew_year the hebrew year.
 @return size of Hebrew year
*)
function hdate_get_size_of_hebrew_year (hebrew_year : cInt) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_size_of_hebrew_year';

(*************************************************************)
(*************************************************************)

(**
 @brief Days since Tishrey 3744
 
 @author Amos Shapir 1984 (rev. 1985, 1992) Yaacov Zamir 2003-2005 
 
 @param hebrew_year The Hebrew year
 @return Number of days since 3,1,3744
*)
function hdate_days_from_3744 (hebrew_year : cInt) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_days_from_3744';

(**
 @brief Return Hebrew year type based on size and first week day of year.
 
 @param size_of_year Length of year in days
 @param new_year_dw First week day of year
 @return the number for year type (1..14)
*)
function hdate_get_year_type (size_of_year, new_year_dw : cInt) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_year_type';

(**
 @brief Compute Julian day from Gregorian date

 @author Yaacov Zamir (algorithm from Henry F. Fliegel and Thomas C. Van Flandern ,1968)

 @param day Day of month 1..31
 @param month Month 1..12
 @param year Year in 4 digits e.g. 2001
 @return the julian day number
 *)
 function hdate_gdate_to_jd (day, month, year : cInt) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_gdate_to_jd';

(**
 @brief Compute Julian day from Hebrew day, month and year
 
 @author Amos Shapir 1984 (rev. 1985, 1992) Yaacov Zamir 2003-2005

 @param day Day of month 1..31
 @param month Month 1..14 (13 - Adar 1, 14 - Adar 2)
 @param year Hebrew year in 4 digits e.g. 5753
 @param jd_tishrey1 return the julian number of 1 Tishrey this year
 @param jd_tishrey1_next_year return the julian number of 1 Tishrey next year
 @return the julian day number
 *)
 function hdate_hdate_to_jd (day, month, year : cInt; jd_tishrey1, jd_tishrey1_next_year : pcInt) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_hdate_to_jd';
 
(**
 @brief Converting from the Julian day to the Gregorian date
 
 @author Yaacov Zamir (Algorithm, Henry F. Fliegel and Thomas C. Van Flandern ,1968)

 @param jd Julian day
 @param day return Day of month 1..31
 @param month return Month 1..12
 @param year return Year in 4 digits e.g. 2001
 *)
 procedure hdate_jd_to_gdate (jd : cInt; day, month, year : pcInt);
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_jd_to_gdate';

(**
 @brief Converting from the Julian day to the Hebrew day
 
 @author Yaacov Zamir 2005

 @param jd Julian day
 @param day return Day of month 1..31
 @param month return Month 1..14 (13 - Adar 1, 14 - Adar 2)
 @param year return Year in 4 digits e.g. 2001
 @param jd_tishrey1 return the julian number of 1 Tishrey this year
 @param jd_tishrey1_next_year return the julian number of 1 Tishrey next year
 *)
procedure hdate_jd_to_hdate (jd : cInt; day, month, year, jd_tishrey1, jd_tishrey1_next_year : pcInt);
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_jd_to_hdate';

(*************************************************************)
(*************************************************************)

(**
 @brief days from 1 january
  
 @param day this day of month
 @param month this month
 @param year this year
 @return the days from 1 jan
*)
function hdate_get_day_of_year (day, month, year : cInt) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_day_of_year';

(**
 @brief utc sunrise/set time for a gregorian date
  
 @param day this day of month
 @param month this month
 @param year this year
 @param longitude longitude to use in calculations
	degrees, negative values are east
  @param latitude latitude to use in calculations
	degrees, negative values are south
 @param sunrise return the utc sunrise in minutes after midnight (00:00)
 @param sunset return the utc sunset in minutes after midnight (00:00)
*)
procedure hdate_get_utc_sun_time (day, month, year : cInt; latitude, longitude : cDouble; sunrise, sunset : pcInt);
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_utc_sun_time';

(*************************************************************)
(*************************************************************)

(**
 @brief get the Gregorian day of the month

 @param h pointer this hdate struct.
 @return the Gregorian day of the month, 1..31.
 *)
 function hdate_get_gday (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_gday';

(**
 @brief get the Gregorian month

 @param h pointer this hdate struct.
 @return the Gregorian month, jan = 1.
 *)
function hdate_get_gmonth (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_gmonth';

(**
 @brief get the Gregorian year

 @param h pointer this hdate struct.
 @return the Gregorian year.
 *)
function hdate_get_gyear (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_gyear';

(**
 @brief get the Hebrew day of the month

 @param h pointer this hdate struct.
 @return the Hebrew day of the month, 1..30.
 *)
function hdate_get_hday (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_hday';

(**
 @brief get the Hebrew month

 @param h pointer this hdate struct.
 @return the Hebrew month, Tishery = 1 .. Adar I =13, Adar II = 14.
 *)
function hdate_get_hmonth (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_hmonth';

(**
 @brief get the Hebrew year

 @param h pointer this hdate struct.
 @return the Hebrew year.
 *)
function hdate_get_hyear (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_hyear';

(**
 @brief get the day of the week

 @param h pointer this hdate struct.
 @return the the day of the week.
 *)
function hdate_get_day_of_the_week (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_day_of_the_week';

(**
 @brief get the size of the hebrew year

 @param h pointer this hdate struct.
 @return the the size of the hebrew year.
 *)
function hdate_get_size_of_year (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_size_of_year';

(**
 @brief get the new year day of the week

 @param h pointer this hdate struct.
 @return the the new year day of the week.
 *)
function hdate_get_new_year_day_of_the_week (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_new_year_day_of_the_week';

(**
 @brief get the Julian day number

 @param h pointer this hdate struct.
 @return the Julian day number.
 *)
function hdate_get_julian (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_julian';

(**
 @brief get the number of days passed since 1 tishrey

 @param h pointer this hdate struct.
 @return the number of days passed since 1 tishrey.
 *)
function hdate_get_days (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_days';

(**
 @brief get the number of weeks passed since 1 tishrey

 @param h pointer this hdate struct.
 @return the number of weeks passed since 1 tishrey.
 *)
function hdate_get_weeks (h : Phdate_struct) : cInt;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_weeks';

(*************************************************************)
(*************************************************************)

(**
 @brief creat a new hdate struct object, must be deleted using delete_hdate.

 @return a new hdate object
 *)
function new_hdate : Phdate_struct;
     cdecl external LIBHDATE_LIBRARY_NAME name 'new_hdate';

(**
 @brief delete an hdate struct object.

 @param h pointer this hdate struct.
 *)
function delete_hdate (h : Phdate_struct) : Phdate_struct;
     cdecl external LIBHDATE_LIBRARY_NAME name 'delete_hdate';

(*************************************************************)
(*************************************************************)

(**
 @brief Return a static string, with the package name and version

 @return a a static string, with the package name and version
*)
function hdate_get_version_string : PChar;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_version_string';

(**
 @brief Return a static string, with the name of translator

 @return a a static string, with the name of translator
*)
function hdate_get_translator_string : PChar;
     cdecl external LIBHDATE_LIBRARY_NAME name 'hdate_get_translator_string';

implementation

end.
