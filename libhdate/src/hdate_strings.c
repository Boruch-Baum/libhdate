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
#include <locale.h>

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
	locale = setlocale (LC_MESSAGES, NULL);

	if ((locale != NULL) && (locale[0] == 'h') && (locale[1] == 'e'))
	{
		/* do nothing hebrew locale */
	}
	else
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
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif

	/* create a static days array */
	static char *days[2][7] = {
		{N_("Sun"), N_("Mon"), N_("Tue"), N_("Wed"), N_("Thu"),
		 N_("Fri"), N_("Sat")},
		{N_("Sunday"), N_("Monday"), N_("Tuesday"), N_("Wednesday"),
		 N_("Thusday"), N_("Friday"), N_("Saturday")}
	};

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
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif

	static char *months[2][12] = {
		{N_("January"), N_("February"), N_("March"),
		 N_("April"), N_("May"), N_("June"),
		 N_("July"), N_("August"), N_("September"),
		 N_("October"), N_("November"), N_("December")},
		{N_("Jan"), N_("Feb"), N_("Mar"), N_("Apr"), N_("May"),
		 N_("Jun"), N_("Jul"), N_("Aug"), N_("Sep"), N_("Oct"),
		 N_("Nov"), N_("Dec")}
	};

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
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif

	/* FIXME: abbrev of Hebrew months */
	static char *months[2][14] = {
		{N_("tishre"), N_("Heshvan"), N_("Kislev"), N_("Tevet"),
		 N_("Shvat"), N_("Adar"), N_("Nisan"), N_("Iyar"),
		 N_("Sivan"), N_("Tamuz"), N_("Av"), N_("Elul"), N_("Adar I"),
		 N_("Adar II")},
		{N_("tishre"), N_("Heshvan"), N_("Kislev"), N_("Tevet"),
		 N_("Shvat"), N_("Adar"), N_("Nisan"), N_("Iyar"),
		 N_("Sivan"), N_("Tamuz"), N_("Av"), N_("Elul"), N_("Adar I"),
		 N_("Adar II")}
	};

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
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif
	static char *holydays[2][23] = {
		{
		 N_("New year I"),
		 N_("New year II"),
		 N_("Gdalya fast"),
		 N_("Day of Cipurim"),
		 N_("Sukot"),
		 N_("Hol hamoed Sukot"),
		 N_("Hoshana raba"),
		 N_("Tora happiness"),
		 N_("Hanuka"),
		 N_("Ten of Tevet"),
		 N_("Tu be Shvat"),
		 N_("Ester fast"),
		 N_("Purim"),
		 N_("Shushan purim"),
		 N_("Pesach"),
		 N_("Hol hamoed Pesach"),
		 N_("Independence Day"),
		 N_("Lag ba omer"),
		 N_("Erev Shvoot"),
		 N_("Shvoot"),
		 N_("Seventeen of Tamuz fast"),
		 N_("Nine of Av fast"),
		 N_("Tu be Av")},
		{
		 N_("New year I"),
		 N_("New year II"),
		 N_("Gdalya fast"),
		 N_("Day of Cipurim"),
		 N_("Sukot"),
		 N_("Hol hamoed Sukot"),
		 N_("Hoshana raba"),
		 N_("Tora happiness"),
		 N_("Hanuka"),
		 N_("Ten of Tevet"),
		 N_("Tu be Shvat"),
		 N_("Ester fast"),
		 N_("Purim"),
		 N_("Shushan purim"),
		 N_("Pesach"),
		 N_("Hol hamoed Pesach"),
		 N_("Independence Day"),
		 N_("Lag ba omer"),
		 N_("Erev Shvoot"),
		 N_("Shvoot"),
		 N_("Seventeen of Tamuz fast"),
		 N_("Nine of Av fast"),
		 N_("Tu be Av")}
	};

	/* make sure s is 0 or 1 */
	s = s ? 0 : 1;

	if (holyday >= 1 && holyday <= 23)
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
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif

	/* FIXME: abbrev of parasha */
	static char *parashaot[2][62] = {
		{
		 N_("none"),
		 N_("Bereshit"),
		 N_("Noach"),
		 N_("Lech-Lecha"),
		 N_("Vayera"),
		 N_("Chayei-Sarah"),
		 N_("Toldot"),
		 N_("Vayetze"),
		 N_("Vayishlach"),
		 N_("Vayeshev"),
		 N_("Miketz"),
		 N_("Vayigash"),
		 N_("Vayechi"),
		 N_("Shemot"),
		 N_("Vaera"),
		 N_("Bo"),
		 N_("Beshalach"),
		 N_("Yitro"),
		 N_("Mishpatim"),
		 N_("Teruma"),
		 N_("Tetzave"),
		 N_("Ki-Tisa"),
		 N_("Vayakhel"),
		 N_("Pekudei"),
		 N_("Vayikra"),
		 N_("Tzav"),
		 N_("Shemini"),
		 N_("Tazri'a"),
		 N_("Metzora"),
		 N_("Acharei-Mot"),
		 N_("Kedoshim"),
		 N_("Emor"),
		 N_("Behar"),
		 N_("Bechukotai"),
		 N_("Bamidbar"),
		 N_("Naso"),
		 N_("Beha'alotcha"),
		 N_("Shelach"),
		 N_("Korach"),
		 N_("Chukat"),
		 N_("Balak"),
		 N_("Pinchas"),
		 N_("Matot"),
		 N_("Mase'ei"),
		 N_("Devarim"),
		 N_("Vaetchanan"),
		 N_("Ekev"),
		 N_("Re'e"),
		 N_("Shoftim"),
		 N_("Ki-Tetze"),
		 N_("Ki-Tavo"),
		 N_("Nitzavim"),
		 N_("Vayelech"),
		 N_("Ha'azinu"),
		 N_("Vezot-Habracha"),
		 N_("Vayakhel Pekudei"),
		 N_("Tazri'a Metzora"),
		 N_("Acharei-Mot Kedoshim"),
		 N_("Behar Bechukotai"),
		 N_("Chukat Balak"),
		 N_("Matot Mase'ei"),
		 N_("Nitzavim Vayelech")},
		{
		 N_("none"),
		 N_("Bereshit"),
		 N_("Noach"),
		 N_("Lech-Lecha"),
		 N_("Vayera"),
		 N_("Chayei-Sarah"),
		 N_("Toldot"),
		 N_("Vayetze"),
		 N_("Vayishlach"),
		 N_("Vayeshev"),
		 N_("Miketz"),
		 N_("Vayigash"),
		 N_("Vayechi"),
		 N_("Shemot"),
		 N_("Vaera"),
		 N_("Bo"),
		 N_("Beshalach"),
		 N_("Yitro"),
		 N_("Mishpatim"),
		 N_("Teruma"),
		 N_("Tetzave"),
		 N_("Ki-Tisa"),
		 N_("Vayakhel"),
		 N_("Pekudei"),
		 N_("Vayikra"),
		 N_("Tzav"),
		 N_("Shemini"),
		 N_("Tazri'a"),
		 N_("Metzora"),
		 N_("Acharei-Mot"),
		 N_("Kedoshim"),
		 N_("Emor"),
		 N_("Behar"),
		 N_("Bechukotai"),
		 N_("Bamidbar"),
		 N_("Naso"),
		 N_("Beha'alotcha"),
		 N_("Shelach"),
		 N_("Korach"),
		 N_("Chukat"),
		 N_("Balak"),
		 N_("Pinchas"),
		 N_("Matot"),
		 N_("Mase'ei"),
		 N_("Devarim"),
		 N_("Vaetchanan"),
		 N_("Ekev"),
		 N_("Re'e"),
		 N_("Shoftim"),
		 N_("Ki-Tetze"),
		 N_("Ki-Tavo"),
		 N_("Nitzavim"),
		 N_("Vayelech"),
		 N_("Ha'azinu"),
		 N_("Vezot-Habracha"),
		 N_("Vayakhel Pekudei"),
		 N_("Tazri'a Metzora"),
		 N_("Acharei-Mot Kedoshim"),
		 N_("Behar Bechukotai"),
		 N_("Chukat Balak"),
		 N_("Matot Mase'ei"),
		 N_("Nitzavim Vayelech")}
	};

	/* make sure s is 0 or 1 */
	s = s ? 0 : 1;

	if (parasha >= 1 && parasha <= 62)
	{
		return _(parashaot[s][parasha - 1]);
	}

	/* if not a valid month return NULL */
	return NULL;
}

