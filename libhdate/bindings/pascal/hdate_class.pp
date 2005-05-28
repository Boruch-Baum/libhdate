 {
This unit create a Pascal object oriented class for libhdate.
This unit is created as a true object oriented class.
   
   Copyright  2005       Ido Kanner <idokan@gmail.com>
   
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
    19-21/05/2005 - Initial creation
 }
{$IFDEF FPC}
  {$MODE OBJFPC}
{$ELSE}
  {$Warning 'This code was tested with Free Pascal Only.'}
{$ENDIF}
 unit hdate_class;
 
 interface
 uses hdate, SysUtils;

 type
	 THdateClass = class
	 private
		 fh           : Phdate_struct;
	   FToDestroy   : Boolean;
	   fJulianDay   : LongInt;
     fDiaspora    : Boolean;
     fHolyday,
     fHolydayType : LongInt;
     fParasha     : LongInt;
     fHebrewDay,
     fHebrewYear  : String;
   
     procedure InitValue;
   
     procedure SetHebDay    (Value : LongInt);
     procedure SetGregDay   (Value : LongInt);
     procedure SetJulianDay (Value : LongInt);
     procedure SetHebMonth  (Value : LongInt);
     procedure SetGregMonth (Value : LongInt);
     procedure SetGregYear  (Value : LongInt);
     procedure SetHebYear   (Value : LongInt);
     procedure SetDiaspora  (Value : Boolean);
     
     function GetGregDay   : LongInt;
     function GetGregMonth : LongInt;
     function GetGregYear  : LongInt;
     function GetHebDay    : LongInt;
     function GetHebMonth  : LongInt;
     function GetHebYear   : LongInt;
	 public
	   constructor Create;                                       virtual; overload;
	   constructor Create  (hDateRec : Phdate_struct);           virtual; overload;
	   destructor Destroy;                                       override;
     
     procedure Assign    (Source : THdateClass);                virtual;
   
     function get_format_date              (s : LongInt)                                 : String;
     function get_day_of_week_string       (s : LongInt)                                 : String;
     function get_month_string             (s : LongInt)                                 : String;
     function get_hebrew_month_string      (s : LongInt)                                 : String;
     function get_holyday_string           (s : LongInt)                                 : String;
     function get_parasha_string           (s : LongInt)                                 : String;
     function GetDayOfWeek                                                               : LongInt;
     function get_size_of_year                                                           : LongInt;
     function get_new_year_day_of_the_week                                               : LongInt;
     function get_days                                                                   : LongInt;
     function get_weeks                                                                  : LongInt;
     function get_sunrise                   (latitude, longitude : Double; tz : LongInt) : LongInt;
     function get_sunset                    (latitude, longitude : Double; tz : LongInt) : LongInt;
     
     property Days             : LongInt  read get_days;
     property DayOfWeek        : LongInt  read GetDayOfWeek;
     property HebrewDay        : String   read fHebrewDay;
     property HebrewYear       : String   read fHebrewYear;
     property Holyday          : LongInt  read fHolyday;
     property HolydayType      : LongInt  read fHolydayType;
     property NewYearDayOfWeek : LongInt  read get_new_year_day_of_the_week;
     property Parasha          : LongInt  read fParasha;
     property Weeks            : LongInt  read get_weeks;
     property YearSize         : Longint  read get_size_of_year;
   published
     property HebDay           : LongInt  read GetHebDay     write SetHebDay;
     property GregDay          : LongInt  read GetGregDay    write SetGregDay;
     property Diaspora         : Boolean  read fDiaspora     write SetDiaspora    default True;
	   property JulianDay        : LongInt  read fJulianDay    write SetJulianDay;
     property HebMonth         : LongInt  read GetHebMonth   write SetHebMonth;
	   property GregMonth        : LongInt  read GetGregMonth  write SetGregMonth;
	   property ToDestroy        : Boolean  read FToDestroy    write fToDestroy     default True;
     property GregYear         : LongInt  read GetGregYear   write SetGregYear;
		 property HebYear          : LongInt  read GetHebYear    write SetHebYear;
	 end;
  
implementation
uses DateUtils, cTypes;

procedure THdateClass.InitValue;
var
  ADay, AMonth, AYear : Word;
begin
 DecodeDate(SysUtils.Now, AYear, AMonth, ADay);
 
 hdate_set_gdate (Fh, ADay, AMonth, AYear);

 hdate_set_hdate (fh, hdate_get_hday (fh), hdate_get_hmonth (fh), hdate_get_hyear (fh));
 fHebrewDay  := StrPas (hdate_get_int_string (fh^.hd_day));
 fHebrewYear := StrPas (hdate_get_int_string (fh^.hd_year));
 
 SetJulianDay (hdate_gdate_to_jd (ADay, AMonth, AYear));
 SetDiaspora  (True);
end;

constructor THdateClass.Create;
begin
 fh         := new_hdate;
 fToDestroy := True;
 InitValue; 
end;

constructor THdateClass.Create (hDateRec : Phdate_struct);
begin
 fh         := hDateRec;
 fToDestroy := False;
 InitValue;
end;

destructor THdateClass.Destroy;
begin
 if fToDestroy then
   delete_hdate (fh);
  
 fh := nil;
  
 inherited;  
end;

