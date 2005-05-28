{$MODE OBJFPC}
program hdate_demo;

{
  Copyright:  2004 (c) Yaacov Zamir 
	Copyright:  2005 (c) Ido Kanner
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
  02111-1307, USA.

History:
  11/03/2005 - Initial translation.
}

uses hdate_class, hdate;

var
 h : THdateClass;

begin
 h := THdateClass.Create;
 try
	 
  // print gregorian date
  write ('Today is : ', h.GregDay, ',');
  write (h.GregMonth, ',');
  writeln (h.GregYear)
  // print hebrew date
  writeln (h.get_format_date (0, 0));
 
 finally
  h.Free;
 end;
end.
