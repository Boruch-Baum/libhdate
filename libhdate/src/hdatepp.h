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
 
#ifndef __HDATE_PP_H__
#define __HDATE_PP_H__

#include <hdate.h>

/**
 @brief the libhdate namespace.
 */
namespace hdate 
{
	/**
	 @brief Hdate class.
	
	 class for Hebrew/Gregorian date conversions
	 */
	class Hdate;
		
	/**
	 @brief Hdate class.
	
	 class for Hebrew/Gregorian date conversions
	 */
	class Hdate
	{
		
	public:
		////////////////////////////////////////
		////////////////////////////////////////
		
		/**
		 @brief Hdate constructor.
		 */
		Hdate()
		{
			h = new_hdate();
		}
		
		/**
		 @brief Hdate destructor.
		 */
		~Hdate()
		{
			delete_hdate(h);
		}
		
		////////////////////////////////////////
		////////////////////////////////////////
		
		/**
		 @brief compute date structure from the Gregorian date
		
		 @param d Day of month 1..31
		 @param m Month 1..12 ,  if m or d is 0 return current date.
		 @param y Year in 4 digits e.g. 2001
		 */
		void
		set_gdate (int d, int m, int y)
		{
			hdate_set_gdate (h, d, m, y);
		}
		
		/**
		 @brief compute date structure from the Hebrew date
		
		 @param d Day of month 1..31
		 @param m Month 1..14 ,  if m or d is 0 return current date.
		 @param y Year in 4 digits e.g. 5731
		 */
		void
		set_hdate (int d, int m, int y)
		{
			hdate_set_hdate (h, d, m, y);
		}
		
		/**
		 @brief compute date structure from the Julian day
		
		 @param jd the julian day number.
		 */
		void
		set_jd (int jd)
		{
			hdate_set_jd (h, jd);
		}
		
		////////////////////////////////////////
		////////////////////////////////////////
		
		/**
		 @brief get formated hebrew date.
		
		 return the short ( e.g. "1 Tishrey" ) or 
		 long (e.g. "Tuesday 18 Tishrey 5763 Hol hamoed Sukot" ) formated date.
		
		 @param diaspora if true give diaspora holydays.
		 @param s short flag.
		 @return a static string of foramted date
		*/
		char *
		get_format_date (int diaspora, int s)
		{
			return hdate_get_format_date (h, diaspora, s);
		}
		
		/**
		 @brief get name of week day.
		
		 @param s short flag 
		   true - returns a short string: sun, false returns: sunday.
		 @return a static string of the day of the week
		*/
		char *
		get_day_of_week_string (int s)
		{
			return hdate_get_day_string (h->hd_dw, s);
		}
		
		/**
		 @brief get name month.
		
		 @param s short flag 
		   true - returns a short string: Mar, false returns: March.
		 @return a static string of month name
		*/
		char *
		get_month_string (int s)
		{
			return hdate_get_month_string (h->gd_mon, s);
		}
		
		/**
		 @brief get name hebrew month.
		
		 @param s short flag 
		   true - returns a short string.
		 @return a static string of hebrew month name
		*/
		char *
		get_hebrew_month_string (int s)
		{
			return hdate_get_hebrew_month_string (h->hd_mon, s);
		}
		
		/**
		 @brief get name hebrew holiday.
		
		 @param s short flag 
		   true - returns a short string.
		 @return a static string of hebrew holiday name
		*/
		char *
		get_holyday_string (int diaspora, int s)
		{
			int holyday;
			
			holyday = hdate_get_holyday (h, diaspora);
			return hdate_get_holyday_string (holyday, s);
		}
		
		/**
		 @brief get the hebrew holiday type.
		
		 @return the holiday type 
			(1- regular holyday, 2- three regels, 3- tzom, 0- no holiday)
		*/
		int
		get_holyday_type (int diaspora)
		{
			int holyday;
			
			holyday = hdate_get_holyday (h, diaspora);
			return hdate_get_holyday_type (holyday);
		}
		
		/**
		 @brief get name hebrew parasha.
		
		 @param diaspora if true give diaspora parashot.
		 @param s short flag 
		   true - returns a short string.
		 @return a static string of hebrew parash name
		*/
		char *
		get_parasha_string (int diaspora, int s)
		{
			int parasha;
			
			parasha = hdate_get_parasha (h, diaspora);
			return hdate_get_parasha_string (parasha, s);
		}
		