procedure THdateClass.SetHebDay (Value : LongInt);
begin
 hdate_set_hdate (Fh, Value, GetHebMonth, GetHebYear);
 fHebrewDay := StrPas (hdate_get_int_string (fh^.hd_day));
end;

procedure THdateClass.SetGregDay (Value : LongInt);
begin
  hdate_set_gdate (fh, Value, GetGregMonth, GetGregYear);
end;

procedure THdateClass.SetJulianDay (Value : LongInt);
begin
  fJulianDay := Value;
  hdate_set_jd (fh, fJulianDay);
end;

procedure THdateClass.SetHebMonth (Value : LongInt);
begin
  hdate_set_hdate (Fh, GetHebDay, Value, GetHebYear);
end;

procedure THdateClass.SetGregMonth (Value : LongInt);
begin
  hdate_set_gdate (fh, GetGregDay, Value, GetGregYear);
end;

procedure THdateClass.SetHebYear (Value : LongInt);
begin
 hdate_set_hdate (Fh, GetHebDay, GetGregMonth, Value);
 fHebrewYear := StrPas (hdate_get_int_string (fh^.hd_year));
end;

procedure THdateClass.SetGregYear (Value : LongInt);
begin
  hdate_set_gdate (fh, GetGregDay, GetGregMonth, Value);
end;

procedure THdateClass.SetDiaspora  (Value : Boolean);
begin
 fDiaspora    := Value;
 fHolyday     := hdate_get_holyday      (fh, cInt(FDiaspora));
 fHolydayType := hdate_get_holyday_type (fHolyday);
 fParasha     := hdate_get_parasha      (fh, cInt(fDiaspora));
end;

function THdateClass.GetGregDay : LongInt;
begin
  Result := hdate_get_gday (fh);
end;

function THdateClass.GetGregMonth : LongInt;
begin
  Result := hdate_get_gmonth (fh);
end;

function THdateClass.GetGregYear  : LongInt;
begin
  Result := hdate_get_gyear (fh);
end;

function THdateClass.GetHebDay : LongInt;
begin
  Result := hdate_get_hday (fh);
end;

function THdateClass.GetHebMonth : LongInt;
begin
  Result := hdate_get_hmonth (fh);
end;

function THdateClass.GetHebYear : LongInt;
begin
  Result :=hdate_get_hyear (fh);
end;

procedure THdateClass.Assign (Source : THdateClass);
begin
  if (not Assigned(Source)) then
    begin
      raise Exception.Create ('Source is not allocated.');
    end;
    
  Assert (Assigned(Source), 'Source is not allocated.'); // Should never happned ... but just in case...

  fToDestroy := Source.ToDestroy;
  
  HebDay    := Source.HebDay;
  GregDay   := Source.GregDay;
  HebMonth  := Source.HebMonth;
  GregMonth := Source.GregMonth;
  HebYear   := Source.HebYear;
  GregYear  := Source.GregYear;
  
  SetJulianDay (Source.JulianDay);
  SetDiaspora  (Source.Diaspora);
end;

function THdateClass.get_format_date (s : LongInt) : String;
begin
  Result := StrPas (hdate_get_format_date (fh, cInt(fDiaspora), s));
end;

function THdateClass.get_day_of_week_string (s : LongInt) : String;
begin
  Result := StrPas (hdate_get_day_string (fh^.hd_dw, s));
end;

function THdateClass.get_month_string (s : LongInt) : String;
begin
  Result := StrPas (hdate_get_month_string (fh^.gd_mon, s));
end;

function THdateClass.get_hebrew_month_string (s : LongInt) : String;
begin
  Result := StrPas (hdate_get_hebrew_month_string (fh^.hd_mon, s));
end;

function THdateClass.get_holyday_string (s : LongInt) : String;
begin
  Result  := StrPas (hdate_get_holyday_string (fHolyday, s));
end;

function THdateClass.get_parasha_string (s : LongInt) : String;
begin
  Result := StrPas (hdate_get_parasha_string (fParasha, s));
end;

function THdateClass.GetDayOfWeek : LongInt;
begin
  Result := hdate_get_day_of_the_week (fh);
end;

function THdateClass.get_size_of_year : LongInt;
begin
  Result := hdate_get_size_of_year (fh);
end;

function THdateClass.get_new_year_day_of_the_week : LongInt;
begin
  Result := hdate_get_new_year_day_of_the_week (fh);
end;

function THdateClass.get_days : LongInt;
begin
  Result := hdate_get_days (fh);
end;

function THdateClass.get_weeks : LongInt;
begin
  Result := hdate_get_weeks (fh);
end;

function THdateClass.get_sunrise (latitude, longitude : Double; tz : LongInt) : LongInt;
var
 sunrise,
 sunset   : cInt;
begin
  hdate_get_utc_sun_time (fh^.gd_day, fh^.gd_mon, fh^.gd_year, latitude, longitude, @sunrise, @sunset);
	Result := sunrise + tz * 60;
end;

function THdateClass.get_sunset (latitude, longitude : Double; tz : LongInt) : LongInt;
var
 sunrise,
 sunset   : cInt;
begin
	hdate_get_utc_sun_time (fh^.gd_day, fh^.gd_mon, fh^.gd_year, latitude, longitude, @sunrise, @sunset);
	Result := sunset + tz * 60;
end;

end.
