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

#include "hdate.h"
#include "support.h"

/**
 @brief Return number of hebrew parasha.

 @param hebdate The hdate_struct of the date to use.
 @return the name of parasha 1. Bereshit etc..
 (55 trow 61 are joined strings e.g. Vayakhel Pekudei)
*/
int
hdate_get_parasha (hdate_struct * h)
{
	int join_flags[24][7] =
	{
		{1, 1, 1, 1, 0, 1, 1},/* 353 */
		{1, 1, 1, 1, 0, 1, 0},
		{1, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 0, 1, 0},
		{1, 1, 1, 1, 0, 1, 1},/* 354 */
		{1, 1, 1, 1, 0, 1, 1},
		{1, 1, 1, 0, 0, 1, 0},
		{1, 1, 1, 1, 0, 1, 0},
		{1, 1, 1, 1, 0, 1, 1},/* 355 */
		{1, 1, 1, 1, 0, 1, 1},
		{0, 1, 1, 1, 0, 1, 0},
		{1, 1, 1, 1, 0, 1, 1},
		{0, 0, 0, 0, 0, 1, 1},/* 383 */
		{0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0},/* 384 */
		{0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 1},
		{0, 0, 0, 0, 0, 1, 1},
		{0, 0, 0, 0, 0, 0, 0},/* 385 */
		{0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 1},
		{0, 0, 0, 0, 0, 1, 1}
	};

	int join_flag_22;
	int join_flag_27;
	int join_flag_29;
	int join_flag_32;
	int join_flag_39;
	int join_flag_42;
	int join_flag_51;
	
	int reading;

	int days;
	int weeks;

	join_flag_22 = join_flags[h->hd_year_type - 1][0];
	join_flag_27 = join_flags[h->hd_year_type - 1][1];
	join_flag_29 = join_flags[h->hd_year_type - 1][2];
	join_flag_32 = join_flags[h->hd_year_type - 1][3];
	join_flag_39 = join_flags[h->hd_year_type - 1][4];
	join_flag_42 = join_flags[h->hd_year_type - 1][5];
	join_flag_51 = join_flags[h->hd_year_type - 1][6];
	
	days = 0;
	if (h->hd_mon == 12)
	{			/* adar 1 */
		h->hd_mon = 5;
	}
	if (h->hd_mon == 13)
	{			/* adar 2 */
		h->hd_mon = 5;
		days = 30;
	}
	days += ((h->hd_mon) * 30 - (h->hd_mon) / 2 + h->hd_day + 1);
	if (h->hd_size_of_year % 10 > 4 && h->hd_mon > 2)	/* long Heshvan */
		days++;
	if (h->hd_size_of_year % 10 < 4 && h->hd_mon > 3)	/* short Kislev */
		days--;
	if (h->hd_size_of_year > 365 && h->hd_mon > 5)	/* leap year */
		days += 30;
	if (h->hd_new_year_dw == 7)
	{
		weeks = (days) / 7;
	}
	else
	{
		weeks = (days - h->hd_new_year_dw) / 7;
	}
	if (weeks == 0)
	{
		if (h->hd_new_year_dw == 7)
		{
			reading = 0;
			return reading;
		}
		else if ((h->hd_new_year_dw == 2) || (h->hd_new_year_dw == 3))
		{
			reading = 52;
			return reading;
		}
		else if (h->hd_new_year_dw == 5)
		{
			reading = 53;
			return reading;
		}
	}
	if (weeks == 1)
	{
		if (h->hd_new_year_dw == 5)
		{
			reading = 0;
			return reading;
		}
		else
		{
			reading = 53;
			return reading;
		}
	}
	if (weeks == 2)
	{
		reading = 0;
		return reading;
	}
	if (weeks == 3)
	{
		if (h->hd_new_year_dw == 7)
		{
			reading = 54;
			return reading;
		}
		else
		{
			reading = 1;
			return reading;
		}
	}
	if (weeks > 3)
	{
		reading = weeks - 1;
		if (h->hd_new_year_dw == 7)
			reading = reading - 2;
		if (h->hd_new_year_dw == 5)
			reading = reading - 1;
		if ((h->hd_new_year_dw == 2) || (h->hd_new_year_dw == 3))
			reading = reading - 1;
		if (reading < 22)
		{
			return reading;
		}
		/* joining */
		if (join_flag_22 && (reading >= 22))
		{
			if (reading == 22)
			{
				reading = 55;
				return reading;
			}
			else
			{
				reading++;
			}
		}

		/* pesach */
		if ((h->hd_mon == 6) && (h->hd_day > 13) && (h->hd_day < 21))
		{
			reading = 0;
			return reading;
		}
		if ((h->hd_mon == 6) && (h->hd_day > 20))
		{
			reading--;
		}
		if (h->hd_mon > 6)
		{
			reading--;
		}

		if (join_flag_27 && (reading >= 27))
		{
			if (reading == 27)
			{
				reading = 56;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		if (join_flag_29 && (reading >= 29))
		{
			if (reading == 29)
			{
				reading = 57;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		if (join_flag_32 && (reading >= 32))
		{
			if (reading == 32)
			{
				reading = 58;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		if (join_flag_39 && (reading >= 39))
		{
			if (reading == 39)
			{
				reading = 59;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		if (join_flag_42 && (reading >= 42))
		{
			if (reading == 42)
			{
				reading = 60;
				return reading;
			}
			else
			{
				reading++;
			}
		}
		if (join_flag_51 && (reading >= 51))
		{
			if (reading == 51)
			{
				reading = 61;
				return reading;
			}
			else
			{
				reading++;
			}
		}
	}

	return reading;
}
