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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include "hdate.h"
#include "support.h"

/**
 @brief convert an integer to hebrew string UTF-8 (logical)
 
 @param n The int to convert

 @attention ( 0 < n < 10000)
 @warning uses a static string, so output should be copied away.
*/
char *
hdate_get_int_string (int n)
{
	int length;
	static char h_number[100];
	static char *digits[3][10] = {
		{" ", "א", "ב", "ג", "ד", "ה", "ו", "ז", "ח", "ט"},
		{"ט", "י", "כ", "ל", "מ", "נ", "ס", "ע", "פ", "צ"},
		{" ", "ק", "ר", "ש", "ת"}
	};

	char *locale;

	/* Get the name of the current locale.  */
#ifdef ENABLE_NLS
	locale = setlocale (LC_MESSAGES, NULL);
#else
	locale = NULL
#endif

	if ((!locale) || (locale[0] != 'h') || (locale[1] != 'e'))
	{
		/* not hebrew locale return the number in decimal form */
		snprintf (h_number, 100, "%d", n);
		return h_number;
	}

	/* sanity checks */
	if (n < 1 || n > 10000)
	{
		return NULL;
	}

	strncpy (h_number, "", 100);

	if (n >= 1000)
	{
		strncat (h_number, digits[0][n / 1000], 100);
		n %= 1000;
	}
	while (n >= 400)
	{
		strncat (h_number, digits[2][4], 100);
		n -= 400;
	}
	if (n >= 100)
	{
		strncat (h_number, digits[2][n / 100], 100);
		n %= 100;
	}
	if (n >= 10)
	{
		if (n == 15 || n == 16)
			n -= 9;
		strncat (h_number, digits[1][n / 10], 100);
		n %= 10;
	}
	if (n > 0)
		strncat (h_number, digits[0][n], 16);

	length = strlen (h_number);

	/* add the ' and " to hebrew numbers */
	if (length <= 2)
	{
		strncat (h_number, "'", 100);
	}
	else
	{
		h_number[length + 1] = h_number[length];
		h_number[length] = h_number[length - 1];
		h_number[length - 1] = h_number[length - 2];
		h_number[length - 2] = '\"';
	}

	return h_number;
}

/**
 @brief Return a static string, with name of wek day.

 @param day The number of the day 0..6 (0 - sun).
 @param s A short flag (true - returns a short string: sun, .., false returns: sunday, .. ).
*/
char *
hdate_get_day_string (int day, int s)
{
	/* create a static days array */
	static char *days[2][7] = {
		{N_("Sun"), N_("Mon"), N_("Tue"), N_("Wed"), N_("Thu"),
		 N_("Fri"), N_("Sat")},
		{N_("Sunday"), N_("Monday"), N_("Tuesday"), N_("Wednesday"),
		 N_("Thusday"), N_("Friday"), N_("Saturday")}
	};

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif
	
	/* make sure s is 0 or 1 */
	s = s ? 0 : 1;

	if (day >= 1 && day <= 7)
	{
		return _(days[s][day - 1]);
	}

	/* if not a valid day return NULL */
	return NULL;
}

/**
 @brief Return a static string, with name of month.

 @param month The number of the month 1..12 (1 - jan).
 @param s A short flag.
*/
char *
hdate_get_month_string (int month, int s)
{
	
	static char *months[2][12] = {
		{N_("Jan"), N_("Feb"), N_("Mar"), N_("Apr"), N_("May"),
		 N_("Jun"), N_("Jul"), N_("Aug"), N_("Sep"), N_("Oct"),
		 N_("Nov"), N_("Dec")},
		{N_("January"), N_("February"), N_("March"),
		 N_("April"), N_("May"), N_("June"),
		 N_("July"), N_("August"), N_("September"),
		 N_("October"), N_("November"), N_("December")}
	};

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif
	
	/* make sure s is 0 or 1 */
	s = s ? 0 : 1;

	if (month >= 1 && month <= 12)
	{
		return _(months[s][month - 1]);
	}

	/* if not a valid month return NULL */
	return NULL;
}

/**
 @brief Return a static string, with name of hebrew month.

 @param month The number of the month 1..14 (1 - tishre, 13 - adar 1, 14 - adar 2).
 @param s A short flag.
*/
char *
hdate_get_hebrew_month_string (int month, int s)
{
	/* FIXME: abbrev of Hebrew months */
	static char *months[2][14] = {
		{N_("Tishrei"), N_("Cheshvan"), N_("Kislev"), N_("Tevet"),
		 N_("Sh'vat"), N_("Adar"), N_("Nisan"), N_("Iyyar"),
		 N_("Sivan"), N_("Tamuz"), N_("Av"), N_("Elul"), N_("Adar I"),
		 N_("Adar II")},
		{N_("Tishrei"), N_("Cheshvan"), N_("Kislev"), N_("Tevet"),
		 N_("Sh'vat"), N_("Adar"), N_("Nisan"), N_("Iyyar"),
		 N_("Sivan"), N_("Tamuz"), N_("Av"), N_("Elul"), N_("Adar I"),
		 N_("Adar II")}
	};

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif
	
	/* make sure s is 0 or 1 */
	s = s ? 0 : 1;

	if (month >= 1 && month <= 14)
	{
		return _(months[s][month - 1]);
	}

	/* if not a valid month return NULL */
	return NULL;
}

