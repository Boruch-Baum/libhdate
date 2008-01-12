(*  libhdate
   Copyright (C) 1984-2003 Amos Shapir, 2004-2007  Yaacov Zamir <kzamir@walla.co.il>
   Copyright (C)  2008  Ido Kanner   <idokan@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

History:
  12/01/2008 - Added support for dynamic loading for the library
*)
unit hdate_dyn_pascal;

{$IFDEF FPC}
  {$MODE FPC}
  {$PACKRECORDS C}
  {$CALLING cdecl}
{$ENDIF}

(*
   Auto load means that we are loading the functions when the unit is loaded.
   Disable this define will mean that you should load and unload it by hand, using the needed procedures
*)
{$DEFINE AUTO_LOAD}

interface

{$IFDEF FPC}
uses
  ctypes;
{$ENDIF}

procedure hdate_init;
procedure hdate_done;
function IsLoaded : Boolean;

const
  {$IFDEF UNIX}
  LIBHDATE_LIBRARY_NAME = 'libhdate.so';
  {$ELSE}
    {$FATAL Unsupported environment}
  {$ENDIF}

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

var
(**
 @brief compute date structure from the Gregorian date

 @param h pointer this hdate struct.
 @param d Day of month 1..31
 @param m Month 1..12
 @param y Year in 4 digits e.g. 2001
 @return pointer to this hdate struct
 *)
  hdate_set_gdate : function (h : Phdate_struct; d, m, y : cInt) : Phdate_struct;

(**
 @brief compute date structure from the Hebrew date

 @param h pointer this hdate struct.
 @param d Day of month 1..31
 @param m Month 1..14 , (13 - Adar 1, 14 - Adar 2)
 @param y Year in 4 digits e.g. 5731
 @return pointer to this hdate struct
 *)
  hdate_set_hdate : function (h : Phdate_struct; d, m, y : cInt) : Phdate_struct;

(**
 @brief compute date structure from the Julian day

 @param h pointer this hdate struct.
 @param jd the julian day number.
 @return pointer to this hdate struct
 *)
   hdate_set_jd : function (h : Phdate_struct; jd : cInt) : Phdate_struct;
   
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
  hdate_get_format_date : function  (h : Phdate_struct; diaspora, s : cInt) : PChar;

(**
 @brief get the number of hebrew parasha.

 @param h pointer this hdate struct.
 @param diaspora if true give diaspora readings
 @return the number of parasha 1. Bereshit etc..
   (55 trow 61 are joined strings e.g. Vayakhel Pekudei)
*)
   hdate_get_parasha : function (h : Phdate_struct; diaspora : cInt) : cInt;

(**
 @brief get the number of hebrew holyday.

 @param h pointer this hdate struct.
 @param diaspora if true give diaspora holydays
 @return the number of holyday.
*)
  hdate_get_holyday : function (h : Phdate_struct; diaspora : cInt) : cInt;

(*************************************************************)
(*************************************************************)

(**
 @brief convert an integer to hebrew string.

 @param n The int to convert
 @return a static string of the hebrew number UTF-8 (logical)
 @attention ( 0 < n < 10000)
*)
  hdate_get_int_string : function (n : cInt) : PChar;


(**
 @brief get name of week day.

 @param day The number of the day 1..7 (1 - sun).
 @param s short flag
   true - returns a short string: sun, false returns: sunday.
 @return a static string of the day of the week
*)
   hdate_get_day_string : function (day : cInt; s : cInt) : PChar;


(**
 @brief name of month.

 @param month the number of the month 1..12 (1 - jan).
 @param s short flag.
 @return a static string of month name
*)
   hdate_get_month_string : function (month : cInt; s : cInt) : PChar;


(**
 @brief name of hebrew month.

 @param month the number of the month 1..14
   (1 - tishre, 13 - adar 1, 14 - adar 2).
 @param s short flag.
 @return a static string of month name
*)
   hdate_get_hebrew_month_string : function (month : cInt; s : cInt) : PChar;


(**
 @brief name of hebrew holyday.

 @param holyday the holyday number.
 @param s short flag.
 @return a static string of holyday name
*)
   hdate_get_holyday_string : function (holyday, s : cInt) : PChar;


