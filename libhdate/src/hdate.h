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
	/** The number of day in the hebrew month (1..31). */
	int hd_day;
	/** The number of the hebrew month 1..14 (1 - tishre, 13 - adar 1, 14 - adar 2). */
	int hd_mon;
	/** The number of the hebrew year. */
	int hd_year;
	/** The number of the day in the month. (1..31) */
	int gd_day;
	/** The number of the month 1..12 (1 - jan). */
	int gd_mon;
	/** The number of the year. */
	int gd_year;
	/** hd_dw The day of the week 1..7 (1 - sunday). */
	int hd_dw;
	/** The length of the year in days. */
	int hd_size_of_year;
	/** The week day of Hebrew new year. */
	int hd_new_year_dw;
	/** hd_year_type The number type of year. */
	int hd_year_type;
	/** The Julian day number */
	int hd_jd;
	/** The number of days passed since 1 tishrey */
	int hd_days;
	/** The number of weeks passed since 1 tishrey */
	int hd_weeks;
} hdate_struct;

/********************************************************************************/
/********************************************************************************/

/**
 @brief compute date structure from the Gregorian date

 @param h pointer this hdate struct.
 @param d Day of month 1..31
 @param m Month 1..12 ,  if m or d is 0 return current date.
 @param y Year in 4 digits e.g. 2001
 @return pointer to this hdate struct
 */
hdate_struct *
hdate_set_gdate (hdate_struct *h, int d, int m, int y);

/**
 @brief compute date structure from the Hebrew date

 @param h pointer this hdate struct.
 @param d Day of month 1..31
 @param m Month 1..14 ,  if m or d is 0 return current date.
 @param y Year in 4 digits e.g. 5731
 @return pointer to this hdate struct
 */
hdate_struct *
hdate_set_hdate (hdate_struct *h, int d, int m, int y);

/**
 @brief compute date structure from the Julian day

 @param h pointer this hdate struct.
 @param jd the julian day number.
 @return pointer to this hdate struct
 */
hdate_struct *
hdate_set_jd (hdate_struct *h, int jd);

/*************************************************************/
/*************************************************************/

/**
 @brief get formated hebrew date.

 return the short ( e.g. "1 Tishrey" ) or 
 long (e.g. "Tuesday 18 Tishrey 5763 Hol hamoed Sukot" ) formated date.

 @param h pointer this hdate struct.
 @param diaspora if true give diaspora holydays.
 @param s short flag.
 @return a static string of foramted date
*/
char *
hdate_get_format_date (hdate_struct * h, int diaspora, int s);

/**
 @brief get the number of hebrew parasha.

 @param h pointer this hdate struct.
 @param diaspora if true give diaspora readings
 @return the number of parasha 1. Bereshit etc..
   (55 trow 61 are joined strings e.g. Vayakhel Pekudei)
*/
int
hdate_get_parasha (hdate_struct * h, int diaspora);

/**
 @brief get the number of hebrew holyday.

 @param h pointer this hdate struct.
 @param diaspora if true give diaspora holydays
 @return the number of holyday.
*/
int
hdate_get_holyday (hdate_struct * h, int diaspora);

/*************************************************************/
/*************************************************************/

/**
 @brief convert an integer to hebrew string. 
 
 @param n The int to convert
 @return a static string of the hebrew number UTF-8 (logical)
 @attention ( 0 < n < 10000)
*/
char *
hdate_get_int_string (int n);

/**
 @brief get name of week day.

 @param day The number of the day 1..7 (1 - sun).
 @param s short flag 
   true - returns a short string: sun, false returns: sunday.
 @return a static string of the day of the week
*/
char *
hdate_get_day_string (int day, int s);

/**
 @brief name of month.

 @param month the number of the month 1..12 (1 - jan).
 @param s short flag.
 @return a static string of month name
*/
char *
hdate_get_month_string (int month, int s);

/**
 @brief name of hebrew month.

 @param month the number of the month 1..14 
   (1 - tishre, 13 - adar 1, 14 - adar 2).
 @param s short flag.
 @return a static string of month name
*/
char *
hdate_get_hebrew_month_string (int month, int s);

