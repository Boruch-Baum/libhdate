/*  libhdate - Hebrew calendar library: http://libhdate.sourceforge.net
 *
 *  Copyright (C) 2011-2012 Boruch Baum  <boruch-baum@users.sourceforge.net>
 *                2004-2007 Yaacov Zamir <kzamir@walla.co.il>
 *                1984-2003 Amos Shapir
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>  /// For fnmatch
#include <langinfo.h> /// for nl_langinfo()
#include <locale.h>   /// for set_locale()

//#ifdef ENABLE_NLS
//#include <locale.h>
//#endif

#include "hdate.h"
#include "support.h"


/** These are the glibc/gcc constant literals required for
 ** nl_langinfo(), to get internationalization ... **/
static const int langinfo_months[24] = {
	MON_1, MON_2, MON_3, MON_4, MON_5, MON_6, 
	MON_7, MON_8, MON_9, MON_10, MON_11, MON_12, 
	ABMON_1, ABMON_2, ABMON_3, ABMON_4, ABMON_5, ABMON_6, 
	ABMON_7, ABMON_8, ABMON_9, ABMON_10, ABMON_11, ABMON_12 };
static const int langinfo_days[14] = {
	DAY_1, DAY_2, DAY_3, DAY_4, DAY_5, DAY_6, DAY_7,
	ABDAY_1, ABDAY_2, ABDAY_3, ABDAY_4, ABDAY_5, ABDAY_6, ABDAY_7 };


// FIXME - The long and short versions of this next array are
//         identical. Why two sets? Are we considering that a
//         localization might have differences between the 
//         long and short versions?  I doubt it ...
static char *hebrew_months[2][2][14] = {
	{ /// begin english
	{ /// begin english long
	 N_("Tishrei"), N_("Cheshvan"), N_("Kislev"), N_("Tevet"),
	 N_("Shevat"), N_("Adar"), N_("Nisan"), N_("Iyyar"),
	 N_("Sivan"), N_("Tammuz"), N_("Av"), N_("Elul"), N_("Adar_I"),
	 N_("Adar_II")},
	{ /// begin english short
	 N_("Tishrei"), N_("Cheshvan"), N_("Kislev"), N_("Tevet"),
	 N_("Shevat"), N_("Adar"), N_("Nisan"), N_("Iyyar"),
	 N_("Sivan"), N_("Tammuz"), N_("Av"), N_("Elul"), N_("Adar_I"),
	 N_("Adar_II")}
	},
	{ /// begin hebrew
	{ /// begin hebrew long
	 "תשרי", "חשון", "כסלו", "טבת", "שבט", "אדר", "ניסן", "אייר",
	  "סיון", "תמוז", "אב", "אלול", "אדר א", "אדר ב" },
	{ /// begin hebrew short
	 "תשרי", "חשון", "כסלו", "טבת", "שבט", "אדר", "ניסן", "אייר",
	  "סיון", "תמוז", "אב", "אלול", "אדר א", "אדר ב" }}
	};

// FIXME - The english array should not be necessary because
//         we can/should rely on the system locale and
//         nl_langinfo()
static char *gregorian_months[2][12] = {
	{N_("January"), N_("February"), N_("March"),
	 N_("April"), N_("May"), N_("June"),
	 N_("July"), N_("August"), N_("September"),
	 N_("October"), N_("November"), N_("December")},
	{N_("Jan"), N_("Feb"), N_("Mar"), N_("Apr"), N_("May"),
	 N_("Jun"), N_("Jul"), N_("Aug"), N_("Sep"), N_("Oct"),
	 N_("Nov"), N_("Dec")}
};




/**
 @brief helper function to find hebrew locale
 
 @return 0 = latin locale, -1 = hebrew locale
*/
int
hdate_is_hebrew_locale()
{
	char *locale;
	char *language;

	/// Get the name of the current locale.
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
		/// not hebrew locale return false
		return 0;
	}
	
	return -1;
}

/**
 @brief Return a string, with the hebrew date.

 @return NULL pointer upon failure or, upon success, a pointer to a
 string containing the short ( e.g. "1 Tishrey" ) or long (e.g. "Tuesday
 18 Tishrey 5763 Hol hamoed Sukot" ) formated date. You must free() the
 pointer after use.

 @param h The hdate_struct of the date to print.
 @param diaspora if true give diaspora holydays
 @param short_format A short flag (true - returns a short string, false returns a long string).

 @warning This was originally written using a local static string,
          calling for output to be copied away.
*/

