// hdate_pascal.pp
// example file for libhdate.
//

//
// compile: fpc hdate_pascal.pp
//

//
//  Copyright (C) 2005  Ido Kaner
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

{$MODE OBJFPC}
{$IFDEF MSWINDOWS}
{$APPTYPE CONSOLE} //Make sure that in Windows we will be in a console mode
{$ENDIF}
program hdate_demo;
uses hdate_class, hdate;

var
 h : THdateClass;
 diaspora : Integer;

begin
 h := THdateClass.Create;
 diaspora := 0;

 try
	 
  // print gregorian date
  write   ('Today : ', h.GregDay, ',');
  write   (h.GregMonth, ',');
  writeln (h.GregYear);
 
  // print hebrew date
  writeln (h.get_format_date (diaspora));
 finally
  h.Free;
 end;
end.
