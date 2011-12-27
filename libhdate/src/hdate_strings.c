/*  libhdate - Hebrew calendar library
 *
 *  Copyright (C) 1984-2003 Amos Shapir, 2004-2007  Yaacov Zamir <kzamir@walla.co.il>
 *                2011 Boruch Baum
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
 @brief helper function to find hebrew locale
 
 @return 0 - latin locale, -1 - hebrew locale
*/
int
hdate_is_hebrew_locale()
{
	char *locale;
	char *language;

	/* Get the name of the current locale.  */
#ifdef ENABLE_NLS
	locale = setlocale (LC_MESSAGES, NULL);
	language = getenv ("LANGUAGE");
#else
	locale = NULL;
	language = NULL;
#endif

	if (!((locale && (locale[0] == 'h') && (locale[1] == 'e')) ||
		  (language && (language[0] == 'h') && (language[1] == 'e'))))
	{
		/* not hebrew locale return false */
		return 0;
	}
	
	return -1;
}
/**
 @brief convert an integer to hebrew string UTF-8 (logical)
 
 @param n The int to convert

 @attention ( 0 < n < 11000)
 @warning This function is now just a wrapper, and is subject to
		  either deprecation or to have its calling definition
		  changed to match hdate_get_int_string_.
		  Callers to this function must free() after use the memory
		  pointed to by the return value.
          The original function outputted to a local static string,
          and suggested that the caller copied it away. The shiny,
          new version improves thread-safety by requiring the caller
		  to pass a buffer of size HEBREW_NUMBER_BUFFER_SIZE into
		  which we ourput.
		  It also adds an option to output a compressed number,
		  without apostrophes or quotation marks.
*/
char *
hdate_get_int_string (int n)
{
	char *dest;
	int hebrew_form = TRUE;

	dest = malloc(HEBREW_NUMBER_BUFFER_SIZE);
	if (!hdate_is_hebrew_locale()) hebrew_form = FALSE;
	return hdate_get_int_string_(dest, n, 0, hebrew_form);
}

/**
 @brief convert an integer to hebrew string UTF-8 (logical)

 @param *dest pointer to a buffer of size HEBREW_NUMBER_BUFFER_SIZE
 
 @param n The int to convert

 @param opt_compressed don't include apostrophes and quotes

 @param hebrew_form (as opposed to arabic numerals)

 @attention ( 0 < n < 11000)
 @warning This was originally written using a local static string,
          calling for output to be copied away.  The shiny,
          new version improves thread-safety by requiring the caller
		  to pass a buffer of size HEBREW_NUMBER_BUFFER_SIZE into
		  which we ourput.
		  It also adds an option to output a compressed number,
		  without apostrophes or quotation marks.
*/
char *
hdate_get_int_string_ (char *dest, int n, int opt_compressed, int hebrew_form)
{
	/***********************************************************
	* How large should the buffer be? Hebrew year 10,999 would
	* be י'תתקצ"ט, eight characters, each two bytes, plus an
	* end-of-string delimiter, equals 17. This could effectively
	* yield a range extending to Hebrew year 11,899, י"א תתצ"ט,
	* due to the extra ק needed for the '900' century. However,
	* for readability, I would want a an extra space at that
	* point between the millenium and the century...
	***********************************************************/
	// #define HEBREW_NUMBER_BUFFER_SIZE 17	// done in hdate.h
	#define H_CHAR_WIDTH 2
	int length;
	static char *digits[3][10] = {
		{" ", "א", "ב", "ג", "ד", "ה", "ו", "ז", "ח", "ט"},
		{"ט", "י", "כ", "ל", "מ", "נ", "ס", "ע", "פ", "צ"},
		{" ", "ק", "ר", "ש", "ת"}
	};


	// bounds check
	if (n < 1 || n > 10999)	return NULL;


	// not hebrew locale - return the number in decimal form
	if (!hebrew_form)
	{
		snprintf (dest, HEBREW_NUMBER_BUFFER_SIZE, "%d", n);
		return dest;
	}

	// Why?? - for the null?
	//strncpy (dest, "", H_CHAR_WIDTH);
	dest[0] = '\0';

	if (n >= 1000)
	{
		strncat (dest, digits[0][n / 1000], H_CHAR_WIDTH);
		n %= 1000;
	}
	while (n >= 400)
	{
		strncat (dest, digits[2][4], H_CHAR_WIDTH);
		n -= 400;
	}
	if (n >= 100)
	{
		strncat (dest, digits[2][n / 100], H_CHAR_WIDTH);
		n %= 100;
	}
	if (n >= 10)
	{
		if (n == 15 || n == 16)
			n -= 9;
		strncat (dest, digits[1][n / 10], H_CHAR_WIDTH);
		n %= 10;
	}
	if (n > 0)
		strncat (dest, digits[0][n], H_CHAR_WIDTH);
		
 	// possibly add the ' and " to hebrew numbers	
	if (!opt_compressed)
	{
		length = strlen (dest);
		if (length <= H_CHAR_WIDTH) strncat (dest, "'", H_CHAR_WIDTH);
		else
		{
			dest[length + 1] = dest[length];
			dest[length] = dest[length - 1];
			dest[length - 1] = dest[length - 2];
			dest[length - 2] = '\"';
			dest[length + 2] = '\0';
		}
	}
#define DEBUG 0
#if DEBUG
	length = strlen (dest);
	printf("\nhebrew integer value = %d, string length = %d, string =%s\n",n,length,dest);
	int ii; for (ii=0; ii<(length+3); ii++) printf("%x ",dest[ii]); printf("\n");
#endif
	return dest;
}

