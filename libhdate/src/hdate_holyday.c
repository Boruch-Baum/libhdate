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
        int month, day, day_of_week;
              
        month = h->hd_mon - 1;
        day = h->hd_day;
        day_of_week = h->hd_dw - 1;

        switch (month)
                {
                case 0:        /* Tishrey */
                        if (day == 1)
                                return 1;
                        if (day == 2)
                                return 2;
                        if ((day == 3) && (day_of_week != 7))
                                return 3;
                        if ((day == 4) && (day_of_week == 1))
                                return 3;
                        if (day == 10)
                                return 4;
                        if (day == 15)
                                return 5;
                        if ((day > 15) && (day < 21))
                                return 6;
                        if (day == 21)
                                return 7;
                        if (day == 22)
                                return 8;
                        return 0;
                        break;
                case 1:        /* Heshvan */
                        return 0;
                        break;
                case 2:        /* Kislev */
                        if ((day > 24) && (day < 32))
                                return 9;
                        return 0;
                        break;
                case 3:        /* Tevet */
                        if (day < 3)
                                return 9;
			/* Hanukah in a short year */
                        if ((h->hd_size_of_year % 10 == 3) && (day == 3))
                                return 9;
                        if ((day == 10) && (day_of_week != 7))
                                return 10;
                        return 0;
                        break;
                case 4:        /* Shvat */
                         if (day == 15)
                                return 11;
                        return 0;
                        break;
                case 5:        /* Adar */
                        if ((day == 11) && (day_of_week == 5))
                                return 12;
                        if ((day == 13) && (day_of_week != 7))
                                return 12;
                        if (day == 14)
                                return 13;
                        if (day == 15)
                                return 14;
                        return 0;
                        break;
                case 6:        /* Nisan */
                        if (day == 15)
                                return 15;
                        if ((day > 15) && (day < 22))
                                return 16;
                        return 0;
                        break;
                case 7:        /* Iyar */
                        if (h->hd_year >= 1948)
                                {
                                        if ((day == 3) && (day_of_week == 5))
                                                return 17;
                                        if ((day == 4) && (day_of_week == 5))
                                                return 17;
                                        if ((day == 5) && (day_of_week != 6)
                                            && (day_of_week != 7))
                                                return 17;
                                }
                        if (day == 18)
                                return 18;
                        return 0;
                        break;
                case 8:        /* Sivan */
                        if (day == 5)
                                return 19;
                        if (day == 6)
                                return 20;
                        return 0;
                        break;
                case 9:        /* Tamuz */
                        if ((day == 17) && (day_of_week != 7))
                                return 21;
                        if ((day == 18) && (day_of_week == 1))
                                return 21;
                        return 0;
                        break;
                case 10:       /* Av */
                        if ((day == 9) && (day_of_week != 7))
                                return 22;
                        if ((day == 10) && (day_of_week == 1))
                                return 22;
                        if (day == 15)
                                return 23;
                        return 0;
                        break;
                case 11:       /* Elul */
                        return 0;
                        break;
                case 12:       /* Adar I */
                        return 0;
                        break;
                case 13:       /* Adar II */
                        if ((day == 11) && (day_of_week == 5))
                                return 12;
                        if ((day == 13) && (day_of_week != 7))
                                return 12;
                        if (day == 14)
                                return 13;
                        if (day == 15)
                                return 14;
                        return 0;
                        break;
                default:
                        return 0;
                }
}