/**
 @brief Name of hebrew holyday.

 @param holyday The holyday number.
 @param s A short flag.
*/
char *
hdate_get_holyday_string (int holyday, int s)
{
	/* holyday strings */
	static char *holydays[2][26] = {
		{
		 N_("Rosh Hashana I"),
		 N_("Rosh Hashana II"),
		 N_("Tzom Gedaliah"),
		 N_("Yom Kippur"),
		 N_("Sukkot"),
		 N_("Hol hamoed Sukkot"),
		 N_("Hoshana raba"),
		 N_("Simchat Torah"),
		 N_("Chanukah"),
		 N_("Asara B'Tevet"),
		 N_("Tu B'Shvat"),
		 N_("Ta'anit Esther"),
		 N_("Purim"),
		 N_("Shushan Purim"),
		 N_("Pesach"),
		 N_("Hol hamoed Pesach"),
		 N_("Yom HaAtzma'ut"),
		 N_("Lag B'Omer"),
		 N_("Erev Shavuot"),
		 N_("Shavuot"),
		 N_("Tzom Tammuz"),
		 N_("Tish'a B'Av"),
		 N_("Tu B'Av"),
		 N_("Yom HaShoah"),
		 N_("Yom HaZikaron"),
		 N_("Yom Yerushalayim")},
		{
		 N_("Rosh Hashana I"),
		 N_("Rosh Hashana II"),
		 N_("Tzom Gedaliah"),
		 N_("Yom Kippur"),
		 N_("Sukkot"),
		 N_("Hol hamoed Sukkot"),
		 N_("Hoshana raba"),
		 N_("Simchat Torah"),
		 N_("Chanukah"),
		 N_("Asara B'Tevet"),
		 N_("Tu B'Shvat"),
		 N_("Ta'anit Esther"),
		 N_("Purim"),
		 N_("Shushan Purim"),
		 N_("Pesach"),
		 N_("Hol hamoed Pesach"),
		 N_("Yom HaAtzma'ut"),
		 N_("Lag B'Omer"),
		 N_("Erev Shavuot"),
		 N_("Shavuot"),
		 N_("Tzom Tammuz"),
		 N_("Tish'a B'Av"),
		 N_("Tu B'Av"),
		 N_("Yom HaShoah"),
		 N_("Yom HaZikaron"),
		 N_("Yom Yerushalayim")}
	};

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif
	
	/* make sure s is 0 or 1 */
	s = s ? 0 : 1;

	if (holyday >= 1 && holyday <= 26)
	{
		return _(holydays[s][holyday - 1]);
	}

	/* if not a valid holyday return NULL */
	return NULL;
}