/**
 @brief Return a static string, with name of wek day.

 @param day_of_week The number of the day 0..6 (0 - sun).
 @param short_form A short flag (true - returns a short string: sun, .., false returns: sunday, .. ).
*/
char *
hdate_get_day_string (int day_of_week, int short_form)
{
	return hdate_string( HDATE_STRING_DOW, day_of_week, short_form, HDATE_STRING_LOCAL);
}

/**
 @brief Return a static string, with name of month.

 @param month The number of the month 1..12 (1 - jan).
 @param short_form A short flag.
*/
char *
hdate_get_month_string (int month, int short_form)
{
	return hdate_string( HDATE_STRING_GMONTH, month, short_form, HDATE_STRING_LOCAL);
}

/**
 @brief Return a static string, with name of hebrew month.

 @param month The number of the month 1..14 (1 - tishre, 13 - adar 1, 14 - adar 2).
 @param short_form A short flag.
*/
char *
hdate_get_hebrew_month_string (int month, int short_form)
{
	return hdate_string( HDATE_STRING_HMONTH, month, short_form, HDATE_STRING_LOCAL);
}

/**
 @brief Name of hebrew holyday.

 @param holyday The holyday number.
 @param short_text A short flag. 0=true, !0=false
*/
char *
hdate_get_holyday_string (int holiday, int short_text)
{
	return hdate_string( HDATE_STRING_HOLIDAY, holiday, short_text, HDATE_STRING_LOCAL);
}

/**
 @brief Return a static string, with the day in the omer

 @param omer day The day in the omer.
 @return a static string, with the day in the omer
*/
char *
hdate_get_omer_string (int omer_day)
{
	// BUG - should not be a static buffer
	static char omer_string[500];
	
	/* this is not a day in the omer */
	if (omer_day < 1 || omer_day > 49)
		return NULL;
	
	/* create a nice string */
	snprintf(omer_string, 500, "%s %s",
					hdate_get_int_string(omer_day),
					_("in the Omer"));
	
	/* return the string */
	return omer_string;
}

