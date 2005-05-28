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