/**
 @brief Name of Parasha

 @param parasha The Number of Parasha 1-Bereshit
	(55 trow 61 are joined strings e.g. Vayakhel Pekudei)
 @param s A short flag.
*/
char *
hdate_get_parasha_string (int parasha, int s)
{
	/* FIXME: abbrev of parasha */
	static char *parashaot[2][62] = {
		{
		 N_("none"),
		 N_("Bereshit"), /* 1 */
		 N_("Noach"),
		 N_("Lech-Lecha"),
		 N_("Vayera"),
		 N_("Chayei Sara"),
		 N_("Toldot"),
		 N_("Vayetzei"),
		 N_("Vayishlach"),
		 N_("Vayeshev"),
		 N_("Miketz"), /* 10 */
		 N_("Vayigash"),
		 N_("Vayechi"),
		 N_("Shemot"),
		 N_("Vaera"),
		 N_("Bo"),
		 N_("Beshalach"),
		 N_("Yitro"),
		 N_("Mishpatim"),
		 N_("Terumah"),
		 N_("Tetzaveh"), /* 20 */
		 N_("Ki Tisa"),
		 N_("Vayakhel"),
		 N_("Pekudei"),
		 N_("Vayikra"),
		 N_("Tzav"),
		 N_("Shmini"),
		 N_("Tazria"),
		 N_("Metzora"),
		 N_("Achrei Mot"),
		 N_("Kedoshim"), /* 30 */
		 N_("Emor"),
		 N_("Behar"),
		 N_("Bechukotai"),
		 N_("Bamidbar"),
		 N_("Nasso"),
		 N_("Beha'alotcha"),
		 N_("Sh'lach"),
		 N_("Korach"),
		 N_("Chukat"),
		 N_("Balak"), /* 40 */
		 N_("Pinchas"),
		 N_("Matot"),
		 N_("Masei"),
		 N_("Devarim"),
		 N_("Vaetchanan"),
		 N_("Eikev"),
		 N_("Re'eh"),
		 N_("Shoftim"),
		 N_("Ki Teitzei"),
		 N_("Ki Tavo"), /* 50 */
		 N_("Nitzavim"),
		 N_("Vayeilech"),
		 N_("Ha'Azinu"),
		 N_("Vezot Habracha"), /* 54 */
		 N_("Vayakhel-Pekudei"),
		 N_("Tazria-Metzora"),
		 N_("Achrei Mot-Kedoshim"),
		 N_("Behar-Bechukotai"),
		 N_("Chukat-Balak"),
		 N_("Matot-Masei"),
		 N_("Nitzavim-Vayeilech")},
		{
		 N_("none"),
		 N_("Bereshit"), /* 1 */
		 N_("Noach"),
		 N_("Lech-Lecha"),
		 N_("Vayera"),
		 N_("Chayei Sara"),
		 N_("Toldot"),
		 N_("Vayetzei"),
		 N_("Vayishlach"),
		 N_("Vayeshev"),
		 N_("Miketz"), /* 10 */
		 N_("Vayigash"),
		 N_("Vayechi"),
		 N_("Shemot"),
		 N_("Vaera"),
		 N_("Bo"),
		 N_("Beshalach"),
		 N_("Yitro"),
		 N_("Mishpatim"),
		 N_("Terumah"),
		 N_("Tetzaveh"), /* 20 */
		 N_("Ki Tisa"),
		 N_("Vayakhel"),
		 N_("Pekudei"),
		 N_("Vayikra"),
		 N_("Tzav"),
		 N_("Shmini"),
		 N_("Tazria"),
		 N_("Metzora"),
		 N_("Achrei Mot"),
		 N_("Kedoshim"), /* 30 */
		 N_("Emor"),
		 N_("Behar"),
		 N_("Bechukotai"),
		 N_("Bamidbar"),
		 N_("Nasso"),
		 N_("Beha'alotcha"),
		 N_("Sh'lach"),
		 N_("Korach"),
		 N_("Chukat"),
		 N_("Balak"), /* 40 */
		 N_("Pinchas"),
		 N_("Matot"),
		 N_("Masei"),
		 N_("Devarim"),
		 N_("Vaetchanan"),
		 N_("Eikev"),
		 N_("Re'eh"),
		 N_("Shoftim"),
		 N_("Ki Teitzei"),
		 N_("Ki Tavo"), /* 50 */
		 N_("Nitzavim"),
		 N_("Vayeilech"),
		 N_("Ha'Azinu"),
		 N_("Vezot Habracha"), /* 54 */
		 N_("Vayakhel-Pekudei"),
		 N_("Tazria-Metzora"),
		 N_("Achrei Mot-Kedoshim"),
		 N_("Behar-Bechukotai"),
		 N_("Chukat-Balak"),
		 N_("Matot-Masei"),
		 N_("Nitzavim-Vayeilech")}
	};

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif
	
	/* make sure s is 0 or 1 */
	s = s ? 0 : 1;

	if (parasha >= 0 && parasha <= 61)
	{
		return _(parashaot[s][parasha]);
	}

	/* if not a valid parasha return NULL */
	return NULL;
}

/**
 @brief Return a static string, with the hebrew date.

 return the short ( e.g. "1 Tishrey" ) or 
 long (e.g. "Tuesday 18 Tishrey 5763 Hol hamoed Sukot" ) formated date.

 @param h The hdate_struct of the date to print.
 @param diaspora if true give diaspora holydays
 @param s A short flag (true - returns a short string, false returns a long string. ).
*/
char *
hdate_get_format_date (hdate_struct * h, int diaspora, int s)
{
	static char format_date[500];
	static char temp[500];
	int holyday;
	
	if (h)
	{
		if (s)
		{	/* short format */
			snprintf (format_date, 500, "%s %s",
				  hdate_get_int_string (h->hd_day),
				  hdate_get_hebrew_month_string (h->hd_mon, s));
			return (format_date);
		}
		else
		{
			snprintf (temp, 500, "%s, %s %s",
				  hdate_get_day_string (h->hd_dw, s),
				  hdate_get_int_string (h->hd_day),
				  hdate_get_hebrew_month_string (h->hd_mon, s));
			snprintf (format_date, 500, "%s %s",
				  temp,
				  hdate_get_int_string (h->hd_year));
			
			/* if holyday print it */
			holyday = hdate_get_holyday (h, diaspora);
			
			if (holyday != 0)
			{
				snprintf (temp, 500, "%s, %s",
				  format_date,
				  hdate_get_holyday_string (holyday, 0));
				
				return (temp);
			}
			else
				return (format_date);
		}
	}

	return NULL;
}