(**
 @brief name of parasha

 @param parasha the number of parasha 1-Bereshit
   (55 trow 61 are joined strings e.g. Vayakhel Pekudei)
 @param s short flag.
 @return a static string of parasha name
*)
    hdate_get_parasha_string : function (parasha, s : cInt) : PChar;


(*************************************************************)
(*************************************************************)

(**
 @brief get the hebrew holyday type.

 @param holyday the holyday number.
 @return the number of holyday type.
*)
    hdate_get_holyday_type : function (holyday : cInt) : cInt;


(**
 @brief size of hebrew year in days.

 @param hebrew_year the hebrew year.
 @return size of Hebrew year
*)
    hdate_get_size_of_hebrew_year : function (hebrew_year : cInt) : cInt;


(*************************************************************)
(*************************************************************)

(**
 @brief Days since Tishrey 3744

 @author Amos Shapir 1984 (rev. 1985, 1992) Yaacov Zamir 2003-2005

 @param hebrew_year The Hebrew year
 @return Number of days since 3,1,3744
*)
    hdate_days_from_3744 : function (hebrew_year : cInt) : cInt;


(**
 @brief Return Hebrew year type based on size and first week day of year.

 @param size_of_year Length of year in days
 @param new_year_dw First week day of year
 @return the number for year type (1..14)
*)
    hdate_get_year_type : function (size_of_year, new_year_dw : cInt) : cInt;


(**
 @brief Compute Julian day from Gregorian date

 @author Yaacov Zamir (algorithm from Henry F. Fliegel and Thomas C. Van Flandern ,1968)

 @param day Day of month 1..31
 @param month Month 1..12
 @param year Year in 4 digits e.g. 2001
 @return the julian day number
 *)
    hdate_gdate_to_jd : function (day, month, year : cInt) : cInt;


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
    hdate_hdate_to_jd : function (day, month, year : cInt; jd_tishrey1, jd_tishrey1_next_year : pcInt) : cInt;


(**
 @brief Converting from the Julian day to the Gregorian date

 @author Yaacov Zamir (Algorithm, Henry F. Fliegel and Thomas C. Van Flandern ,1968)

 @param jd Julian day
 @param day return Day of month 1..31
 @param month return Month 1..12
 @param year return Year in 4 digits e.g. 2001
 *)
    hdate_jd_to_gdate : procedure(jd : cInt; day, month, year : pcInt);


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
    hdate_jd_to_hdate : procedure(jd : cInt; day, month, year, jd_tishrey1, jd_tishrey1_next_year : pcInt);


(*************************************************************)
(*************************************************************)

(**
 @brief days from 1 january

 @param day this day of month
 @param month this month
 @param year this year
 @return the days from 1 jan
*)
    hdate_get_day_of_year : function (day, month, year : cInt) : cInt;


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
    hdate_get_utc_sun_time : procedure(day, month, year : cInt; latitude, longitude : cDouble; sunrise, sunset : pcInt);


(*************************************************************)
(*************************************************************)

(**
 @brief get the Gregorian day of the month

 @param h pointer this hdate struct.
 @return the Gregorian day of the month, 1..31.
 *)
    hdate_get_gday : function (h : Phdate_struct) : cInt;


(**
 @brief get the Gregorian month

 @param h pointer this hdate struct.
 @return the Gregorian month, jan = 1.
 *)
    hdate_get_gmonth : function (h : Phdate_struct) : cInt;


(**
 @brief get the Gregorian year

 @param h pointer this hdate struct.
 @return the Gregorian year.
 *)
    hdate_get_gyear : function (h : Phdate_struct) : cInt;


(**
 @brief get the Hebrew day of the month

 @param h pointer this hdate struct.
 @return the Hebrew day of the month, 1..30.
 *)
    hdate_get_hday : function (h : Phdate_struct) : cInt;


(**
 @brief get the Hebrew month

 @param h pointer this hdate struct.
 @return the Hebrew month, Tishery = 1 .. Adar I =13, Adar II = 14.
 *)
    hdate_get_hmonth : function (h : Phdate_struct) : cInt;


(**
 @brief get the Hebrew year

 @param h pointer this hdate struct.
 @return the Hebrew year.
 *)
    hdate_get_hyear : function (h : Phdate_struct) : cInt;


(**
 @brief get the day of the week

 @param h pointer this hdate struct.
 @return the the day of the week.
 *)
    hdate_get_day_of_the_week : function (h : Phdate_struct) : cInt;