char * hdate_get_format_date (hdate_struct const *h, int const diaspora, int const short_format)
{
	int hebrew_format	= HDATE_STRING_LOCAL;
	int omer_day 		= 0;
	int holiday			= 0;
	char *bet_h         = "";	// Hebrew prefix for Hebrew month

	char *hebrew_buffer1, *hebrew_buffer2;
	size_t hebrew_buffer1_len = -1;
	size_t hebrew_buffer2_len = -1;

	char *hday_int_str, *hyear_int_str, *omer_str;

	if (hdate_is_hebrew_locale())
	{
		bet_h="ב";
		hebrew_format = HDATE_STRING_HEBREW;
	}

	hday_int_str = hdate_string(HDATE_STRING_INT, h->hd_day, HDATE_STRING_LONG,hebrew_format);
	if (hday_int_str == NULL) return NULL;
	hyear_int_str = hdate_string(HDATE_STRING_INT, h->hd_year, HDATE_STRING_LONG,hebrew_format);
	if (hyear_int_str == NULL)
	{
		free(hday_int_str);
		return NULL;
	}

	/************************************************************
	* short format
	************************************************************/
	if (short_format)
	{
		hebrew_buffer1_len = asprintf (&hebrew_buffer1, "%s %s %s\n",
				hday_int_str,
				hdate_string( HDATE_STRING_HMONTH , h->hd_mon, HDATE_STRING_LONG, hebrew_format),
				hyear_int_str);
	}


	/************************************************************
	* long (normal) format
	************************************************************/
	else
	{
		hebrew_buffer1_len = asprintf (&hebrew_buffer1, "%s %s%s %s",
				hday_int_str,
				bet_h,
				hdate_string( HDATE_STRING_HMONTH , h->hd_mon, HDATE_STRING_LONG, hebrew_format),
				hyear_int_str);

		/// if a day in the omer print it
		if (hebrew_buffer1_len != -1) omer_day = hdate_get_omer_day(h);
		if (omer_day != 0)
		{
			omer_str = hdate_string(HDATE_STRING_OMER, omer_day, HDATE_STRING_LONG, hebrew_format);
			hebrew_buffer2_len = asprintf (&hebrew_buffer2, "%s, %s", hebrew_buffer1, omer_str);
			if (omer_str != NULL) free(omer_str);
			free(hebrew_buffer1);
			if (hebrew_buffer2_len != -1) hebrew_buffer1 = hebrew_buffer2;
			hebrew_buffer1_len = hebrew_buffer2_len;
		}
		
		/// if holiday print it
		if (hebrew_buffer1_len != -1) holiday = hdate_get_holyday (h, diaspora);
		if (holiday != 0)
		{
			hebrew_buffer2_len = asprintf (&hebrew_buffer2, "%s, %s", hebrew_buffer1,
		  			hdate_string( HDATE_STRING_HOLIDAY, holiday, HDATE_STRING_LONG, hebrew_format));
			free(hebrew_buffer1);
			if (hebrew_buffer2_len != -1) hebrew_buffer1 = hebrew_buffer2;
			hebrew_buffer1_len = hebrew_buffer2_len;
		}

	}

	free(hday_int_str);
	free(hyear_int_str);
	if (hebrew_buffer1_len != -1) return hebrew_buffer1;
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

	/// make a "packge version" string
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
	/// if untranslated return null
	if (strcmp (_("translator"), "translator") == 0)
		return NULL;

	/// return the translator name
	return _("translator");
}

/**
 @brief   Return string values for hdate information
 @return  a pointer to a string containing the information. In the cases
          integers and omer, the strings will NOT be static, and the
          caller must free() them after use.
 @param type_of_string 	0 = integer, 1 = day of week, 2 = parshaot,
						3 = hmonth, 4 = gmonth, 5 = holiday, 6 = omer
 @param index			integer		( 0 < n < 11000)
						day of week ( 0 < n <  8 )
						parshaot	( 0 , n < 62 )
						hmonth		( 0 < n < 15 )
						gmonth		( 0 < n < 13 )
						holiday		( 0 < n < 38 )
						omer		( 0 < n < 50 )
 @param short_form   0 = short format
 @param hebrew_form  0 = not hebrew (native/embedded)
*/

