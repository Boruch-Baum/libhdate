/*  libhdate
 *  Copyright  2004-2005  Yaacov Zamir <kzamir@walla.co.il>
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * See Amos Shapir hdate_README file attached for 
 * Copyright notice for functions based on Amos's code
 */
 
#ifndef __HDATE_H__
#define __HDATE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** Base structore for hebrew dates

*/
typedef struct
{
	/** The in the hebrew month (1..31). */
	int hd_day;
	/** The number of the hebrew month 1..14 (1 - tishre, 13 - adar 1, 14 - adar 2). */
	int hd_mon;
	/** The number of the hebrew year. */
	int hd_year;
	/** gd_mon The number of the month 1..12 (1 - jan). */
	int gd_day;
	/** gd_day The number of the day in the month. (1..31)*/
	int gd_mon;
	/** gd_year; The number of the year. */
	int gd_year;
	/** hd_dw The day of the week 1..7 (1 - sunday). */
	int hd_dw;
	/** hd_size_of_year The length of the year in days. */
	int hd_size_of_year;
	/** hd_new_year_dw The week day of Hebrew new year. */
	int hd_new_year_dw;
	/** hd_year_type The number type of year. */
	int hd_year_type;
} hdate_struct;

/**
 @brief Days since bet (?) Tishrey 3744
 
 @author Amos Shapir 1984 (rev. 1985, 1992) Yaacov Zamir 2003-2005 
 
 @param hebrew_year The Hebrew year
 @return Number of days since 3,1,3744
*/
int
hdate_days_from_3744 (int hebrew_year);

/**
 @brief Size of Hebrew year in days
 
 @param hebrew_year The Hebrew year
 @return Size of Hebrew year
*/
int
hdate_size_of_hebrew_year (int hebrew_year);

/**
 @brief Return Hebrew year type based on size and first week day of year.

 @param size_of_year Length of year in days
 @param new_year_dw First week day of year
 @return A number for year type (1..24)
*/
int
hdate_get_year_type (int size_of_year, int new_year_dw);

/**
 @brief Compute Julian day from Gregorian day, month and year
 Algorithm from 'Julian and Gregorian Day Numbers' by Peter Meyer

 @author Yaacov Zamir ( algorithm from Henry F. Fliegel and Thomas C. Van Flandern ,1968)

 @param day Day of month 1..31
 @param month Month 1..12
 @param year Year in 4 digits e.g. 2001
 @return The julian day number
 */
int
hdate_gdate_to_jd (int day, int month, int year);

/**
 @brief Compute Julian day from Hebrew day, month and year
 
 @author Amos Shapir 1984 (rev. 1985, 1992) Yaacov Zamir 2003-2005

 @param day Day of month 1..31
 @param month Month 1..14 (13 - Adar 1, 14 - Adar 2)
 @param year Hebrew year in 4 digits e.g. 5753
 @return The julian day number
 */
int
hdate_hdate_to_jd (int day, int month, int year);

/**
 @brief Converting from the Julian day to the Gregorian day
 Algorithm from 'Julian and Gregorian Day Numbers' by Peter Meyer 

 @author Yaacov Zamir ( Algorithm, Henry F. Fliegel and Thomas C. Van Flandern ,1968)

 @param jd Julian day
 @param d Return Day of month 1..31
 @param m Return Month 1..12
 @param y Return Year in 4 digits e.g. 2001
 */
void
hdate_jd_to_gdate (int jd, int *d, int *m, int *y);

/**
 @brief Converting from the Julian day to the Hebrew day
 
 @author Yaacov Zamir 2005

 @param jd Julian day
 @param day Return Day of month 1..31
 @param month Return Month 1..14 (13 - Adar 1, 14 - Adar 2)
 @param year Return Year in 4 digits e.g. 2001
 */
void
hdate_jd_to_hdate (int jd, int *day, int *month, int *year);

/********************************************************************************/
/********************************************************************************/

/**
 @brief compute date structure from the Gregorian date

 @param d Day of month 1..31
 @param m Month 1..12 ,  if m or d is 0 return current date.
 @param y Year in 4 digits e.g. 2001
 */
hdate_struct *
hdate_hdate (int d, int m, int y);

/**
 @brief compute date structure from the Hebrew date

 @param d Day of month 1..31
 @param m Month 1..14 ,  if m or d is 0 return current date.
 @param y Year in 4 digits e.g. 5731
 */
hdate_struct *
hdate_gdate (int d, int m, int y);

/********************************************************************************/
/********************************************************************************/

/**
 @brief convert an integer to hebrew string UTF-8 (logical)
 
 @param n The int to convert

 @attention ( 0 < n < 10000)
*/
char *
hdate_get_int_string (int n);

/**
 @brief Return a static string, with name of wek day.

 @param day The number of the day 0..6 (0 - sun).
 @param s A short flag (true - returns a short string: sun, .., false returns: sunday, .. ).
*/
char *
hdate_get_day_string (int day, int s);

/**
 @brief Return a static string, with name of month.

 @param month The number of the month 1..12 (1 - jan).
 @param s A short flag.
*/
char *
hdate_get_month_string (int month, int s);

/**
 @brief Return a static string, with name of hebrew month.

 @param month The number of the month 1..14 (1 - tishre, 13 - adar 1, 14 - adar 2).
 @param s A short flag.
*/
char *
hdate_get_hebrew_month_string (int month, int s);

/**
 @brief Name of hebrew holyday.

 @param holyday The holyday number.
 @param s A short flag.
*/
char *
hdate_get_holyday_string (int holyday, int s);

/**
 @brief Name of Parasha

 @param parasha The Number of Parasha 1-Bereshit
	(55 trow 61 are joined strings e.g. Vayakhel Pekudei)
 @param s A short flag.
*/
char *
hdate_get_parasha_string (int parasha, int s);

/**
 @brief Return a static string, with the hebrew date.

 return the short ( e.g. "1 Tishrey" ) or 
 long (e.g. "Tuesday 18 Tishrey 5763 Hol hamoed Sukot" ) formated date.

 @param h The hdate_struct of the date to print.
 @param s A short flag (true - returns a short string, false returns a long string. ).
*/
char *
hdate_get_format_date (hdate_struct * h, int s);

/********************************************************************************/
/********************************************************************************/

/**
 @brief Return number of hebrew parasha.

 @param hebdate The hdate_struct of the date to use.
 @return the number of parasha 1. Bereshit etc..
 (55 trow 61 are joined strings e.g. Vayakhel Pekudei)
*/
int
hdate_get_parasha (hdate_struct * hebdate);

/********************************************************************************/
/********************************************************************************/

/**
 @brief Return number of hebrew holyday.

 @param h The hdate_struct of the date to use.
 @return the number of holyday.
*/
int
hdate_get_holyday (hdate_struct * h);


#ifdef __cplusplus
}
#endif

#endif