(**
 @brief get the size of the hebrew year

 @param h pointer this hdate struct.
 @return the the size of the hebrew year.
 *)
    hdate_get_size_of_year : function (h : Phdate_struct) : cInt;


(**
 @brief get the new year day of the week

 @param h pointer this hdate struct.
 @return the the new year day of the week.
 *)
    hdate_get_new_year_day_of_the_week : function (h : Phdate_struct) : cInt;


(**
 @brief get the Julian day number

 @param h pointer this hdate struct.
 @return the Julian day number.
 *)
    hdate_get_julian : function (h : Phdate_struct) : cInt;


(**
 @brief get the number of days passed since 1 tishrey

 @param h pointer this hdate struct.
 @return the number of days passed since 1 tishrey.
 *)
    hdate_get_days : function (h : Phdate_struct) : cInt;


(**
 @brief get the number of weeks passed since 1 tishrey

 @param h pointer this hdate struct.
 @return the number of weeks passed since 1 tishrey.
 *)
    hdate_get_weeks : function (h : Phdate_struct) : cInt;


(*************************************************************)
(*************************************************************)

(**
 @brief creat a new hdate struct object, must be deleted using delete_hdate.

 @return a new hdate object
 *)
    new_hdate : function : Phdate_struct;


(**
 @brief delete an hdate struct object.

 @param h pointer this hdate struct.
 *)
    delete_hdate : function (h : Phdate_struct) : Phdate_struct;


(*************************************************************)
(*************************************************************)

(**
 @brief Return a static string, with the package name and version

 @return a a static string, with the package name and version
*)
    hdate_get_version_string : function : PChar;

(**
 @brief Return a static string, with the name of translator

 @return a a static string, with the name of translator
*)
    hdate_get_translator_string : function : PChar;

(**
 @brief utc sun times for altitude at a gregorian date

 @param day this day of month
 @param month this month
 @param year this year
 @param latitude latitude to use in calculations
 @param longitude longitude to use in calculations
 @param deg degrees of sun's altitude (0 -  Zenith .. 90 - Horizon)
 @param sunrise return the utc sunrise in minutes
 @param sunset return the utc sunset in minutes
*)
    hdate_get_utc_sun_time_deg : procedure (day, month, year : cint; latitude, longitude, deg : cdouble; sunrise, sunset : pcint);

(**
 @brief utc sunrise/set time for a gregorian date

 @param day this day of month
 @param month this month
 @param year this year
 @param longitude longitude to use in calculations
 @param latitude latitude to use in calculations
 @param sun_hour return the length of shaa zaminit in minutes
 @param first_light return the utc alut ha-shachar in minutes
 @param talit return the utc tphilin and talit in minutes
 @param sunrise return the utc sunrise in minutes
 @param midday return the utc midday in minutes
 @param sunset return the utc sunset in minutes
 @param first_stars return the utc tzeit hacochavim in minutes
 @param three_stars return the utc shlosha cochavim in minutes
*)

    hdate_get_utc_sun_time_full : procedure (day, month, year                                       : cint;
                                             latitude, longitude                                    : cdouble;
                                             sun_hour, first_light, talit, sunrise, midday, sunset,
                                             first_stars, three_stars                               :  pcint);


implementation
uses dynlibs;

var
  Loaded    : Boolean;
  LibHandle : TLibHandle;

procedure hdate_init;

  function AssignProc(Name : PChar; Fnc : Pointer) : boolean;
  var
    address : pointer;
    pfnc    : ^pointer;
  begin
    address    := nil;
    address    := GetProcedureAddress(LibHandle, Name);
    pfnc       := Fnc;
    pfnc^      := address;
    AssignProc := address <> nil;
  end;
