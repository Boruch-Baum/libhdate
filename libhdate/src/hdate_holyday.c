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
 @brief Return number of hebrew holyday.

 @param h The hdate_struct of the date to use.
 @return the number of holyday.
*/
int
hdate_get_holyday (hdate_struct * h)
{
	int holyday;

	/* holydays table */
	static int holydays_table[14][30] = 
	{
		{	/* Tishrey */
			1, 2, 3, 3, 0, 0, 0, 0, 0, 4,
			0, 0, 0, 0, 5, 6, 6, 6, 6, 6,
			7, 8, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Heshvan */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Kislev */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 9, 9, 9, 9, 9, 9},
		{	/* Tevet */
			9, 9, 9, 0, 0, 0, 0, 0, 0, 10,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Shvat */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 11, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Adar */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			12, 0, 12, 13, 14, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Nisan */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 15, 16, 16, 16, 16, 16,
			16, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Iyar */
			0, 0, 17, 17, 17, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 18, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Sivan */
			0, 0, 0, 0, 19, 20, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Tamuz */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 21, 21, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Av */
			0, 0, 0, 0, 0, 0, 0, 0, 22, 22,
			0, 0, 0, 0, 23, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Elul */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Adar 1 */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{	/* Adar 2 */
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			12, 0, 12, 13, 14, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	};
	
	/* sanity check */
	if (h->hd_mon < 1 || h->hd_mon > 14 || h->hd_day < 0 || h->hd_day > 30)
		return 0;
	
	holyday = holydays_table[h->hd_mon - 1][h->hd_day - 1];
	
	/* if tzom on sat delay one day */
	/* yom cipur on sat */
	if ((holyday == 3) && (h->hd_dw == 7 || (h->hd_day == 4 && h->hd_dw !=1)))
		holyday = 0;
	/* 17 of Tanuz on sat */
	if ((holyday == 21) && ((h->hd_dw == 7) || (h->hd_day == 18 && h->hd_dw != 1)))
		holyday = 0;
	/* 9 of Av on sat */
	if ((holyday == 22) && ((h->hd_dw == 7) || (h->hd_day == 10 && h->hd_dw != 1)))
		holyday = 0;
	
	/* Hanukah in a long year */
    if ((holyday == 9) && (h->hd_size_of_year % 10 != 3) && (h->hd_day == 3))
		holyday = 0;
	
	/* if tanit ester on sat mov to Thu */
	if ((holyday == 12) && ((h->hd_dw == 7) || (h->hd_day == 11 && h->hd_dw != 5)))
		holyday = 0;
	
	/* yom ha azmaot on sat or fri mov to Thu*/
	if (holyday == 12)
	{
		if (h->hd_year < 1948)
			holyday = 0;
		else
		{
			if ((h->hd_day == 3) && (h->hd_dw != 5))
				holyday = 0;
            if ((h->hd_day == 4) && (h->hd_dw != 5))
                holyday = 0;
            if ((h->hd_day == 5) && (h->hd_dw == 6 || h->hd_dw == 7))
				holyday = 0;
		}
	}
	
	/* TODO: add holydays that are not in the table: yemey zicaron ... */
	
	/* defualt */
	return 0;
}

/**
 @brief Return number of hebrew holyday type.

 @param h The hdate_struct of the date to use.
 @return the number of holyday type.
*/
int
hdate_get_holyday_type (hdate_struct * h)
{
	int holyday_type;
	
	switch (hdate_get_holyday (h))
	{
	case 0: /* regular day */
		holyday_type = 0;
		break;
	case 5:
	case 15:
	case 20: /* 3 regels */
		holyday_type = 2;
		break;
	case 3:
	case 4: 
	case 10:
	case 12:
	case 21:
	case 22: /* tzom */
		holyday_type = 3;
		break;
	default: /* regular holyday */ 
		holyday_type = 1;
		break;
	}

	return holyday_type;
}