/**
 @brief Return a static string, with the hebrew date.

 return the short ( e.g. "1 Tishrey" ) or 
 long (e.g. "Tuesday 18 Tishrey 5763 Hol hamoed Sukot" ) formated date.

 @param h The hdate_struct of the date to print.
 @param s A short flag (true - returns a short string, false returns a long string. ).
*/
char *
hdate_get_format_date (hdate_struct * h, int s)
{
	static char format_date[500];
	static char temp[500];

	/* you dont realy need it here */
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif

	if (s)
	{			/* short format */
		if (h != 0)
		{
			snprintf (format_date, 500,
				  "%s %s",
				  hdate_get_int_string (h->
							hd_day),
				  hdate_get_hebrew_month_string (h->hd_mon, s));

			return (format_date);
		}
		return NULL;
	}
	else
	{			/* long format */
		if (h != 0)
		{
			snprintf (format_date, 500, "%s %s",
				  hdate_get_day_string (h->
							hd_dw,
							s),
				  hdate_get_int_string (h->hd_day));
			snprintf (temp, 500, " %s %s ",
				  hdate_get_hebrew_month_string
				  (h->hd_mon, s),
				  hdate_get_int_string (h->hd_year));
			strncat (format_date, temp, 500);

			return (format_date);
		}
		return NULL;
	}

	return NULL;
}