// TODO - Number days of chol hamoed, and maybe have an entry for shabbat chol hamoed
/// DONE - (I hope) change short to be = 1 long = 0, and switch order of data structures
///        this way user app opt.short = 0/FALSE will work as a parameter to pass here

/// These definitions are in hdate.h
///
/// HDATE_STRING_INT     0
/// HDATE_STRING_DOW     1
/// HDATE_STRING_PARASHA 2
/// HDATE_STRING_HMONTH  3
/// HDATE_STRING_GMONTH  4
/// HDATE_STRING_HOLIDAY 5
/// HDATE_STRING_OMER    6
/// HDATE STRING_SHORT   1
/// HDATE_STRING_LONG    0
/// HDATE_STRING_HEBREW  1
/// HDATE_STRING_LOCAL   0
char* hdate_string( int const type_of_string, int const index, int const input_short_form, int const input_hebrew_form)
{
	int short_form = 0;
	int hebrew_form = 0;

	/// type_of_string: integer and omer require allocated strings
	char *return_string = NULL;
	int return_string_len = -1;

	/// for nl_langinfo calls for DOW an gregrorian months
	char* langinfo_ptr;

	#define H_CHAR_WIDTH 2
	static char *digits[3][10] = {
		{" ", "א", "ב", "ג", "ד", "ה", "ו", "ז", "ח", "ט"},
		{"ט", "י", "כ", "ל", "מ", "נ", "ס", "ע", "פ", "צ"},
		{" ", "ק", "ר", "ש", "ת"}
	};

	// FIXME - The english array should not be necessary because
	//         we can/should rely on the system locale and
	//         nl_langinfo()
	static char *days[2][2][7] = {
		{ /// begin english
		{ /// begin english long
		N_("Sunday"), N_("Monday"), N_("Tuesday"), N_("Wednesday"),
		 N_("Thursday"), N_("Friday"), N_("Saturday")},
		{ /// begin english short
		 N_("Sun"), N_("Mon"), N_("Tue"), N_("Wed"), N_("Thu"),
		 N_("Fri"), N_("Sat")}
		},
		{ /// begin hebrew
		{ /// begin hebrew long
		"ראשון", "שני", "שלישי", "רביעי", "חמישי", "שישי", "שבת"},
		{ /// begin hebrew short
		"א", "ב", "ג", "ד", "ה", "ו", "ש"}
		}
		};

	static char *parashaot[2][2][62] = {
		{ /// begin english
		{ /// begin english long
		 N_("none"),		N_("Bereshit"),		N_("Noach"),
		 N_("Lech-Lecha"),	N_("Vayera"),		N_("Chayei_Sara"),
		 N_("Toldot"),		N_("Vayetzei"),		N_("Vayishlach"),
		 N_("Vayeshev"),	N_("Miketz"),		N_("Vayigash"),		/* 11 */
		 N_("Vayechi"),		N_("Shemot"),		N_("Vaera"),
		 N_("Bo"),		N_("Beshalach"),	N_("Yitro"),
		 N_("Mishpatim"),	N_("Terumah"),		N_("Tetzaveh"),		/* 20 */
		 N_("Ki_Tisa"),		N_("Vayakhel"),		N_("Pekudei"),
		 N_("Vayikra"),		N_("Tzav"),		N_("Shmini"),
		 N_("Tazria"),		N_("Metzora"),		N_("Achrei_Mot"),
		 N_("Kedoshim"),	N_("Emor"),		N_("Behar"),		/* 32 */
		 N_("Bechukotai"),	N_("Bamidbar"),		N_("Nasso"),
		 N_("Beha'alotcha"),	N_("Sh'lach"),		N_("Korach"),
		 N_("Chukat"),		N_("Balak"),		N_("Pinchas"),		/* 41 */
		 N_("Matot"),		N_("Masei"),		N_("Devarim"),
		 N_("Vaetchanan"),	N_("Eikev"),		N_("Re'eh"),
		 N_("Shoftim"),		N_("Ki_Teitzei"),	N_("Ki_Tavo"),		/* 50 */
		 N_("Nitzavim"),	N_("Vayeilech"),	N_("Ha'Azinu"),
		 N_("Vezot_HaBracha"),	/* 54 */
		 N_("Vayakhel-Pekudei"),N_("Tazria-Metzora"),	N_("Achrei_Mot-Kedoshim"),
		 N_("Behar-Bechukotai"),N_("Chukat-Balak"),	N_("Matot-Masei"),
		 N_("Nitzavim-Vayeilech")},
		{ /// begin english short
		 N_("none"),		N_("Bereshit"),		N_("Noach"),
		 N_("Lech-Lecha"),	N_("Vayera"),		N_("Chayei_Sara"),
		 N_("Toldot"),		N_("Vayetzei"),		N_("Vayishlach"),
		 N_("Vayeshev"),	N_("Miketz"),		N_("Vayigash"),		/* 11 */
		 N_("Vayechi"),		N_("Shemot"),		N_("Vaera"),
		 N_("Bo"),		N_("Beshalach"),	N_("Yitro"),
		 N_("Mishpatim"),	N_("Terumah"),		N_("Tetzaveh"),		/* 20 */
		 N_("Ki_Tisa"),		N_("Vayakhel"),		N_("Pekudei"),
		 N_("Vayikra"),		N_("Tzav"),		N_("Shmini"),
		 N_("Tazria"),		N_("Metzora"),		N_("Achrei_Mot"),
		 N_("Kedoshim"),	N_("Emor"),		N_("Behar"),		/* 32 */
		 N_("Bechukotai"),	N_("Bamidbar"),		N_("Nasso"),
		 N_("Beha'alotcha"),	N_("Sh'lach"),		N_("Korach"),
		 N_("Chukat"),		N_("Balak"),		N_("Pinchas"),		/* 41 */
		 N_("Matot"),		N_("Masei"),		N_("Devarim"),
		 N_("Vaetchanan"),	N_("Eikev"),		N_("Re'eh"),
		 N_("Shoftim"),		N_("Ki_Teitzei"),	N_("Ki_Tavo"),		/* 50 */
		 N_("Nitzavim"),	N_("Vayeilech"),	N_("Ha'Azinu"),
		 N_("Vezot_HaBracha"),	/* 54 */
		 N_("Vayakhel-Pekudei"),N_("Tazria-Metzora"),	N_("Achrei_Mot-Kedoshim"),
		 N_("Behar-Bechukotai"),N_("Chukat-Balak"),	N_("Matot-Masei"),
		 N_("Nitzavim-Vayeilech")}
		},
		{ /// begin hebrew
		{ /// begin hebrew long
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
		 "בהר-בחוקתי",	"חוקת-בלק",		"מטות-מסעי",
		 "נצבים-וילך"},
		{ /// begin hebrew short
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
		 "בהר-בחוקתי",	"חוקת-בלק",		"מטות-מסעי",
		 "נצבים-וילך"}
		}
		};


	static char *holidays[2][2][40] = {
		{ /// begin english
		{ /// begin english long
/**  0 **/ N_("regular_weekday_(no_holiday)"),
/**  1 **/ N_("Rosh_HaShana_(first_day)"),	N_("Rosh HaShana_(second_day)"),
		   N_("Tzom_Gedaliah"),				N_("Yom_Kippur"),
/**  5 **/ N_("Sukkot"),						N_("Hol_HaMoed_Sukkot"),
		   N_("Hoshana_Rabbah"),				N_("Simchat_Torah"),
/**  9 **/ N_("Chanukah"),					N_("Asara_B'Tevet"),
		   N_("Tu_B'Shvat"),					N_("Ta'anit_Esther"),
/** 13 **/ N_("Purim"),						N_("Shushan_Purim"),
		   N_("Pesach"),						N_("Hol_HaMoed_Pesach"),
/** 17 **/ N_("Yom_HaAtzma'ut"),				N_("Lag_B'Omer"),
		   N_("Erev_Shavuot"),				N_("Shavuot"),
/** 21 **/ N_("Tzom_Tammuz"),					N_("Tish'a_B'Av"),
		   N_("Tu_B'Av"),						N_("Yom_HaShoah"),
/** 25 **/ N_("Yom_HaZikaron"),				N_("Yom_Yerushalayim"),
		   N_("Shmini_Atzeret"),				N_("Shevi'i_shel_Pesach"),
/** 29 **/ N_("Acharon_shel_Pesach"),			N_("Shavuot_(second_day)"),
		   N_("Sukkot_(second_day)"),			N_("Pesach_(second_day)"),
/** 33 **/ N_("Family_Day"),					N_("Memorial_day_for_fallen_whose_place_of_burial_is_unknown"), 
		   N_("Yitzhak_Rabin_memorial_day"),	N_("Zeev_Zhabotinsky_day"),
/** 37 **/ N_("Erev_Yom_Kippur"),				N_("Erev_Pesach"),
/** 39 **/ N_("Erev_Sukkot")},
		{ /// begin_english short
/**  0 **/ N_("regular_day"),
		 N_("Rosh HaShana_(day_1)"),	N_("Rosh HaShana_(day_2)"),
		 N_("Tzom_Gedaliah"),			N_("Yom_Kippur"),
		 N_("Sukkot"),					N_("Hol_HaMoed_Sukkot"),
		 N_("Hoshana_Rabbah"),			N_("Simchat_Torah"),
		 N_("Chanukah"),				N_("Asara_B'Tevet"),	/* 10 */
		 N_("Tu_B'Shvat"),				N_("Ta'anit_Esther"),
		 N_("Purim"),					N_("Shushan_Purim"),
		 N_("Pesach"),					N_("Hol_HaMoed_Pesach"),
		 N_("Yom_HaAtzma'ut"),			N_("Lag_B'Omer"),
		 N_("Erev_Shavuot"),			N_("Shavuot"),			/* 20 */
		 N_("Tzom_Tammuz"),				N_("Tish'a_B'Av"),
		 N_("Tu_B'Av"),					N_("Yom_HaShoah"),
		 N_("Yom_HaZikaron"),			N_("Yom_Yerushalayim"),
		 N_("Shmini_Atzeret"),			N_("Pesach_(day_7)"),
		 N_("Pesach_(day_8)"),			N_("Shavuot_(day_2)"),   /* 30 */
		 N_("Sukkot_(day_2)"),			N_("Pesach_(day_2)"),	 
		 N_("Family_Day"),				N_("Memorial_day_for_fallen_whose_place_of_burial_is_unknown"), 
		 N_("Rabin_memorial_day"),		N_("Zhabotinsky_day"),
		 N_("Erev_Yom_Kippur"),			N_("Erev_Pesach"),
		 N_("Erev_Sukkot")}
		},
		{ /// begin hebrew
		{ /// begin hebrew long
		 "יום חול",
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
		 "ערב יום הכפורים",	"ערב פסח",
		 "ערב סוכות"},
		{ /// begin hebrew short
		 "חול",
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
		 "עיוה\"כ",			"ע\"פ",
		 "ערב סוכות"}	}
		};

	/// This next is for counting days, weeks, or months
	static char *count_days[23] = {
		"שני", "אחד", "שניים", "שלשה", "ארבעה",	"חמשה",
		"ששה", "שבעה", "שמונה",	"תשעה",	"עשרה", "עשר",
		"עשרים", "שלשים", "ארבעים",
		};

	static char *vav = "ו";

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
#endif

	/// validate parameters
	if (input_short_form != 0) short_form = 1;
	if (input_hebrew_form != 0) hebrew_form = 1;

	switch (type_of_string)
	{
	case HDATE_STRING_DOW: if (index >= 1 && index <= 7)
		{
	/** Use our local data structure and very limited set of gettext po
	 ** translations only if the host OS does not have, or fails to set,
	 ** the locale for time and date data. The exception for Hebrew is
	 ** because a) it's expected to be used by users in all locales; and
	 ** b) it's much 'cheap'er than setting and resetting the locale **/
		if ((setlocale(LC_TIME,"") == NULL) || hebrew_form )
				return _(days[hebrew_form][short_form][index - 1]);
	/** If setlocale() returns a string, then the system has information
	 ** that nl_langinfo can use to give us a localized string. **/
		langinfo_ptr = nl_langinfo(langinfo_days[ ( (short_form*7)) + (index-1) ]);
		
	/** nl_langinfo may return a pointer to a null string if it does
	 ** not have the requeste value. In such a case return the English
	 ** (or possibly gettext ?) string **/
		if ( strcmp(langinfo_ptr, "") == 0 ) return _(days[hebrew_form][short_form][index - 1]);
		else return langinfo_ptr;
		}
		break;
	case HDATE_STRING_PARASHA: if (index >= 1 && index <= 61)
				return _(parashaot[hebrew_form][short_form][index]);
				break;
	case HDATE_STRING_HMONTH: if (index >= 1 && index <= 14)
				return _(hebrew_months[hebrew_form][short_form][index - 1]);
				break;
	case HDATE_STRING_GMONTH: if (index >= 1 && index <= 12)
	/** Use our local data structure and very limited set of gettext po
	 ** translations only if the host OS does not have, or fails to set,
	 ** the locale for time and date data. **/
		if (setlocale(LC_TIME,"") == NULL)
				return _(gregorian_months[short_form][index - 1]);
	/** This code improvement is to enable full internationalization
	 ** using nl_langinfo(), which requires glibc/gcc constant literals
	 ** so I've defined an array 'langinfo_months' with the list, for
	 ** all month full names an abbreviations **/
		langinfo_ptr = nl_langinfo(langinfo_months[ ((index-1)+(short_form*12)) ]);

	/** nl_langinfo may return a pointer to a null string if it does
	 ** not have the requested value. In such a case return the English
	 ** (or possibly gettext ?) string **/
		if ( strcmp(langinfo_ptr, "") == 0 ) return _(gregorian_months[short_form][index - 1]);
		else return langinfo_ptr;
		break;
	case HDATE_STRING_HOLIDAY: if (index >= 0 && index <= 39)
				return _(holidays[hebrew_form][short_form][index]);
				break;
	case HDATE_STRING_OMER:
				if (index > 0 && index < 50)
				{
					if (index < 11)
					{
						if (index == 2)  return_string_len = asprintf(&return_string, "%s ", count_days[0]);
						else return_string_len = asprintf(&return_string, "%s ", count_days[index]);
					}
					else
					{
						if ((index%10) == 0)
							return_string_len = asprintf(&return_string, "%s ", count_days[(index/10)+10]);
						else if (index < 20)
							return_string_len = asprintf(&return_string, "%s %s ",
													count_days[(index%10)], count_days[10]);
						else return_string_len = asprintf(&return_string, "%s %s%s ",
														count_days[(index%10)], vav, count_days[(index/10)+10]);
					}
					if (return_string_len != -1) return return_string;
				}
				return NULL;
				break;
	case HDATE_STRING_INT:
				if ((index > 0) && (index < 11000))
				{
					/// not hebrew form - return the number in decimal form
					if (!hebrew_form)
					{
						return_string_len = asprintf (&return_string, "%d", index);
						if (return_string_len == -1) return NULL;
						return return_string;
					}

					/// HEBREW_NUMBER_BUFFER_SIZE 17	defined in hdate.h
					return_string = malloc(HEBREW_NUMBER_BUFFER_SIZE);
					if (return_string == NULL) return NULL;
					
					return_string[0] = '\0';

					int n = index;

					if (n >= 1000)
					{
						strncat (return_string, digits[0][n / 1000], H_CHAR_WIDTH);
						n %= 1000;
					}
					while (n >= 400)
					{
						strncat (return_string, digits[2][4], H_CHAR_WIDTH);
						n -= 400;
					}
					if (n >= 100)
					{
						strncat (return_string, digits[2][n / 100], H_CHAR_WIDTH);
						n %= 100;
					}
					if (n >= 10)
					{
						if (n == 15 || n == 16)
							n -= 9;
						strncat (return_string, digits[1][n / 10], H_CHAR_WIDTH);
						n %= 10;
					}
					if (n > 0)
						strncat (return_string, digits[0][n], H_CHAR_WIDTH);
						
				 	/// possibly add the ' and " to hebrew numbers	
					if (!short_form)
					{
						return_string_len = strlen (return_string);
						if (return_string_len <= H_CHAR_WIDTH) strncat (return_string, "'", H_CHAR_WIDTH);
						else
						{
							return_string[return_string_len + 1] = return_string[return_string_len];
							return_string[return_string_len] = return_string[return_string_len - 1];
							return_string[return_string_len - 1] = return_string[return_string_len - 2];
							return_string[return_string_len - 2] = '\"';
							return_string[return_string_len + 2] = '\0';
						}
					}
					return return_string;
				}
				return NULL;
				break;
	} /// end of switch(type_of_string)

	return NULL;
}



