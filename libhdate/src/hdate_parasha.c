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

#define FALSE 0
#define TRUE  1

/**
 @brief Return number of hebrew parasha.

 @param hebdate The hdate_struct of the date to use.
 @return the name of parasha 1. Bereshit etc..
 (55 trow 61 are joined strings e.g. Vayakhel Pekudei)
*/
int
hdate_get_parasha (hdate_struct * hebdate)
{
        hdate_struct h, g;
        int size_of_year;
        int first_day_of_year;
        int join_flag_22;
        int join_flag_27;
        int join_flag_29;
        int join_flag_32;
        int join_flag_39;
        int join_flag_42;
        int join_flag_51;

        int type_of_year;
        int reading;

        int days;
        int weeks;

        h = *hebdate;

        if (h.hd_dw != 7)
                return 0;

        g = *hdate_gdate (1, 1, h.hd_year);
        size_of_year = hdate_size_of_hebrew_year (h.hd_year);
        first_day_of_year = g.hd_dw;

        if (size_of_year == 353)
                {
                        if (first_day_of_year == 2)
                                {
                                        type_of_year = 1;
                                        join_flag_22 = TRUE;
                                        join_flag_27 = TRUE;
                                        join_flag_29 = TRUE;
                                        join_flag_32 = TRUE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = TRUE;
                                }
                        if (first_day_of_year == 7)
                                {
                                        type_of_year = 2;
                                        join_flag_22 = TRUE;
                                        join_flag_27 = TRUE;
                                        join_flag_29 = TRUE;
                                        join_flag_32 = TRUE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = FALSE;
                                }
                }
        if (size_of_year == 354)
                {
                        if (first_day_of_year == 2)
                                {
                                        type_of_year = 3;
                                        join_flag_22 = TRUE;
                                        join_flag_27 = TRUE;
                                        join_flag_29 = TRUE;
                                        join_flag_32 = TRUE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = TRUE;
                                }
                        if (first_day_of_year == 3)
                                {
                                        type_of_year = 3;
                                        join_flag_22 = TRUE;
                                        join_flag_27 = TRUE;
                                        join_flag_29 = TRUE;
                                        join_flag_32 = TRUE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = TRUE;
                                }
                        if (first_day_of_year == 5)
                                {
                                        type_of_year = 4;
                                        join_flag_22 = TRUE;
                                        join_flag_27 = TRUE;
                                        join_flag_29 = TRUE;
                                        join_flag_32 = FALSE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = FALSE;
                                }
                        if (first_day_of_year == 7)
                                {
                                        type_of_year = 2;
                                        join_flag_22 = TRUE;
                                        join_flag_27 = TRUE;
                                        join_flag_29 = TRUE;
                                        join_flag_32 = TRUE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = FALSE;
                                }
                }
        if (size_of_year == 355)
                {
                        if (first_day_of_year == 2)
                                {
                                        type_of_year = 5;
                                        join_flag_22 = TRUE;
                                        join_flag_27 = TRUE;
                                        join_flag_29 = TRUE;
                                        join_flag_32 = TRUE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = TRUE;
                                }
                        if (first_day_of_year == 5)
                                {
                                        type_of_year = 6;
                                        join_flag_22 = FALSE;
                                        join_flag_27 = TRUE;
                                        join_flag_29 = TRUE;
                                        join_flag_32 = TRUE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = FALSE;
                                }
                        if (first_day_of_year == 7)
                                {
                                        type_of_year = 7;
                                        join_flag_22 = TRUE;
                                        join_flag_27 = TRUE;
                                        join_flag_29 = TRUE;
                                        join_flag_32 = TRUE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = TRUE;
                                }
                }

        if (size_of_year == 383)
                {
                        if (first_day_of_year == 2)
                                {
                                        type_of_year = 8;
                                        join_flag_22 = FALSE;
                                        join_flag_27 = FALSE;
                                        join_flag_29 = FALSE;
                                        join_flag_32 = FALSE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = TRUE;
                                }
                        if (first_day_of_year == 5)
                                {
                                        type_of_year = 9;
                                        join_flag_22 = FALSE;
                                        join_flag_27 = FALSE;
                                        join_flag_29 = FALSE;
                                        join_flag_32 = FALSE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = FALSE;
                                        join_flag_51 = FALSE;
                                }

                }
        if (size_of_year == 384)
                {
                        if (first_day_of_year == 3)
                                {
                                        type_of_year = 11;
                                        join_flag_22 = FALSE;
                                        join_flag_27 = FALSE;
                                        join_flag_29 = FALSE;
                                        join_flag_32 = FALSE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = FALSE;
                                        join_flag_51 = FALSE;
                                }
                        if (first_day_of_year == 7)
                                {
                                        type_of_year = 10;
                                        join_flag_22 = FALSE;
                                        join_flag_27 = FALSE;
                                        join_flag_29 = FALSE;
                                        join_flag_32 = FALSE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = TRUE;
                                }
                        if (first_day_of_year == 5)
                                {
                                        type_of_year = 13;
                                        join_flag_22 = FALSE;
                                        join_flag_27 = FALSE;
                                        join_flag_29 = FALSE;
                                        join_flag_32 = FALSE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = FALSE;
                                        join_flag_51 = TRUE;
                                }
                }
        if (size_of_year == 385)
                {
                        if (first_day_of_year == 2)
                                {
                                        type_of_year = 12;
                                        join_flag_22 = FALSE;
                                        join_flag_27 = FALSE;
                                        join_flag_29 = FALSE;
                                        join_flag_32 = FALSE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = FALSE;
                                        join_flag_51 = FALSE;
                                }
                        if (first_day_of_year == 5)
                                {
                                        type_of_year = 13;
                                        join_flag_22 = FALSE;
                                        join_flag_27 = FALSE;
                                        join_flag_29 = FALSE;
                                        join_flag_32 = FALSE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = FALSE;
                                        join_flag_51 = TRUE;
                                }
                        if (first_day_of_year == 7)
                                {
                                        type_of_year = 14;
                                        join_flag_22 = FALSE;
                                        join_flag_27 = FALSE;
                                        join_flag_29 = FALSE;
                                        join_flag_32 = FALSE;
                                        join_flag_39 = FALSE;
                                        join_flag_42 = TRUE;
                                        join_flag_51 = TRUE;
                                }
                }

        days = 0;
        if (h.hd_mon == 12)
                {               /* adar 1 */
                        h.hd_mon = 5;
                }
        if (h.hd_mon == 13)
                {               /* adar 2 */
                        h.hd_mon = 5;
                        days = 30;
                }
        days += ((h.hd_mon) * 30 - (h.hd_mon) / 2 + h.hd_day + 1);
        if (size_of_year % 10 > 4 && h.hd_mon > 2)      /* long Heshvan */
                days++;
        if (size_of_year % 10 < 4 && h.hd_mon > 3)      /* short Kislev */
                days--;
        if (size_of_year > 365 && h.hd_mon > 5) /* leap year */
                days += 30;
        if (first_day_of_year == 7)
                {
                        weeks = (days) / 7;
                }
        else
                {
                        weeks = (days - first_day_of_year) / 7;
                }
        if (weeks == 0)
                {
                        if (first_day_of_year == 7)
                                {
                                        reading = 0;
                                        return reading;
                                }
                        else if ((first_day_of_year == 2)
                                 || (first_day_of_year == 3))
                                {
                                        reading = 52;
                                        return reading;
                                }
                        else if (first_day_of_year == 5)
                                {
                                        reading = 53;
                                        return reading;
                                }
                }
        if (weeks == 1)
                {
                        if (first_day_of_year == 5)
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
                        if (first_day_of_year == 7)
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
                        if (first_day_of_year == 7)
                                reading = reading - 2;
                        if (first_day_of_year == 5)
                                reading = reading - 1;
                        if ((first_day_of_year == 2)
                            || (first_day_of_year == 3))
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
                        if ((h.hd_mon == 6) && (h.hd_day > 13)
                            && (h.hd_day < 21))
                                {
                                        reading = 0;
                                        return reading;
                                }
                        if ((h.hd_mon == 6) && (h.hd_day > 20))
                                {
                                        reading--;
                                }
                        if (h.hd_mon > 6)
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