begin
  if (Loaded) OR (LibHandle <> NilHandle) then
    exit;
    
  Loaded := False;
  LibHandle := LoadLibrary(LIBHDATE_LIBRARY_NAME);
  if LibHandle = NilHandle then
    exit;
    
  if not AssignProc('hdate_set_gdate',                    @hdate_set_gdate)                    then exit;
  if not AssignProc('hdate_set_hdate',                    @hdate_set_hdate)                    then exit;
  if not AssignProc('hdate_set_jd',                       @hdate_set_jd)                       then exit;
  if not AssignProc('hdate_get_format_date',              @hdate_get_format_date)              then exit;
  if not AssignProc('hdate_get_parasha',                  @hdate_get_parasha)                  then exit;
  if not AssignProc('hdate_get_holyday',                  @hdate_get_holyday)                  then exit;
  if not AssignProc('hdate_get_int_string',               @hdate_get_int_string)               then exit;
  if not AssignProc('hdate_get_day_string',               @hdate_get_day_string)               then exit;
  if not AssignProc('hdate_get_month_string',             @hdate_get_month_string)             then exit;
  if not AssignProc('hdate_get_hebrew_month_string',      @hdate_get_hebrew_month_string)      then exit;
  if not AssignProc('hdate_get_holyday_string',           @hdate_get_holyday_string)           then exit;
  if not AssignProc('hdate_get_parasha_string',           @hdate_get_parasha_string)           then exit;
  if not AssignProc('hdate_get_holyday_type',             @hdate_get_holyday_type)             then exit;
  if not AssignProc('hdate_get_size_of_hebrew_year',      @hdate_get_size_of_hebrew_year)      then exit;
  if not AssignProc('hdate_days_from_3744',               @hdate_days_from_3744)               then exit;
  if not AssignProc('hdate_get_year_type',                @hdate_get_year_type)                then exit;
  if not AssignProc('hdate_gdate_to_jd',                  @hdate_gdate_to_jd)                  then exit;
  if not AssignProc('hdate_hdate_to_jd',                  @hdate_hdate_to_jd)                  then exit;
  if not AssignProc('hdate_jd_to_gdate',                  @hdate_jd_to_gdate)                  then exit;
  if not AssignProc('hdate_jd_to_hdate',                  @hdate_jd_to_hdate)                  then exit;
  if not AssignProc('hdate_get_day_of_year',              @hdate_get_day_of_year)              then exit;
  if not AssignProc('hdate_get_utc_sun_time',             @hdate_get_utc_sun_time)             then exit;
  if not AssignProc('hdate_get_gday',                     @hdate_get_gday)                     then exit;
  if not AssignProc('hdate_get_gmonth',                   @hdate_get_gmonth)                   then exit;
  if not AssignProc('hdate_get_gyear',                    @hdate_get_gyear)                    then exit;
  if not AssignProc('hdate_get_hday',                     @hdate_get_hday)                     then exit;
  if not AssignProc('hdate_get_hmonth',                   @hdate_get_hmonth)                   then exit;
  if not AssignProc('hdate_get_hyear',                    @hdate_get_hyear)                    then exit;
  if not AssignProc('hdate_get_day_of_the_week',          @hdate_get_day_of_the_week)          then exit;
  if not AssignProc('hdate_get_size_of_year',             @hdate_get_size_of_year)             then exit;
  if not AssignProc('hdate_get_new_year_day_of_the_week', @hdate_get_new_year_day_of_the_week) then exit;
  if not AssignProc('hdate_get_julian',                   @hdate_get_julian)                   then exit;
  if not AssignProc('hdate_get_days',                     @hdate_get_days)                     then exit;
  if not AssignProc('hdate_get_weeks',                    @hdate_get_weeks)                    then exit;
  if not AssignProc('new_hdate',                          @new_hdate)                          then exit;
  if not AssignProc('delete_hdate',                       @delete_hdate)                       then exit;
  if not AssignProc('hdate_get_version_string',           @hdate_get_version_string)           then exit;
  if not AssignProc('hdate_get_translator_string',        @hdate_get_translator_string)        then exit;
  if not AssignProc('hdate_get_utc_sun_time_deg',         @hdate_get_utc_sun_time_deg)         then exit;
  if not AssignProc('hdate_get_utc_sun_time_full',        @hdate_get_utc_sun_time_full)        then exit;
  
  Loaded := true;
end;

procedure hdate_done;
begin
 if LibHandle <> NilHandle then
    UnloadLibrary(LibHandle);

 Loaded := false;
end;

function IsLoaded : Boolean;
begin
  IsLoaded := Loaded;
end;

initialization
  Loaded := false;
{$IFDEF AUTO_LOAD}
  hdate_init;
{$ENDIF}
finalization
{$IFDEF AUTO_LOAD}
  hdate_done;
{$ENDIF}
end.