/************************************************************
* hdate_parse_month_text_string
* 
* compares a string to system locale's month strings and to 
* Hebrew month strings. It does not rely only on gettext/po,
* and should work for ALL localizations worldwide for which
* nl_langinfo() calls work.
*
* returns month number   1 -  12 for gregorian
*                      101 - 114 for Hebrew
* returns 0 on failure
************************************************************/
// Better idea maybe: return 101 - 114 for Hebrew
int hdate_parse_month_text_string( const char* month_text )
{
	/** This next call is supposed to guarantee that, if a locale
	 ** exists, that the time spec for it is set. This is to enable
	 ** full internationalization using nl_langinfo() **/
	setlocale(LC_TIME,"");

	int i;
	
	/// Check for a match against the list of month strings for the
	/// user's locale (both full month names and abbreviations)
	for (i=0; i<24; i++)
		if ( strcasecmp( month_text, nl_langinfo(langinfo_months[i])) == 0 )
			return (i%12)+1;

	/// Maybe the user entered a Hebrew month
	for (i=0; i<14; i++)
		if (( strcasecmp( month_text, hebrew_months[0][0][i]) == 0 ) ||
			( strcasecmp( month_text, hebrew_months[1][0][i]) == 0 ) )
			/// This checks the latin and Hebrew character strings
			/// It ignores the 'short' versions because they are
			/// in practice identical to the 'long' versions
			return (i%14)+101;

	/// Alternative latin character spellings of Hebrew Months
	/// and alternative names of Hebrew months in latin characters
	static char *alt_latin_spell_hebrew_months[14] = {
		"@(?(@(Y|J)ere@(ch|kh|h|x|j)?( |_|-)Ha?(-))@(A|Ai|Ay|Ei)t?(h)a?(h)n@(i|ee)m|Ti[cs]hr@([ie]?(y)|a[iy]))",  
		"@(b@(oo|ou|u)?(l)l|?(Mar?( |-|_))@(?(C|K)h|X|J)e[sc]hva?(h)n)",
		"Kisl@(e|[ae][iy])v",
		"T@(e|[ae][iy])v@(e|[ae][iy])t",
		"[SC]h?([e'])vat",
		"Ad?(d)a?(h)r",
		"N@(i|ee)s?(s)a?(h)n",
		"@(ziv|zeev|@(I|e|Ee)?(y|yy|j)?(')a?(h)r)",
		"[SC]@(i|ee)v?(v)a?(h)n",
		"Tam?(m)uz",
		"?(Mena@(ch|kh|x|j)@(e|ei|ey|ai|ay|i)m?( |_|-))A?(h)v",
		"El?(l)@(u|ou|oo)l?(l)",
		"Ad?(d)a?(h)r?( |-|_)@(A|1|I|alef|aleph)",
		"Ad?(d)a?(h)r?( |-|_)@(B|2|II|bet?(h))"
		};

	for (i=0; i<14; i++)
		if ( fnmatch( alt_latin_spell_hebrew_months[i], month_text, FNM_EXTMATCH|FNM_CASEFOLD)==0 )
			return (i+101);

	/// Alternative Hebrew names for Hebrew Months
	static char *alt_hebrew_spell_hebrew_months[14] = {
		"?(ירח?( |-|־)ה)איתנים",
		"@(?(מר?( |-|־))חשון|בול)",
		"",
		"",
		"",
		"",
		"",
		"זיו",
		"",
		"",
		"?(מנחם?( |-|־))אב",
		"",
		"אדר?( |-|־)א?(')",
		"אדר?( |-|־)ב?(')"
		};

	for (i=0; i<14; i++)
		if ( fnmatch( alt_hebrew_spell_hebrew_months[i], month_text, FNM_EXTMATCH|FNM_CASEFOLD)==0 )
			return (i+101);


	// This REALLY should not be necessary ...
	/** nl_langinfo may return a pointer to a null string if it does
	 ** not have the requested value. In such a case return the English
	 ** (or possibly gettext ?) string **/
	for (i=0; i<14; i++)
		if (( strcasecmp( month_text, gregorian_months[0][i]) == 0 ) ||
			( strcasecmp( month_text, gregorian_months[1][i]) == 0 ) )
			return (i%12)+1;

	
	return 0;
}