/**
 @brief name of hebrew holyday.

 @param holyday the holyday number.
 @param s short flag.
 @return a static string of holyday name
*/
char *
hdate_get_holyday_string (int holyday, int s);

/**
 @brief name of parasha

 @param parasha the number of parasha 1-Bereshit
   (55 trow 61 are joined strings e.g. Vayakhel Pekudei)
 @param s short flag.
 @return a static string of parasha name
*/
char *
hdate_get_parasha_string (int parasha, int s);

/*************************************************************/
/*************************************************************/

/**
 @brief get the hebrew holyday type.

 @param holyday the holyday number.
 @return the number of holyday type.
*/
int
hdate_get_holyday_type (int holyday);

/**
 @brief size of hebrew year in days.
 
 @param hebrew_year the hebrew year.
 @return size of Hebrew year
*/
int
hdate_get_size_of_hebrew_year (int hebrew_year);

/*************************************************************/
/*************************************************************/

/**
 @brief Days since Tishrey 3744
 
 @author Amos Shapir 1984 (rev. 1985, 1992) Yaacov Zamir 2003-2005 
 
 @param hebrew_year The Hebrew year
 @return Number of days since 3,1,3744
*/
int
hdate_days_from_3744 (int hebrew_year);

/**
 @brief Return Hebrew year type based on size and first week day of year.
 
 year type | year length | Tishery 1 day of week
 | 1       | 353         | 2 
 | 2       | 353         | 7 
 | 3       | 354         | 3 
 | 4       | 354         | 5 
 | 5       | 355         | 2 
 | 6       | 355         | 5 
 | 7       | 355         | 7 
 | 8       | 383         | 2 
 | 9       | 383         | 5 
 |10       | 383         | 7 
 |11       | 384         | 3 
 |12       | 385         | 2 
 |13       | 385         | 5 
 |14       | 385         | 7 
 
 @param size_of_year Length of year in days
 @param new_year_dw First week day of year
 @return the number for year type (1..14)
*/
int
hdate_get_year_type (int size_of_year, int new_year_dw);

/**
 @brief Compute Julian day from Gregorian date

 @author Yaacov Zamir (algorithm from Henry F. Fliegel and Thomas C. Van Flandern ,1968)

 @param day Day of month 1..31
 @param month Month 1..12
 @param year Year in 4 digits e.g. 2001
 @return the julian day number
 */
int
hdate_gdate_to_jd (int day, int month, int year);

/**
 @brief Compute Julian day from Hebrew day, month and year
 
 @author Amos Shapir 1984 (rev. 1985, 1992) Yaacov Zamir 2003-2005

 @param day Day of month 1..31
 @param month Month 1..14 (13 - Adar 1, 14 - Adar 2)
 @param year Hebrew year in 4 digits e.g. 5753
 @return the julian day number
 */
int
hdate_hdate_to_jd (int day, int month, int year, int *jd_tishrey1, int *jd_tishrey1_next_year);

/**
 @brief Converting from the Julian day to the Gregorian date
 
 @author Yaacov Zamir (Algorithm, Henry F. Fliegel and Thomas C. Van Flandern ,1968)

 @param jd Julian day
 @param d return Day of month 1..31
 @param m return Month 1..12
 @param y return Year in 4 digits e.g. 2001
 */
void
hdate_jd_to_gdate (int jd, int *day, int *month, int *year);

/**
 @brief Converting from the Julian day to the Hebrew day
 
 @author Yaacov Zamir 2005

 @param jd Julian day
 @param day return Day of month 1..31
 @param month return Month 1..14 (13 - Adar 1, 14 - Adar 2)
 @param year return Year in 4 digits e.g. 2001
 */
void
hdate_jd_to_hdate (int jd, int *day, int *month, int *year, int *jd_tishrey1, int *jd_tishrey1_next_year);

#ifdef __cplusplus
}
#endif

#endif