		/**
		 @brief get name hebrew year.
		
		 @return a static string of hebrew year
		*/
		char *
		get_hebrew_year_string ()
		{
			return hdate_get_int_string (h->hd_year);
		}
		
		/**
		 @brief get name hebrew hebrew day of the month
		
		 @return a static string of hebrew day of the month
		*/
		char *
		get_hebrew_day_string ()
		{
			return hdate_get_int_string (h->hd_day);
		}
		
		////////////////////////////////////////
		////////////////////////////////////////
		
		/**
		 @brief get parash number
		
		 @param diaspora if true give diaspora parashot.
		 @return the hebrew parasha number
		*/
		int
		get_parasha (int diaspora)
		{
			return hdate_get_parasha (h, diaspora);
		}
		
		/**
		 @brief get holiday number
		
		 @param diaspora if true give diaspora holidays.
		 @return the hebrew holiday number
		*/
		int
		get_holyday (int diaspora)
		{
			return hdate_get_holyday (h, diaspora);
		}
		
		/**
		 @brief get Gregorian day of the month
		
		 @return the Gregorian day of the month
		*/
		int
		get_gday ()
		{
			return hdate_get_gday (h);
		}
		
		/**
		 @brief get Gregorian month
		
		 @return the Gregorian month
		*/
		int
		get_gmonth ()
		{
			return hdate_get_gmonth (h);
		}
		
		/**
		 @brief get Gregorian year
		
		 @return the Gregorian year
		*/
		int
		get_gyear ()
		{
			return hdate_get_gyear (h);
		}
		
		/**
		 @brief get Hebrew day of the month
		
		 @return the Hebrew day of the month
		*/
		int
		get_hday ()
		{
			return hdate_get_hday (h);
		}
		
		/**
		 @brief get Hebrew month
		
		 @return the Hebrew month
		*/
		int
		get_hmonth ()
		{
			return hdate_get_hmonth (h);
		}
		
		/**
		 @brief get Hebrew year
		
		 @return the Hebrew year
		*/
		int
		get_hyear ()
		{
			return hdate_get_hyear (h);
		}
		
		/**
		 @brief get the day of the week
		
		 @return the day of the week
		*/
		int
		get_day_of_the_week ()
		{
			return hdate_get_day_of_the_week (h);
		}
		
		/**
		 @brief get the size of the Hebrew year in days
		
		 @return the size of the Hebrew year in days
		*/
		int
		get_size_of_year ()
		{
			return hdate_get_size_of_year (h);
		}
		
		/**
		 @brief get the day of the week of hebrew new years
		
		 @return the day of the week of hebrew new years
		*/
		int
		get_new_year_day_of_the_week ()
		{
			return hdate_get_new_year_day_of_the_week (h);
		}
		
		/**
		 @brief get the Julian day number
		
		 @return the Julian day number
		*/
		int
		get_julian ()
		{
			return hdate_get_julian (h);
		}
		
		/**
		 @brief get the number of days sice Tishrey I
		
		 @return the the number of days sice Tishrey I
		*/
		int
		get_days ()
		{
			return hdate_get_days (h);
		}
		
		/**
		 @brief get the number of weeks sice Tishrey I
		
		 @return the the number of weeks sice Tishrey I
		*/
		int
		get_weeks ()
		{
			return hdate_get_weeks (h);
		}
		
		////////////////////////////////////////
		////////////////////////////////////////
		
		/**
		 @brief sunrise time
		
		 @param longitude longitude to use in calculations
			degrees, negative values are east
		 @param latitude latitude to use in calculations
			degrees, negative values are south
		 @param tz time zone
		 @return sunrise in minutes after midnight (00:00)
		 */
		int
		get_sunrise (double latitude, double longitude, int tz)
		{
			int sunrise;
			int sunset;
			
			hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, 
				latitude, longitude, 
				&sunrise, &sunset);
			
			return sunrise + tz * 60;
		}
		
		/**
		 @brief sunset time
		
		 @param longitude longitude to use in calculations
			degrees, negative values are east
		 @param latitude latitude to use in calculations
			degrees, negative values are south
		 @param tz time zone
		 @return sunset in minutes after midnight (00:00)
		 */
		int
		get_sunset (double latitude, double longitude, int tz)
		{
			int sunrise;
			int sunset;
			
			hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, 
				latitude, longitude, 
				&sunrise, &sunset);
			
			return sunset + tz * 60;
		}
		
	private:
		
		hdate_struct *h;
	
	};

} // name space

#endif