/**
 @brief Name of Parasha

 @param parasha The Number of Parasha 1-Bereshit
	(55 trow 61 are joined strings e.g. Vayakhel Pekudei)
 @param short_form A short flag. 0=true, !0 = false
*/
char *
hdate_get_parasha_string (int parasha, int short_form)
{
	return hdate_string( HDATE_STRING_PARASHA, parasha, short_form, HDATE_STRING_LOCAL);
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
hdate_get_format_date (hdate_struct const *h, int diaspora, int s)
{
	
	static char format_date[500];
	static char temp[500];
	int holiday;
	int omer_day;
	
	if (h)
	{
		if (s)
		{						
			/* short format */
			/* if hebrew add bet to the month */
			if (hdate_is_hebrew_locale())
				snprintf (format_date, 500, "%s ב%s",
					hdate_get_int_string (h->hd_day),
					hdate_get_hebrew_month_string (h->hd_mon, s));
			else
				snprintf (format_date, 500, "%s %s",
					hdate_get_int_string (h->hd_day),
					hdate_get_hebrew_month_string (h->hd_mon, s));
			
			return (format_date);
		}
		else
		{
			/* if hebrew add bet to the month */
			if (hdate_is_hebrew_locale())
				snprintf (temp, 500, "%s, %s ב%s",
					hdate_get_day_string (h->hd_dw, s),
					hdate_get_int_string (h->hd_day),
					hdate_get_hebrew_month_string (h->hd_mon, s));
			else
				snprintf (temp, 500, "%s, %s %s",
					hdate_get_day_string (h->hd_dw, s),
					hdate_get_int_string (h->hd_day),
					hdate_get_hebrew_month_string (h->hd_mon, s));
					
			snprintf (format_date, 500, "%s %s",
					temp, hdate_get_int_string (h->hd_year));

			/* if a day in the omer print it */
			omer_day = hdate_get_omer_day(h);
			
			if (omer_day != 0)
				snprintf (temp, 500, "%s, %s",
						format_date, hdate_get_omer_string (omer_day));
			else
				snprintf (temp, 500, "%s", format_date);
			
			/* if holiday print it */
			holiday = hdate_get_holyday (h, diaspora);
			
			if (holiday != 0)
				snprintf (temp, 500, "%s, %s",
						format_date,
			  			hdate_string( HDATE_STRING_HOLIDAY, holiday, HDATE_STRING_LONG, HDATE_STRING_LOCAL));
			else
				snprintf (temp, 500, "%s", format_date);
			
			return (temp);
		}
	}

	return NULL;
}

/**
 @brief Return a static string, with the package name and version

 @return a a static string, with the package name and version
*/
char *
hdate_get_version_string ()
{
	static char version[500];

	/* make a "packge version" string */
//	snprintf (version, 500, "%s %s", PACKAGE, VERSION);

	return (version);
}

/**
 @brief Return a static string, with the name of translator

 @return a a static string, with the name of translator
*/
char *
hdate_get_translator_string ()
{
	/* if untranslated return null */
	if (strcmp (_("translator"), "translator") == 0)
		return NULL;

	/* return the translator name */
	return _("translator");
}

/**
 @brief Return a pointer to a static string

 @param type_of_string 	1 = day of week, 2 = parshaot, 3 = hmonth,
						4 = gmonth, 5 = holiday
 @param index
 @param short_form   0 = short format
 @param hebrew_form  0 = not hebrew (native/embedded)
*/
// there are still separate functions for omer and integers
// TODO - Number days of chol hamoed, and maybe have an entry for shabbat chol hamoed
// TODO - change short to be = 1 long = 0, and switch order of data structures
//        this way user app opt.short = 1/TRUE will work as a parameter to pass here

// These definitions are in hdate.h
// HDATE_STRING_DOW     1
// HDATE_STRING_PARASHA 2
// HDATE_STRING_HMONTH  3
// HDATE_STRING_GMONTH  4
// HDATE_STRING_HOLIDAY 5
// HDATE STRING_SHORT   0
// HDATE_STRING_LONG    1
// HDATE_STRING_HEBREW  1
// HDATE_STRING_LOCAL   0
char* hdate_string( int type_of_string, int index, int short_form, int hebrew_form)
{

	static char *days[2][2][7] = {
		{ // begin english
		{ // begin english short
		 N_("Sun"), N_("Mon"), N_("Tue"), N_("Wed"), N_("Thu"),
		 N_("Fri"), N_("Sat")},
		{ // begin english long
		N_("Sunday"), N_("Monday"), N_("Tuesday"), N_("Wednesday"),
		 N_("Thursday"), N_("Friday"), N_("Saturday")}
		},
		{ // begin hebrew
		{ // begin hebrew short
		"א", "ב", "ג", "ד", "ה", "ו", "ש"},
		{ // begin hebrew long
		"ראשון", "שני", "שלישי", "רביעי", "חמישי", "שישי", "שבת"}
		}
		};

	static char *parashaot[2][2][62] = {
		{ // begin english
		{ // begin english short
		 N_("none"),		N_("Bereshit"),		N_("Noach"),
		 N_("Lech-Lecha"),	N_("Vayera"),		N_("Chayei Sara"),
		 N_("Toldot"),		N_("Vayetzei"),		N_("Vayishlach"),
		 N_("Vayeshev"),	N_("Miketz"),		N_("Vayigash"),		/* 11 */
		 N_("Vayechi"),		N_("Shemot"),		N_("Vaera"),
		 N_("Bo"),		N_("Beshalach"),	N_("Yitro"),
		 N_("Mishpatim"),	N_("Terumah"),		N_("Tetzaveh"),		/* 20 */
		 N_("Ki Tisa"),		N_("Vayakhel"),		N_("Pekudei"),
		 N_("Vayikra"),		N_("Tzav"),		N_("Shmini"),
		 N_("Tazria"),		N_("Metzora"),		N_("Achrei Mot"),
		 N_("Kedoshim"),	N_("Emor"),		N_("Behar"),		/* 32 */
		 N_("Bechukotai"),	N_("Bamidbar"),		N_("Nasso"),
		 N_("Beha'alotcha"),	N_("Sh'lach"),		N_("Korach"),
		 N_("Chukat"),		N_("Balak"),		N_("Pinchas"),		/* 41 */
		 N_("Matot"),		N_("Masei"),		N_("Devarim"),
		 N_("Vaetchanan"),	N_("Eikev"),		N_("Re'eh"),
		 N_("Shoftim"),		N_("Ki Teitzei"),	N_("Ki Tavo"),		/* 50 */
		 N_("Nitzavim"),	N_("Vayeilech"),	N_("Ha'Azinu"),
		 N_("Vezot Habracha"),	/* 54 */
		 N_("Vayakhel-Pekudei"),N_("Tazria-Metzora"),	N_("Achrei Mot-Kedoshim"),
		 N_("Behar-Bechukotai"),N_("Chukat-Balak"),	N_("Matot-Masei"),
		 N_("Nitzavim-Vayeilech")},
		{ // begin english long
		 N_("none"),		N_("Bereshit"),		N_("Noach"),
		 N_("Lech-Lecha"),	N_("Vayera"),		N_("Chayei Sara"),
		 N_("Toldot"),		N_("Vayetzei"),		N_("Vayishlach"),
		 N_("Vayeshev"),	N_("Miketz"),		N_("Vayigash"),		/* 11 */
		 N_("Vayechi"),		N_("Shemot"),		N_("Vaera"),
		 N_("Bo"),		N_("Beshalach"),	N_("Yitro"),
		 N_("Mishpatim"),	N_("Terumah"),		N_("Tetzaveh"),		/* 20 */
		 N_("Ki Tisa"),		N_("Vayakhel"),		N_("Pekudei"),
		 N_("Vayikra"),		N_("Tzav"),		N_("Shmini"),
		 N_("Tazria"),		N_("Metzora"),		N_("Achrei Mot"),
		 N_("Kedoshim"),	N_("Emor"),		N_("Behar"),		/* 32 */
		 N_("Bechukotai"),	N_("Bamidbar"),		N_("Nasso"),
		 N_("Beha'alotcha"),	N_("Sh'lach"),		N_("Korach"),
		 N_("Chukat"),		N_("Balak"),		N_("Pinchas"),		/* 41 */
		 N_("Matot"),		N_("Masei"),		N_("Devarim"),
		 N_("Vaetchanan"),	N_("Eikev"),		N_("Re'eh"),
		 N_("Shoftim"),		N_("Ki Teitzei"),	N_("Ki Tavo"),		/* 50 */
		 N_("Nitzavim"),	N_("Vayeilech"),	N_("Ha'Azinu"),
		 N_("Vezot Habracha"),	/* 54 */
		 N_("Vayakhel-Pekudei"),N_("Tazria-Metzora"),	N_("Achrei Mot-Kedoshim"),
		 N_("Behar-Bechukotai"),N_("Chukat-Balak"),	N_("Matot-Masei"),
		 N_("Nitzavim-Vayeilech")}
		},
		{ // begin hebrew
		{ // begin hebrew short
		 "none",		"בראשית",		"נח",
		 "לך לך",		"וירא",			"חיי שרה",
		 "תולדות",		"ויצא",			"וישלח",
		 "וישב",		"מקץ",			"ויגש",		/* 11 */
		 "ויחי",		"שמות",			"וארא",
		 "בא",			"בשלח",			"יתרו",
		 "משפטים",		"תרומה",		"תצוה",		/* 20 */
		 "כי תשא",		"ויקהל",		"פקודי",
		 "ויקרא",		"צו",			"שמיני",
		 "תזריע",		"מצורע",		"אחרי מות",
		 "קדושים",		"אמור",			"בהר",		/* 32 */
		 "בחוקתי",		"במדבר",		"נשא",
		 "בהעלתך",		"שלח",			"קרח",
		 "חקת",			"בלק",			"פנחס",		/* 41 */
		 "מטות",		"מסעי",			"דברים",
		 "ואתחנן",		"עקב",			"ראה",
		 "שופטים",		"כי תצא",		"כי תבוא",		/* 50 */
		 "נצבים",		"וילך",			"האזינו",
		 "וזאת הברכה",	/* 54 */
		 "ויקהל-פקודי",	"תזריע-מצורע",	"אחרי מות-קדושים",
		 "בהר-בחוקתי",	"חוקת-בלק",		"מטות מסעי",
		 "נצבים-וילך"},
		{ // begin hebrew long
		 "none",		"בראשית",		"נח",
		 "לך לך",		"וירא",			"חיי שרה",
		 "תולדות",		"ויצא",			"וישלח",
		 "וישב",		"מקץ",			"ויגש",		/* 11 */
		 "ויחי",		"שמות",			"וארא",
		 "בא",			"בשלח",			"יתרו",
		 "משפטים",		"תרומה",		"תצוה",		/* 20 */
		 "כי תשא",		"ויקהל",		"פקודי",
		 "ויקרא",		"צו",			"שמיני",
		 "תזריע",		"מצורע",		"אחרי מות",
		 "קדושים",		"אמור",			"בהר",		/* 32 */
		 "בחוקתי",		"במדבר",		"נשא",
		 "בהעלתך",		"שלח",			"קרח",
		 "חקת",			"בלק",			"פנחס",		/* 41 */
		 "מטות",		"מסעי",			"דברים",
		 "ואתחנן",		"עקב",			"ראה",
		 "שופטים",		"כי תצא",		"כי תבוא",		/* 50 */
		 "נצבים",		"וילך",			"האזינו",
		 "וזאת הברכה",	/* 54 */
		 "ויקהל-פקודי",	"תזריע-מצורע",	"אחרי מות-קדושים",
		 "בהר-בחוקתי",	"חוקת-בלק",		"מטות מסעי",
		 "נצבים-וילך"}
		}
		};

	static char *hebrew_months[2][2][14] = {
		{ // begin english
		{ // begin english short
		 N_("Tishrei"), N_("Cheshvan"), N_("Kislev"), N_("Tevet"),
		 N_("Sh'vat"), N_("Adar"), N_("Nisan"), N_("Iyyar"),
		 N_("Sivan"), N_("Tammuz"), N_("Av"), N_("Elul"), N_("Adar I"),
		 N_("Adar II")},
		{ // begin english long
		 N_("Tishrei"), N_("Cheshvan"), N_("Kislev"), N_("Tevet"),
		 N_("Sh'vat"), N_("Adar"), N_("Nisan"), N_("Iyyar"),
		 N_("Sivan"), N_("Tammuz"), N_("Av"), N_("Elul"), N_("Adar I"),
		 N_("Adar II")}},
		{ // begin hebrew
		{ // begin hebrew short
		 "תשרי", "חשון", "כסלו", "טבת", "שבט", "אדר", "ניסן", "אייר",
		  "סיון", "תמוז", "אב", "אלול", "אדר א", "אדר ב" },
		{ // begin hebrew long
		 "תשרי", "חשון", "כסלו", "טבת", "שבט", "אדר", "ניסן", "אייר",
		  "סיון", "תמוז", "אב", "אלול", "אדר א", "אדר ב" }}
		};

	static char *gregorian_months[2][12] = {
		{N_("Jan"), N_("Feb"), N_("Mar"), N_("Apr"), N_("May"),
		 N_("Jun"), N_("Jul"), N_("Aug"), N_("Sep"), N_("Oct"),
		 N_("Nov"), N_("Dec")},
		{N_("January"), N_("February"), N_("March"),
		 N_("April"), N_("May"), N_("June"),
		 N_("July"), N_("August"), N_("September"),
		 N_("October"), N_("November"), N_("December")}
	};

	static char *holidays[2][2][37] = {
		{ // begin english
		{ // begin english short
		 N_("Rosh Hashana I"),	N_("Rosh Hashana II"),
		 N_("Tzom Gedaliah"),	N_("Yom Kippur"),
		 N_("Sukkot"),			N_("Hol hamoed Sukkot"),
		 N_("Hoshana raba"),	N_("Simchat Torah"),
		 N_("Chanukah"),		N_("Asara B'Tevet"),	/* 10 */
		 N_("Tu B'Shvat"),		N_("Ta'anit Esther"),
		 N_("Purim"),			N_("Shushan Purim"),
		 N_("Pesach"),			N_("Hol hamoed Pesach"),
		 N_("Yom HaAtzma'ut"),	N_("Lag B'Omer"),
		 N_("Erev Shavuot"),	N_("Shavuot"),			/* 20 */
		 N_("Tzom Tammuz"),		N_("Tish'a B'Av"),
		 N_("Tu B'Av"),			N_("Yom HaShoah"),
		 N_("Yom HaZikaron"),	N_("Yom Yerushalayim"),
		 N_("Shmini Atzeret"),	N_("Pesach VII"),
		 N_("Pesach VIII"),		N_("Shavuot II"),   /* 30 */
		 N_("Sukkot II"),		N_("Pesach II"),	 
		 N_("Family Day"),		N_("Memorial day for fallen whose place of burial is unknown"), 
		 N_("Rabin memorial day"),	 N_("Zhabotinsky day"),
		 N_("Erev Yom Kippur")},
		{ // begin english long
		 N_("Rosh Hashana I"),	N_("Rosh Hashana II"),
		 N_("Tzom Gedaliah"),	N_("Yom Kippur"),
		 N_("Sukkot"),			N_("Hol hamoed Sukkot"),
		 N_("Hoshana raba"),	N_("Simchat Torah"),
		 N_("Chanukah"),		N_("Asara B'Tevet"),
		 N_("Tu B'Shvat"),		N_("Ta'anit Esther"),
		 N_("Purim"),			N_("Shushan Purim"),
		 N_("Pesach"),			N_("Hol hamoed Pesach"),
		 N_("Yom HaAtzma'ut"),	N_("Lag B'Omer"),
		 N_("Erev Shavuot"),	N_("Shavuot"),
		 N_("Tzom Tammuz"),		N_("Tish'a B'Av"),
		 N_("Tu B'Av"),			N_("Yom HaShoah"),
		 N_("Yom HaZikaron"),	N_("Yom Yerushalayim"),
		 N_("Shmini Atzeret"),	N_("Pesach VII"),
		 N_("Pesach VIII"),		N_("Shavuot II"),
		 N_("Sukkot II"),		N_("Pesach II"),
		 N_("Family Day"),		N_("Memorial day for fallen whose place of burial is unknown"), 
		 N_("Yitzhak Rabin memorial day"), N_("Zeev Zhabotinsky day"),
		 N_("Erev Yom Kippur")}
		},
		{ // begin hebrew
		{ // begin hebrew short
		 "א' ראש השנה",		"ב' ראש השנה",
		 "צום גדליה",		"יום הכפורים",
		 "סוכות",		"חול המועד סוכות",
		 "הושענא רבה",		"שמחת תורה",
		 "חנוכה",		"צום עשרה בטבת",/* 10 */
		 "ט\"ו בשבט",		"תענית אסתר",
		 "פורים",		"שושן פורים",
		 "פסח",			"חול המועד פסח",
		 "יום העצמאות",		"ל\"ג בעומר",
		 "ערב שבועות",		"שבועות",	/* 20 */
		 "צום שבעה עשר בתמוז",	"תשעה באב",
		 "ט\"ו באב",		"יום השואה",
		 "יום הזכרון",		"יום ירושלים",
		 "שמיני עצרת",		"שביעי פסח",
		 "אחרון של פסח",	"שני של שבועות",/* 30 */
		 "שני של סוכות",	"שני של פסח",
		 "יום המשפחה",		"יום זכרון...", 
		 "יום הזכרון ליצחק רבין","יום ז\'בוטינסקי",
		 "עיוה\"כ"},
		{ // begin hebrew long
		 "א ר\"ה",		 "ב' ר\"ה",
		 "צום גדליה",		 "יוה\"כ",
		 "סוכות",		 "חוה\"מ סוכות",
		 "הוש\"ר",		 "שמח\"ת",
		 "חנוכה",		 "י' בטבת",	/* 10 */
		 "ט\"ו בשבט",		 "תענית אסתר",
		 "פורים",		 "שושן פורים",
		 "פסח",			 "חוה\"מ פסח",
		 "יום העצמאות",		 "ל\"ג בעומר",
		 "ערב שבועות",		 "שבועות",	/* 20 */
		 "צום תמוז",		 "ט' באב",
		 "ט\"ו באב",		 "יום השואה",
		 "יום הזכרון",		 "יום י-ם",
		 "שמיני עצרת",		 "ז' פסח",
		 "אחרון של פסח",	 "ב' שבועות",   /* 30 */
		 "ב' סוכות",		 "ב' פסח",	 
		 "יום המשפחה",		 "יום זכרון...", 
		 "יום הזכרון ליצחק רבין","יום ז\'בוטינסקי",
		 "עיוה\"כ"}
		}
		};

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif

	// validate parameters
	if (short_form != 0) short_form = 1;
	if (hebrew_form != 0) hebrew_form = 1;

	switch (type_of_string)
	{
	case HDATE_STRING_DOW: if (index >= 1 && index <= 7)
				return _(days[hebrew_form][short_form][index - 1]);
				break;
	case HDATE_STRING_PARASHA: if (index >= 1 && index <= 61)
				return _(parashaot[hebrew_form][short_form][index]);
				break;
	case HDATE_STRING_HMONTH:
				if (index >= 1 && index <= 14)
				return _(hebrew_months[hebrew_form][short_form][index - 1]);
				break;
	case HDATE_STRING_GMONTH:
				if (index >= 1 && index <= 12)
				return _(gregorian_months[short_form][index - 1]);
				break;
	case HDATE_STRING_HOLIDAY: if (index >= 1 && index <= 37)
				return _(holidays[hebrew_form][short_form][index - 1]);
				break;
}

	return NULL;
}

